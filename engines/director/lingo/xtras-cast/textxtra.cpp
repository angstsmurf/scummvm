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

#include "common/endian.h"

#include "graphics/macgui/macfontmanager.h"
#include "graphics/macgui/mactext.h"
#include "graphics/macgui/macwindowmanager.h"

#include "director/director.h"
#include "director/movie.h"
#include "director/cast.h"
#include "director/channel.h"
#include "director/castmember/xtra.h"
#include "director/lingo/lingo-the.h"
#include "director/lingo/xtras-cast/textxtra.h"
#include "director/window.h"

namespace Director {

namespace TextXtra {

static bool isHexDigit(byte c) {
	return (c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f');
}

static uint hexValue(byte c) {
	if (c >= '0' && c <= '9')
		return c - '0';
	if (c >= 'A' && c <= 'F')
		return c - 'A' + 10;
	return c - 'a' + 10;
}

// Fallback: find the first counted byte string, 0x00 <hex length> ',' <text>
static bool findXMEDText(const Common::Array<byte> &data, Common::String &text) {
	uint32 size = data.size();
	for (uint32 i = 0; i + 2 < size; i++) {
		if (data[i] != 0x00)
			continue;

		uint32 j = i + 1;
		uint32 len = 0;
		int digits = 0;
		while (j < size && isHexDigit(data[j]) && digits < 6) {
			len = len * 16 + hexValue(data[j]);
			j++;
			digits++;
		}

		if (digits == 0 || len == 0 || j >= size || data[j] != ',')
			continue;
		j++;

		if (j + len > size)
			continue;

		uint32 printable = 0;
		for (uint32 k = 0; k < len; k++) {
			byte c = data[j + k];
			if ((c >= 0x20 && c <= 0x7e) || c >= 0x80 || c == '\r' || c == '\n' || c == '\t')
				printable++;
		}

		if (printable * 5 < len * 4)
			continue;

		text = Common::String((const char *)&data[j], len);
		return true;
	}
	return false;
}

// Walker over Paige packed data (PGREAD.C pgUnpackNum/pgUnpackBytes).
// Every item starts with a code byte: low nibble 0 = counted bytes
// (<hex length> ',' <bytes>), 1 = short, 2 = long (ASCII hex, optional
// '-'), 3 = terminator. Bit 0x80 repeats the last number, with 0x40 the
// next byte holds the repeat count.
class PackWalker {
public:
	enum { kByteData = 0, kShortData = 1, kLongData = 2, kTerminator = 3 };

	PackWalker(const byte *data, uint32 size) : _p(data), _end(data + size) {}

	int nextCode() const {
		if (_repeat)
			return _lastCode;
		if (_p >= _end)
			return kTerminator;
		return *_p & 0x0F;
	}

	bool atEnd() const {
		int code = nextCode();
		return code != kByteData && code != kShortData && code != kLongData;
	}

	// Reads the next item; size gets its unpacked byte size. Counted
	// bytes come back in ptr/len with value 0.
	bool next(int32 &value, uint32 &size, const byte **ptr = nullptr, uint32 *len = nullptr) {
		value = 0;
		size = 0;
		if (ptr)
			*ptr = nullptr;
		if (len)
			*len = 0;

		if (_repeat) {
			_repeat--;
			value = _last;
			size = (_lastCode == kShortData) ? 2 : 4;
			return true;
		}
		if (atEnd())
			return false;

		byte code = *_p++;
		if ((code & 0x0F) == kByteData) {
			uint32 n = (uint32)hex();
			if (_p < _end && *_p == ',')
				_p++;
			if (n > (uint32)(_end - _p))
				n = _end - _p;
			if (ptr)
				*ptr = _p;
			if (len)
				*len = n;
			_p += n;
			size = n;
			return true;
		}

		_lastCode = code & 0x0F;
		size = (_lastCode == kShortData) ? 2 : 4;
		if (code & 0x80) {
			value = _last;
			if ((code & 0x40) && _p < _end) {
				byte count = *_p++;
				_repeat = count ? count - 1 : 0;
			}
			return true;
		}
		_last = value = hex();
		return true;
	}

	int32 num() {
		int32 v;
		uint32 size;
		next(v, size);
		return v;
	}

	// Unpacked byte size of everything up to the terminator
	uint32 unpackedSize() const {
		PackWalker w = *this;
		uint32 total = 0, size;
		int32 v;
		while (w.next(v, size))
			total += size;
		return total;
	}

private:
	int32 hex() {
		bool negative = false;
		if (_p < _end && *_p == '-') {
			negative = true;
			_p++;
		}
		uint32 result = 0;
		// Like pgUnpackHex: any byte in '0'..'F' is a digit
		while (_p < _end && *_p >= '0' && *_p <= 'F') {
			byte c = *_p++;
			result = (result << 4) | (c <= '9' ? c - '0' : c - 'A' + 10);
		}
		return negative ? -(int32)result : (int32)result;
	}

	const byte *_p, *_end;
	int32 _last = 0;
	int _lastCode = kShortData;
	int _repeat = 0;
};

// Paige file keys (PAIGE.H)
enum {
	kPaigeDocKey = 0,
	kPaigeTextKey = 2,
	kPaigeStyleRunKey = 4,
	kPaigeParRunKey = 5,
	kPaigeStyleInfoKey = 6,
	kPaigeParInfoKey = 7,
	kPaigeFontInfoKey = 8
};

struct PaigeStyle {
	int fontIndex = 0;
	uint16 r = 0, g = 0, b = 0;
	uint16 bkR = 0xffff, bkG = 0xffff, bkB = 0xffff;
	uint16 point = 12;
	byte slant = 0;
};

// Font names are FONT_SIZE (64) byte buffers holding a Pascal string
static Common::String fontNameFromBytes(const byte *ptr, uint32 len) {
	if (!ptr || !len)
		return Common::String();
	if (ptr[0] < len && ptr[0] > 0 && ptr[1] >= 0x20)
		return Common::String((const char *)ptr + 1, ptr[0]);
	uint32 n = 0;
	while (n < len && ptr[n])
		n++;
	return Common::String((const char *)ptr, n);
}

// XMED payload is an ASCII-hex serialized Hermes-Paige document
// (pgSaveDoc): records of a 4-digit hex key, 8-digit hex data size and
// 8-digit hex element count, followed by packed data. The first record
// is the "FFFF" signature.
bool decodeXMED(const Common::Array<byte> &data, XMEDDocument &doc) {
	uint32 size = data.size();
	if (size < 4 || memcmp(data.data(), "FFFF", 4) != 0)
		return false;

	Common::Array<PaigeStyle> styles;
	Common::Array<Common::String> fonts;
	Common::Array<uint32> runOffsets, runStyles;
	bool hasText = false;

	uint32 pos = 0;
	while (pos + 20 <= size) {
		bool valid = true;
		for (uint32 k = 0; k < 20; k++)
			valid = valid && isHexDigit(data[pos + k]);
		if (!valid)
			break;

		uint32 v[3] = {0, 0, 0};
		const uint32 widths[3] = {4, 8, 8};
		uint32 p = pos;
		for (int f = 0; f < 3; f++)
			for (uint32 k = 0; k < widths[f]; k++)
				v[f] = (v[f] << 4) | hexValue(data[p++]);
		int16 key = (int16)v[0];
		uint32 dataSize = v[1];
		uint32 elements = v[2];
		if (dataSize > size - p)
			break;

		PackWalker w(&data[p], dataSize);
		int32 value;
		uint32 itemSize;
		const byte *ptr;
		uint32 len;

		switch (key) {
		case kPaigeDocKey: {
			// paige_rec: version, platform, flags, flags2, pg_type,
			// hdr_ftr_loc[2], resolution, tab_base, doc_top, doc_bottom,
			// unit_h/v, append_h/v, scroll_align_h/v, first/last_word,
			// logical_scroll_pos, scale origin, real_scaling, doc_bounds,
			// then bk_color: the member's background colour
			Common::Array<int32> fields;
			while (fields.size() < 34 && w.next(value, itemSize))
				fields.push_back(value);
			if (fields.size() == 34) {
				doc.bkR = fields[30];
				doc.bkG = fields[31];
				doc.bkB = fields[32];
			}
			break;
		}
		case kPaigeTextKey:
			if (w.next(value, itemSize, &ptr, &len) && ptr) {
				doc.text = Common::String((const char *)ptr, len);
				hasText = true;
			}
			break;
		case kPaigeStyleRunKey:
			for (uint32 i = 0; i < elements && !w.atEnd(); i++) {
				runOffsets.push_back(w.num());
				runStyles.push_back(w.num());
			}
			break;
		case kPaigeStyleInfoKey: {
			w.num();	// insert_style
			if (!elements)
				break;
			// Each style_info is a fixed-size record (pgReadHandlerProc):
			// read it field by field, up to its unpacked size
			uint32 perStyle = w.unpackedSize() / elements;
			for (uint32 i = 0; i < elements && !w.atEnd(); i++) {
				Common::Array<int32> fields;
				uint32 acc = 0;
				while (acc < perStyle && w.next(value, itemSize)) {
					fields.push_back(value);
					acc += itemSize;
				}
				// font_index, char_bytes, max_chars, ascent, descent, leading,
				// shift_verb, class_bits, style_sheet_id, fg_color(4),
				// bk_color(4), char_width, point (16.16), 11 longs, time_stamp,
				// refcon, used_ctr, future[8], styles[32]
				PaigeStyle st;
				if (fields.size() > 45) {
					st.fontIndex = fields[0];
					st.r = fields[9];
					st.g = fields[10];
					st.b = fields[11];
					st.bkR = fields[13];
					st.bkG = fields[14];
					st.bkB = fields[15];
					st.point = (uint16)(fields[18] >> 16);
					// styles[]: bold, italic, underline, outline, shadow,
					// condense, extend, as the Mac style bits
					for (int s = 0; s < 7; s++)
						if (fields[39 + s])
							st.slant |= (1 << s);
				}
				styles.push_back(st);
			}
			break;
		}
		case kPaigeParInfoKey:
			// justification is the first field of the first par_info
			if (elements && !w.atEnd())
				doc.justification = w.num();
			break;
		case kPaigeFontInfoKey: {
			// font_info: name, alternate name, then numbers
			Common::Array<Common::String> names;
			while (w.next(value, itemSize, &ptr, &len)) {
				if (ptr)
					names.push_back(fontNameFromBytes(ptr, len));
			}
			uint32 step = (elements && names.size() >= elements * 2) ? 2 : 1;
			for (uint32 i = 0; i < elements && i * step < names.size(); i++)
				fonts.push_back(names[i * step]);
			break;
		}
		default:
			break;
		}

		pos = p + dataSize;
		if (key == -2)	// pg_eof_key
			break;
	}

	if (!hasText && !findXMEDText(data, doc.text))
		return false;

	doc.runs.clear();
	for (uint32 i = 0; i < runOffsets.size(); i++) {
		if (runOffsets[i] >= doc.text.size() && !doc.runs.empty())
			break;
		StyleRun run;
		run.offset = runOffsets[i];
		if (runStyles[i] < styles.size()) {
			const PaigeStyle &st = styles[runStyles[i]];
			if (st.fontIndex >= 0 && (uint)st.fontIndex < fonts.size())
				run.fontName = fonts[st.fontIndex];
			if (st.point)
				run.fontSize = st.point;
			run.slant = st.slant;
			run.r = st.r;
			run.g = st.g;
			run.b = st.b;
			run.bkR = st.bkR;
			run.bkG = st.bkG;
			run.bkB = st.bkB;
		}
		doc.runs.push_back(run);
	}
	if (!doc.runs.empty())
		doc.runs[0].offset = 0;
	return true;
}

// 76-byte "text" payload observed in Physikus (D7): BE32 height at
// offset 36, width at offset 40, mirroring the authored xtraRect.
bool parseXtraData(const Common::Array<byte> &data, Common::Rect &rect) {
	if (data.size() < 44)
		return false;

	int32 h = READ_BE_INT32(&data[36]);
	int32 w = READ_BE_INT32(&data[40]);
	if (w <= 0 || h <= 0 || w > 0x4000 || h > 0x4000)
		return false;

	rect = Common::Rect((int16)w, (int16)h);
	return true;
}

CastMember *createCastMember(Cast *cast, uint16 castId, XtraCastMember *xtra) {
	return new TextXtraCastMember(cast, castId, *xtra);
}

} // End of namespace TextXtra

TextXtraCastMember::TextXtraCastMember(Cast *cast, uint16 castId, XtraCastMember &source)
		: CastMember(cast, castId) {
	// Not kCastText: the engine casts kCastText members to TextCastMember
	_type = kCastXtra;

	Common::Rect rect;
	if (TextXtra::parseXtraData(source.getXtraData(), rect))
		_initialRect = rect;
}

TextXtraCastMember::TextXtraCastMember(Cast *cast, uint16 castId, TextXtraCastMember &source)
		: CastMember(cast, castId) {
	_type = kCastXtra;
	_initialRect = source._initialRect;
	_boundingRect = source._boundingRect;
	_text = source._text;
	_runs = source._runs;
	_justification = source._justification;
	_bkR = source._bkR;
	_bkG = source._bkG;
	_bkB = source._bkB;
	_color = source._color.clone();
	_loaded = source._loaded;
	if (cast == source._cast)
		_children = source._children;
}

void TextXtraCastMember::load() {
	if (_loaded)
		return;

	for (auto &it : _children) {
		if (it.tag != MKTAG('X', 'M', 'E', 'D'))
			continue;

		if (!_cast->getArchive()->hasResource(it.tag, it.index)) {
			warning("TextXtraCastMember::load(): XMED %d not found", it.index);
			break;
		}
		Common::SeekableReadStreamEndian *r = _cast->getArchive()->getResource(it.tag, it.index);
		Common::Array<byte> data(r->size());
		r->read(data.data(), r->size());
		delete r;

		TextXtra::XMEDDocument doc;
		if (TextXtra::decodeXMED(data, doc)) {
			// Paige keeps the authoring machine's charset: Egypt's Windows
			// release has Mac Roman titles ("Nasser’s", 0xD5)
			_text = Common::U32String(doc.text, _cast->getFileEncoding());
			_runs = doc.runs;
			_justification = doc.justification;
			_bkR = doc.bkR;
			_bkG = doc.bkG;
			_bkB = doc.bkB;
			debugC(3, kDebugText, "TextXtraCastMember::load(): XMED %d: '%s' bk %04x %04x %04x", it.index, doc.text.c_str(), _bkR, _bkG, _bkB);
			for (auto &run : _runs)
				debugC(3, kDebugText, "  run at %d: '%s' %d slant %d color %04x %04x %04x bk %04x %04x %04x",
					run.offset, run.fontName.c_str(), run.fontSize, run.slant, run.r, run.g, run.b, run.bkR, run.bkG, run.bkB);
		} else {
			warning("TextXtraCastMember::load(): XMED %d not decoded", it.index);
		}
		break;
	}

	_loaded = true;
}

TextXtra::StyleRun &TextXtraCastMember::firstRun() {
	load();
	if (_runs.empty())
		_runs.push_back(TextXtra::StyleRun());
	return _runs[0];
}

// Paige stores the authoring machine's font names. Windows names go
// through the movie's cross-platform font map (FXmp, "Arial => Helvetica"),
// like the score's font map does: we only have Mac fonts, and an unmapped
// Arial falls back to a Latin-1 font that draws Mac Roman ’ as Õ
static Common::String mapWindowsFont(Cast *cast, const Common::String &name) {
	Cast *casts[2] = { cast, nullptr };
	Movie *movie = g_director->getCurrentMovie();
	if (movie && movie->getCast() != cast)
		casts[1] = movie->getCast();
	for (Cast *c : casts) {
		if (!c)
			continue;
		for (auto &it : c->_fontXPlatformMap)
			if (it._key.equalsIgnoreCase(name))
				return it._value->toFont;
	}
	return name;
}

static uint16 fontIdForName(Cast *cast, const Common::String &fontName) {
	Graphics::MacFontManager *fontMan = g_director->_wm->_fontMan;
	if (fontName.empty())
		return 1;
	Common::String name = mapWindowsFont(cast, fontName);
	int id = fontMan->getFontIdByName(name);
	// getFontIdByName answers 1 for unknown names too
	if (id == 1 && !fontMan->getFontName(1).equalsIgnoreCase(name))
		id = fontMan->registerFontName(name);
	return id;
}

// Text with MacText format codes, one per style run
Common::U32String TextXtraCastMember::getFormattedText(uint32 spriteColor) {
	firstRun();

	byte sr, sg, sb;
	if (g_director->_wm->_pixelformat.isCLUT8())
		g_director->_wm->getPaletteEntry(spriteColor, sr, sg, sb);
	else
		g_director->_wm->_pixelformat.colorToRGB(spriteColor, sr, sg, sb);

	Common::U32String result;
	for (uint i = 0; i < _runs.size(); i++) {
		const TextXtra::StyleRun &run = _runs[i];
		uint32 start = MIN<uint32>(run.offset, _text.size());
		uint32 end = (i + 1 < _runs.size()) ? MIN<uint32>(_runs[i + 1].offset, _text.size()) : _text.size();
		if (i > 0 && start >= end)
			continue;

		uint16 r = run.r, g = run.g, b = run.b;
		if (_color.type == COLORREF) {
			// the color of member, set from Lingo, colours the whole text
			byte cr, cg, cb;
			_color.u.color->getRGB(cr, cg, cb);
			r = cr * 0x101;
			g = cg * 0x101;
			b = cb * 0x101;
		} else if (!r && !g && !b) {
			// Black text takes the sprite's foreground colour
			// (Bad Milk's grey "skip intro")
			r = sr * 0x101;
			g = sg * 0x101;
			b = sb * 0x101;
		}

		result += Common::U32String(Common::String::format("\001\016%04x%02x%04x%04x%04x%04x",
			fontIdForName(_cast, run.fontName), run.slant, run.fontSize, r, g, b));
		if (start < end)
			result += Common::U32String(_text.begin() + start, _text.begin() + end);
	}
	return result;
}

bool TextXtraCastMember::getOwnBackColor(uint32 &color) {
	load();
	color = g_director->_wm->findBestColor(_bkR >> 8, _bkG >> 8, _bkB >> 8);
	return true;
}

Graphics::MacWidget *TextXtraCastMember::createWidget(Common::Rect &bbox, Channel *channel, SpriteType spriteType) {
	if (!bbox.width() || !bbox.height())
		return nullptr;

	// The text is drawn on the member's own background (Egypt's black top
	// bar titles in copy ink); Background Transparent ink keys out that
	// colour, not the sprite's back colour, see getOwnBackColor (Egypt's
	// white chapter list on a white-backed sprite)
	uint32 fgColor = g_director->_wm->_colorBlack;
	uint32 bgColor;
	getOwnBackColor(bgColor);
	if (channel && channel->_sprite)
		fgColor = channel->_sprite->getForeColor();

	Graphics::TextAlign align = Graphics::kTextAlignLeft;
	switch (_justification) {
	case 1:
		align = Graphics::kTextAlignCenter;
		break;
	case 2:
	case 5:
		align = Graphics::kTextAlignRight;
		break;
	default:
		break;
	}

	Common::U32String text = getFormattedText(fgColor);
	const TextXtra::StyleRun &run = firstRun();
	Graphics::MacFont macFont(fontIdForName(_cast, run.fontName), run.fontSize, run.slant);
	Graphics::MacText *widget = new Graphics::MacText(g_director->getCurrentWindow()->getMacWindow(),
			bbox.left, bbox.top, bbox.width(), bbox.height(), g_director->_wm, text, &macFont,
			fgColor, bgColor, _initialRect.width(), align);
	widget->draw();
	return widget;
}

bool TextXtraCastMember::hasField(int field) {
	switch (field) {
	case kTheColor:
	case kTheText:
	case kTheTextFont:
	case kTheTextSize:
	case kTheTextStyle:
		return true;
	default:
		break;
	}
	return CastMember::hasField(field);
}

static const char *const kSlantNames[] = {
	"bold", "italic", "underline", "outline", "shadow", "condense", "extend"
};

Datum TextXtraCastMember::getField(int field) {
	Datum d;

	switch (field) {
	case kTheCastType:
	case kTheType:
		d = Common::String("text");
		d.type = SYMBOL;
		break;
	case kTheColor:
		if (_color.type == COLORREF) {
			d = _color.clone();
		} else {
			const TextXtra::StyleRun &run = firstRun();
			d = Datum(ColorRef(run.r >> 8, run.g >> 8, run.b >> 8));
		}
		break;
	case kTheText:
		load();
		d = _text.encode(Common::kUtf8);
		break;
	case kTheTextFont:
		d = firstRun().fontName;
		break;
	case kTheTextSize:
		d = firstRun().fontSize;
		break;
	case kTheTextStyle: {
		// D7 text members answer a list of symbols, [#plain] when unstyled
		byte slant = firstRun().slant;
		d.type = ARRAY;
		d.u.farr = new FArray;
		for (int i = 0; i < ARRAYSIZE(kSlantNames); i++) {
			if (slant & (1 << i)) {
				Datum s = Datum(Common::String(kSlantNames[i]));
				s.type = SYMBOL;
				d.u.farr->arr.push_back(s);
			}
		}
		if (d.u.farr->arr.empty()) {
			Datum s(Common::String("plain"));
			s.type = SYMBOL;
			d.u.farr->arr.push_back(s);
		}
		break;
	}
	default:
		d = CastMember::getField(field);
		break;
	}

	return d;
}

// Accepts a list of symbols ([#bold, #italic]) or a string ("bold,italic")
static byte parseSlant(const Datum &d) {
	Common::Array<Common::String> names;
	if (d.type == ARRAY) {
		for (auto &it : d.u.farr->arr)
			names.push_back(it.asString());
	} else {
		Common::String s = d.asString();
		Common::String cur;
		for (uint i = 0; i <= s.size(); i++) {
			if (i == s.size() || s[i] == ',' || s[i] == ' ') {
				if (!cur.empty())
					names.push_back(cur);
				cur.clear();
			} else if (s[i] != '#') {
				cur += s[i];
			}
		}
	}

	byte slant = 0;
	for (auto &name : names) {
		for (int i = 0; i < ARRAYSIZE(kSlantNames); i++)
			if (name.equalsIgnoreCase(kSlantNames[i]))
				slant |= (1 << i);
	}
	return slant;
}

void TextXtraCastMember::setField(int field, const Datum &d) {
	switch (field) {
	case kTheColor:
		if (d.type == COLORREF) {
			_color = d.clone();
		} else {
			// A plain number is a palette index
			_color = Datum(ColorRef(CLIP(d.asInt(), 0, 255)));
		}
		setModified(true);
		return;
	case kTheText: {
		// The new text takes the style of the first character
		TextXtra::StyleRun run = firstRun();
		_runs.clear();
		_runs.push_back(run);
		_text = Common::U32String(d.asString(), Common::kUtf8);
		setModified(true);
		return;
	}
	case kTheTextFont:
	case kTheTextSize:
	case kTheTextStyle: {
		// Setting it on the member restyles the whole text
		firstRun();
		byte slant = (field == kTheTextStyle) ? parseSlant(d) : 0;
		for (auto &run : _runs) {
			if (field == kTheTextFont)
				run.fontName = d.asString();
			else if (field == kTheTextSize)
				run.fontSize = d.asInt();
			else
				run.slant = slant;
		}
		setModified(true);
		return;
	}
	default:
		break;
	}

	CastMember::setField(field, d);
}

Common::String TextXtraCastMember::formatInfo() {
	return Common::String::format("initialRect: %dx%d, text: \"%s\"",
		_initialRect.width(), _initialRect.height(), Common::toPrintable(_text.encode()).c_str());
}

} // End of namespace Director
