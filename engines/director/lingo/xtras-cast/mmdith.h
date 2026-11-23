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

#ifndef DIRECTOR_LINGO_XTRAS_CAST_MMDITH_H
#define DIRECTOR_LINGO_XTRAS_CAST_MMDITH_H

#include "common/str.h"

#include "director/castmember/castmember.h"

namespace Graphics {
struct Surface;
}

namespace Director {

class XtraCastMember;

namespace MMDith {

CastMember *createCastMember(Cast *cast, uint16 castId, XtraCastMember *xtra);

} // End of namespace MMDith

// "MMDith" asset Xtra member (Egypt, Montparnasse Multimedia): shows the
// JPEG named by the sprite property mmDithFileName, dithered to the
// screen, centred on the sprite's loc. MMDithWidth and MMDithHeight
// report the image size; Lingo sizes the sprite with them.
class MMDithCastMember : public CastMember {
public:
	MMDithCastMember(Cast *cast, uint16 castId, XtraCastMember &source);
	MMDithCastMember(Cast *cast, uint16 castId, MMDithCastMember &source);
	~MMDithCastMember() override;

	CastMember *duplicate(Cast *cast, uint16 castId) override { return (CastMember *)(new MMDithCastMember(cast, castId, *this)); }

	Graphics::MacWidget *createWidget(Common::Rect &bbox, Channel *channel, SpriteType spriteType) override;

	Common::Point getRegistrationOffset() override;
	Common::Point getRegistrationOffset(int16 currentWidth, int16 currentHeight) override;

	bool hasOwnColors() override { return true; }

	bool hasProp(const Common::String &propName) override;
	Datum getProp(const Common::String &propName) override;
	void setProp(const Common::String &propName, const Datum &value, bool force = false) override;
	Datum getField(int field) override;

	Common::String formatInfo() override;

private:
	void loadImage(const Common::String &fileName);

	Common::String _fileName;
	Graphics::Surface *_image = nullptr;	// RGB, owned
};

} // End of namespace Director

#endif
