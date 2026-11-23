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

#ifndef DIRECTOR_LINGO_XTRAS_CAST_MMBIGIMG_H
#define DIRECTOR_LINGO_XTRAS_CAST_MMBIGIMG_H

#include "common/array.h"
#include "common/str.h"

#include "director/castmember/castmember.h"

namespace Graphics {
struct Surface;
}

namespace Director {

class XtraCastMember;

namespace MMBigImg {

CastMember *createCastMember(Cast *cast, uint16 castId, XtraCastMember *xtra);

} // End of namespace MMBigImg

// "MMBigImg" asset Xtra member (Egypt, Montparnasse Multimedia): a window
// onto a large tiled image, scrolled with MMSetOrg. Egypt's magnifier.
//
// .mbi file, all little-endian uint32:
//   'IBMM', tile count, width, height, tile width, tile height, columns,
//   rows, then per tile (row-major): JPEG size, file offset, width, height
//
// Behaviour follows MMBigImg.x32: the origin is clamped so the view stays
// inside the image; the view is 800x600 until the first draw, then the
// sprite size (at most the image size). Tiles are drawn 1:1.
class MMBigImgCastMember : public CastMember {
public:
	MMBigImgCastMember(Cast *cast, uint16 castId);
	MMBigImgCastMember(Cast *cast, uint16 castId, MMBigImgCastMember &source);
	~MMBigImgCastMember() override;

	CastMember *duplicate(Cast *cast, uint16 castId) override { return (CastMember *)(new MMBigImgCastMember(cast, castId, *this)); }

	Graphics::MacWidget *createWidget(Common::Rect &bbox, Channel *channel, SpriteType spriteType) override;
	bool hasOwnColors() override { return true; }
	Common::Point getRegistrationOffset() override;
	Common::Point getRegistrationOffset(int16 currentWidth, int16 currentHeight) override;

	bool hasProp(const Common::String &propName) override;
	Datum getProp(const Common::String &propName) override;
	void setProp(const Common::String &propName, const Datum &value, bool force = false) override;
	Datum getField(int field) override;
	bool callSpriteMethod(const Common::String &name, const Common::Array<Datum> &args, Datum &result) override;

	Common::String formatInfo() override;

private:
	struct Tile {
		uint32 size = 0, offset = 0;
		int w = 0, h = 0;
		Graphics::Surface *image = nullptr;	// decoded on first use
	};

	void freeImage();
	bool loadFile(const Common::String &fileName);
	void setOrigin(int x, int y);
	void setView(int w, int h);
	void clampOrigin();
	Graphics::Surface *getTile(uint index);

	Common::String _fileName;
	bool _loaded = false;
	int _width = 0, _height = 0;
	uint _cols = 0, _rows = 0;
	Common::Array<Tile> _tiles;
	Common::Array<byte> _data;	// whole file
	int _orgX = 0, _orgY = 0;
	int _viewW = 800, _viewH = 600;
};

} // End of namespace Director

#endif
