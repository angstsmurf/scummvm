/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef GLK_COMPREHEND_GM_STREAM_H
#define GLK_COMPREHEND_GM_STREAM_H
#include "glk/comprehend/graphics_magician.h"   // Opcode

/* The Graphics Magician opcode-stream dispatcher, shared by every renderer. The high nibble of
 * each command byte is the opcode (graphics_magician.h `Opcode`), the low nibble is `param`, and
 * bit 0 of `param` is the high bit (+256) of the x operand for coordinate opcodes. gmRunStream()
 * owns the loop, the operand decode and the order of the DRAW_BOX lines; each renderer supplies
 * the leaf behaviour of each opcode as a small object `R`, held by reference.
 *
 * `R` must provide (x operands are source coordinates 0..511; the dispatcher passes them through
 * scaleX() first, so everything below the box/line level works in native x units; y is never
 * scaled):
 *
 *   uint16 scaleX(uint16 x);   source x -> native x (identity, or x << 1 on DHGR)
 *   int  penX(); int penY();       current pen in native units (DRAW_BOX start)
 *
 *   void begin();                  per-image init (pen, brush, fill, text, clip)
 *   void end();                    runs on every exit, incl. OPCODE_END
 *
 *   void setTextPos(x, y);         op1  (x already scaled)
 *   void drawText(ch, normal);     op3 (normal = true) / op5 (false)
 *   void advanceText();            after each text op
 *   void setPen(param);            op2
 *   void setShape(param);          op4
 *   void setFill(b);               op6
 *   bool op7(p, end);              op7: consume its own operands from `p` (the
 *                                  legacy Apple dialect consumes none); return
 *                                  true to end the image
 *   void moveTo(x, y);             op8  (x scaled)
 *   void lineTo(x, y);             op10 and each DRAW_BOX edge: draw from the
 *                                  pen to (x,y), pen left at (x,y) (x scaled)
 *   void circle(radius);           op11, centred on the pen
 *   void drawBrush(x, y);          op12 (x scaled)
 *   void delay(units);             op13
 *   void paint(x, y);              op14 (x scaled)
 *   bool opReset(param, p, end);   op15: consume its own operands from `p`;
 *                                  return true to end the image
 *   void beforeOp(p, end, op, param);  trace hook, after the command byte is
 *                                  read (`p` points at its first operand)
 *   void afterOp(op, param);       trace hook, after every op that did not end
 *                                  the image
 *
 * Truncated streams: an opcode whose operands run past `end` ends the image without being
 * executed, so nothing reads beyond the buffer. The dispatcher checks the operand count of the
 * opcodes it decodes itself (gmOperandBytes); op7() and opReset() own theirs and must return true
 * when fewer than they need remain (gmHave).
 */

namespace Glk {
namespace Comprehend {

// Read a coordinate operand pair: x (+256 when bit 0 of `param` is set), then y.
inline void gmReadXY(const uint8 *&p, uint8 param, uint16 &x, uint8 &y) {
	x = (uint16)(*p++ + ((param & 1) ? 256 : 0));
	y = *p++;
}

// True when at least `n` bytes remain between `p` and `end`.
inline bool gmHave(const uint8 *p, const uint8 *end, size_t n) {
	return (size_t)(end - p) >= n;
}

// op7 as the Talisman-era pictures use it: two operand bytes that mean nothing.
// Skips them; returns true (end the image) when fewer than two remain.  The op7
// hook of a renderer that gives the operands no meaning is just this.
inline bool gmSkipOp7(const uint8 *&p, const uint8 *end) {
	if (!gmHave(p, end, 2))
		return true;
	p += 2;
	return false;
}

// Operand bytes of the opcodes the dispatcher decodes itself.  op7 and op15 take
// a renderer-specific number, so they report 0 here and check for themselves.
inline size_t gmOperandBytes(uint8 op) {
	switch (op) {
	case OPCODE_SET_TEXT_POS:
	case OPCODE_MOVE_TO:
	case OPCODE_DRAW_BOX:
	case OPCODE_DRAW_LINE:
	case OPCODE_DRAW_SHAPE:
	case OPCODE_PAINT:
		return 2;
	case OPCODE_TEXT_CHAR:
	case OPCODE_TEXT_OUTLINE:
	case OPCODE_SET_FILL_COLOR:
	case OPCODE_DRAW_CIRCLE:
	case OPCODE_DELAY:
		return 1;
	default:
		return 0;
	}
}

// The op15 fill-clip rectangle: source byte columns (0..39) and picture rows.
// Set to the full picture by op15/1, read from the stream by op15/2 (and by the
// op15/0 RLE bitmap, which reads its bounds the same way).  Every renderer that
// has op15 keeps one; `bottom` resets to the renderer's own last picture row.
struct GmFillBounds {
	uint8 left, right, top, bottom;

