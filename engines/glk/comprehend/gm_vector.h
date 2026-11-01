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

#ifndef gm_vector_h
#define gm_vector_h

#include "glk/comprehend/gm_hires.h"

// Apple II Graphics Magician vector primitives: cursor, line and brush. The caller owns the hi-res
// page and supplies a write hook, so it keeps its own slow-draw and dirty-row bookkeeping.

namespace Glk {
namespace Comprehend {


// Cursor + colour state for the Graphics Magician line primitives. The field
// names match the 6502 zero-page locations the disassembly uses.
typedef struct gm_vector_ctx {
    uint16 HBAS;       // hi-res base address of row HGR_Y
    uint16 HGR_X;      // current pen X (0..279)
    uint8  HGR_Y;      // current pen Y (0..191)
    uint8  HMASK;      // bit mask within the current column byte
    uint8  HGR_BITS;   // colour byte, phase-adjusted for the column
    uint8  HGR_HORIZ;  // current column, i.e. byte within the row (0..39)
    uint8  HGR_COLOR;  // current pen colour (hi-res byte)

    uint8 *screenmem;  // hi-res page (>= 0x2000 bytes), owned by the caller
    void   (*write)(uint16 offset, uint8 value, void *user); // engine write hook
    void    *user;       // opaque, passed through to write()

    // Drawing tables (owned by the caller), needed by gm_draw_brush only.
    const uint8 *pattern_data;   // 120 bytes: 30 colour patterns x 4
    const uint8 *brush_bitmaps;  // 256 bytes: 8 brushes x 4 quadrants x 8 rows
} gm_vector_ctx;

// Spread an 8-bit source byte (brush or glyph row) across two adjacent hi-res
// columns by `offset` pixels: the low 7 bits land in *mask_a (column col), the
// pixels carried past bit 6 land in *mask_b (column col+1). Shared by the brush
// blitter and the text-glyph blitter.
void gm_spread7(uint8 src, uint8 offset, uint8 *mask_a, uint8 *mask_b);

// Set the current pen colour from a 0..7 colour index. Other values leave the
// colour unchanged.
void gm_set_color(uint8 color_index, gm_vector_ctx *c);

// Move the pen to (x, y), recomputing HBAS / HMASK / HGR_BITS for that position.
void gm_set_draw_position(uint16 x, uint8 y, gm_vector_ctx *c);

// Plot the single pixel at the current pen position (exposed for circle drawing).
void gm_plot_pixel(gm_vector_ctx *c);

// Draw a line from the current pen position to (target_x, target_y); leaves the
// pen at the target.
void gm_draw_line(uint16 target_x, uint8 target_y, gm_vector_ctx *c);

// Draw brush shape `brush` (0..7) with its top-left corner at (x, y), blended
// with the fill-colour pattern given as even/odd-row sub-indices. Requires
// c->pattern_data and c->brush_bitmaps to be set. Does not touch the pen cursor.
void gm_draw_brush(uint16 x, uint8 y, uint8 brush,
    uint8 pat_even, uint8 pat_odd, gm_vector_ctx *c);



} // namespace Comprehend
} // namespace Glk

#endif /* gm_vector_h */
