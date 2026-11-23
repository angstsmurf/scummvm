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

#ifndef DIRECTOR_LINGO_XTRAS_CAST_VECTORSHAPE_H
#define DIRECTOR_LINGO_XTRAS_CAST_VECTORSHAPE_H

#include "common/array.h"
#include "common/rect.h"

#include "director/castmember/castmember.h"

namespace Director {

class XtraCastMember;

namespace VectorShape {

struct Vertex {
	Common::Point vertex;
	Common::Point handle1;
	Common::Point handle2;
	bool newCurve = false;	// a [#newCurve] entry starts a new subpath here
};

enum FillMode {
	kFillNone = 0,
	kFillSolid = 1,
	kFillGradient = 2
};

struct Info {
	Common::Point regPoint;
	Common::Rect rect;
	bool closed = true;
	float strokeWidth = 1.0f;
	int fillMode = kFillSolid;
	int gradientType = 0;		// 0 #linear, 1 #radial
	float fillScale = 100.0f;
	int fillDirection = 0;
	Common::Point fillOffset;
	int fillCycles = 1;
	uint32 strokeColor = 0;		// 0xRRGGBB
	uint32 fillColor = 0xffffff;
	uint32 backgroundColor = 0xffffff;
	uint32 endColor = 0;
	Common::Array<Vertex> vertices;
};

CastMember *createCastMember(Cast *cast, uint16 castId, XtraCastMember *xtra);
bool parseXtraData(const Common::Array<byte> &data, Info &info);

} // End of namespace VectorShape

// "vectorShape" member of the Flash Asset Xtra (D7+): a path of straight
// and Bezier segments, stroked and filled, drawn at the sprite's size.
class VectorShapeCastMember : public CastMember {
public:
	VectorShapeCastMember(Cast *cast, uint16 castId, XtraCastMember &source);
	VectorShapeCastMember(Cast *cast, uint16 castId, VectorShapeCastMember &source);

	CastMember *duplicate(Cast *cast, uint16 castId) override { return (CastMember *)(new VectorShapeCastMember(cast, castId, *this)); }

	Graphics::MacWidget *createWidget(Common::Rect &bbox, Channel *channel, SpriteType spriteType) override;

	Common::Point getRegistrationOffset() override;
	Common::Point getRegistrationOffset(int16 currentWidth, int16 currentHeight) override;

	bool isModified() override;
	bool hasOwnColors() override { return true; }

	Datum getField(int field) override;

	Common::String formatInfo() override;

	VectorShape::Info _info;
	bool _valid = false;

private:
	// Palette the colours were matched against (8-bit only): a palette
	// change has to redo the matching.
	byte _builtPalette[768];
	bool _hasBuiltPalette = false;
};

} // End of namespace Director

#endif