	void reset(uint8 bottomRow) { left = 0; right = 39; top = 0; bottom = bottomRow; }

	// Four operand bytes, in stream order: right, left, bottom, top.  Returns false
	// (nothing consumed) if fewer than four remain.
	bool read(const uint8 *&p, const uint8 *end) {
		if (!gmHave(p, end, 4))
			return false;
		right = *p++;
		left = *p++;
		bottom = *p++;
		top = *p++;
		return true;
	}
};

// op15/0: stream a run-length-compressed bitmap into the rectangle `b`, column
// major (top to bottom within a column, then the next column).  A 0x80 byte
// escapes a run: the next two bytes are count and value, written count+1 times.
// It stops the moment the bottom-right cell is written, even mid-run.  The
// renderer supplies the cell write: write(row, col, byte).
template <class Write>
inline void gmDecodeRleBitmap(const GmFillBounds &b, const uint8 *&p,
							  const uint8 *end, Write write) {
	int col = b.left, row = b.top;
	bool done = false;
	while (!done && p < end) {
		uint8 v = *p++;
		int reps = 1;
		if (v == 0x80) {
			if (p + 2 > end) break;
			reps = *p++ + 1;
			v = *p++;
		}
		while (reps--) {
			write(row, col, v);
			if (row != b.bottom) {
				row++;
			} else if (col == b.right) {
				done = true;
				break;
			} else {
				col++;
				row = b.top;
			}
		}
	}
}

template <class R>
inline void gmRunStream(R &r, const uint8 *data, size_t size) {
	const uint8 *p = data;
	const uint8 *e = data + size;
	r.begin();
	int guard = 0;
	while (p < e && guard++ < 100000) {
		const uint8 cmd = *p++;
		const uint8 param = cmd & 0xf;
		const uint8 op = cmd >> 4;
		uint16 x;
		uint8 y;
		bool done = false;

		if (!gmHave(p, e, gmOperandBytes(op)))
			break;                       // truncated operands: stop, do not execute
		r.beforeOp(p, e, op, param);
		switch (op) {
		case OPCODE_END:
			done = true;
			break;

		case OPCODE_OP7:
			done = r.op7(p, e);
			break;

		case OPCODE_SET_TEXT_POS:
			gmReadXY(p, param, x, y);
			r.setTextPos(r.scaleX(x), y);
			break;

		case OPCODE_SET_PEN_COLOR:
			r.setPen(param);
			break;

		case OPCODE_TEXT_CHAR:
			r.drawText(*p++, true);
			r.advanceText();
			break;

		case OPCODE_TEXT_OUTLINE:
			r.drawText(*p++, false);
			r.advanceText();
			break;

		case OPCODE_SET_SHAPE:
			r.setShape(param);
			break;

		case OPCODE_SET_FILL_COLOR:
			r.setFill(*p++);
			break;

		case OPCODE_MOVE_TO:
			gmReadXY(p, param, x, y);
			r.moveTo(r.scaleX(x), y);
			break;

		case OPCODE_DRAW_BOX: {
			// Four chained edges from the pen's start corner; the pen is left at the
			// far corner (a,b), not the start.
			gmReadXY(p, param, x, y);
			const uint16 a = r.scaleX(x);
			const int x0 = r.penX(), y0 = r.penY();
			r.lineTo((uint16)x0, y);          // left edge   (x0,y0)->(x0,b)
			r.lineTo(a, y);                     // bottom edge ->(a,b)
			r.lineTo(a, (uint8)y0);           // right edge  ->(a,y0)
			r.lineTo((uint16)x0, (uint8)y0);// top edge    ->(x0,y0)
			r.moveTo(a, y);
			break;
		}

		case OPCODE_DRAW_LINE:
			gmReadXY(p, param, x, y);
			r.lineTo(r.scaleX(x), y);
			break;

		case OPCODE_DRAW_CIRCLE:
			r.circle(*p++);
			break;

		case OPCODE_DRAW_SHAPE:
			gmReadXY(p, param, x, y);
			r.drawBrush(r.scaleX(x), y);
			break;

		case OPCODE_DELAY:
			r.delay(*p++);
			break;

		case OPCODE_PAINT:
			gmReadXY(p, param, x, y);
			r.paint(r.scaleX(x), y);
			break;

		case OPCODE_RESET:
			done = r.opReset(param, p, e);
			break;
		}
		if (done)
			break;
		r.afterOp(op, param);
	}
	r.end();
}

} // namespace Comprehend
} // namespace Glk

#endif
