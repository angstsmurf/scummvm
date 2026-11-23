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

#ifndef DIRECTOR_LINGO_XTRAS_CAST_TEXTXTRA_H
#define DIRECTOR_LINGO_XTRAS_CAST_TEXTXTRA_H

#include "common/array.h"
#include "common/rect.h"
#include "common/str.h"
#include "common/ustr.h"

#include "director/castmember/castmember.h"

namespace Director {

class XtraCastMember;

namespace TextXtra {

// One style run of the authored text: the style applies from offset
// (in text bytes) up to the next run
struct StyleRun {
	uint32 offset = 0;
	Common::String fontName;
	uint16 fontSize = 12;
	byte slant = 0;			// Mac style bits: bold 1, italic 2, underline 4...
	uint16 r = 0, g = 0, b = 0;	// 16-bit components
	uint16 bkR = 0xffff, bkG = 0xffff, bkB = 0xffff;
};

struct XMEDDocument {
	Common::String text;
	Common::Array<StyleRun> runs;
	int justification = 0;	// Paige: left, center, right, full
	uint16 bkR = 0xffff, bkG = 0xffff, bkB = 0xffff;	// member background
};

CastMember *createCastMember(Cast *cast, uint16 castId, XtraCastMember *xtra);
bool decodeXMED(const Common::Array<byte> &data, XMEDDocument &doc);
bool parseXtraData(const Common::Array<byte> &data, Common::Rect &rect);

} // End of namespace TextXtra

// "Text" Asset Xtra cast member (D7+), display-only in the Projector.
// Successor of the RTE rich text pipeline: both serialize Hermes-Paige
// documents (https://github.com/nmatavka/Hermes-Paige), RTE as binary
// chunks, this one as an ASCII-hex encoded 'XMED' child resource.
class TextXtraCastMember : public CastMember {
public:
	TextXtraCastMember(Cast *cast, uint16 castId, XtraCastMember &source);
	TextXtraCastMember(Cast *cast, uint16 castId, TextXtraCastMember &source);

	CastMember *duplicate(Cast *cast, uint16 castId) override { return (CastMember *)(new TextXtraCastMember(cast, castId, *this)); }

	void load() override;

	Graphics::MacWidget *createWidget(Common::Rect &bbox, Channel *channel, SpriteType spriteType) override;
	bool hasOwnColors() override { return true; }
	bool getOwnBackColor(uint32 &color) override;

	bool hasField(int field) override;
	Datum getField(int field) override;
	void setField(int field, const Datum &value) override;

	Common::String formatInfo() override;

	Common::U32String _text;
	Common::Array<TextXtra::StyleRun> _runs;	// sorted by offset, first at 0
	int _justification = 0;
	uint16 _bkR = 0xffff, _bkG = 0xffff, _bkB = 0xffff;
	Datum _color;	// COLORREF set by Lingo, VOID until then

private:
	TextXtra::StyleRun &firstRun();
	Common::U32String getFormattedText(uint32 spriteColor);
};

} // End of namespace Director

#endif
