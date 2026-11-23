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
#include "common/algorithm.h"
#include "common/hashmap.h"
#include "math/utils.h"
#include "graphics/macgui/macwidget.h"
#include "graphics/macgui/macwindowmanager.h"
#include "director/director.h"
#include "director/cast.h"
#include "director/window.h"
#include "director/castmember/xtra.h"
#include "director/lingo/lingo-the.h"
#include "director/lingo/xtras-cast/vectorshape.h"

namespace Director {

namespace VectorShape {

/*
 * The member data is an 'FLSH' block. Layout as found in Bad Milk (D7),
 * all big-endian; field names follow the property strings of the Flash
 * Asset Xtra, which lists them in reverse storage order:
 *
 *   0x00  'FLSH', block size
 *   0x14  regPoint h, v
 *   0x1c  rect left, top, right, bottom (vertex bounds grown by the stroke)
 *   0x3c  scale (float), then rotation, skew...
 *   0x78  flipH, flipV
 *   0x80  closed
 *   0x84  strokeWidth (float)
 *   0x88  fillMode: 0 #none, 1 #solid, 2 #gradient
 *   0x8c  gradientType: 0 #linear, 1 #radial
 *   0x90  fillScale (float), 0x94 fillDirection, 0x98 fillOffset h, v
 *   0xa0  fillCycles
 *   0xa4  strokeColor, fillColor, backgroundColor, endColor, vertexList
 *         as serialized Lingo values
 *
 * Serialized values: a 4-byte type, then
 *   2    symbol: length + chars; length 0x80000000|n repeats symbol n
 *   7    list: count + values
 *   8    point: h, v
 *   0xa  property list: count + (symbol, value) pairs
 *   0x12 rgb: r, g, b
 */

enum {
	kTypeSymbol = 2,
	kTypeList = 7,
	kTypePoint = 8,
	kTypePropList = 0xa,
	kTypeRGB = 0x12
};

struct Value {
	uint32 type = 0;
	Common::String symbol;
	Common::Point point;
	uint32 rgb = 0;
	Common::Array<Value> items;			// list items, or proplist values
	Common::Array<Common::String> keys;	// proplist keys
};

class Reader {
public:
	Reader(const Common::Array<byte> &data, uint32 pos) : _data(data), _pos(pos) {}

	bool readUint32(uint32 &v) {
		if (_pos + 4 > _data.size())
			return false;
		v = READ_BE_UINT32(&_data[_pos]);
		_pos += 4;
		return true;
	}

	bool readSymbol(Common::String &s) {
		uint32 len;
		if (!readUint32(len))
			return false;
		if (len & 0x80000000) {
			len &= 0x7fffffff;
			if (len >= _symbols.size())
				return false;
			s = _symbols[len];
			return true;
		}
		if (_pos + len > _data.size())
			return false;
		s = Common::String((const char *)&_data[_pos], len);
		_pos += len;
		_symbols.push_back(s);
		return true;
	}

	bool readValue(Value &v, int depth = 0) {
		if (depth > 8 || !readUint32(v.type))
			return false;
		uint32 a, b, c;
		switch (v.type) {
		case kTypeSymbol:
			return readSymbol(v.symbol);
		case kTypePoint:
			if (!readUint32(a) || !readUint32(b))
				return false;
			v.point = Common::Point((int32)a, (int32)b);
			return true;
		case kTypeRGB:
			if (!readUint32(a) || !readUint32(b) || !readUint32(c))
				return false;
			v.rgb = ((a & 0xff) << 16) | ((b & 0xff) << 8) | (c & 0xff);
			return true;
		case kTypeList:
		case kTypePropList: {
			uint32 count;
			if (!readUint32(count) || count > 0x10000)
				return false;
			for (uint32 i = 0; i < count; i++) {
				if (v.type == kTypePropList) {
					Common::String key;
					uint32 keyType;
					if (!readUint32(keyType) || keyType != kTypeSymbol || !readSymbol(key))
						return false;
					v.keys.push_back(key);
				}
				Value item;
				if (!readValue(item, depth + 1))
					return false;
				v.items.push_back(item);
			}
			return true;
		}
		default:
			warning("VectorShape: unknown serialized value type %d at 0x%x", v.type, _pos - 4);
			return false;
		}
	}

private:
	const Common::Array<byte> &_data;
	uint32 _pos;
	Common::Array<Common::String> _symbols;
};

static bool readColor(Reader &r, uint32 &color) {
	Value v;
	if (!r.readValue(v) || v.type != kTypeRGB)
		return false;
	color = v.rgb;
	return true;
}

static float readFloat(const Common::Array<byte> &data, uint32 pos) {
	uint32 bits = READ_BE_UINT32(&data[pos]);
	float f;
	memcpy(&f, &bits, sizeof(f));
	return f;
}

bool parseXtraData(const Common::Array<byte> &data, Info &info) {
	if (data.size() < 0xa4 || READ_BE_UINT32(&data[0]) != MKTAG('F', 'L', 'S', 'H'))
		return false;

	info.regPoint = Common::Point(READ_BE_INT32(&data[0x14]), READ_BE_INT32(&data[0x18]));
	info.rect = Common::Rect(READ_BE_INT32(&data[0x1c]), READ_BE_INT32(&data[0x20]),
		READ_BE_INT32(&data[0x24]), READ_BE_INT32(&data[0x28]));
	if (!info.rect.isValidRect() || info.rect.width() > 0x4000 || info.rect.height() > 0x4000)
		return false;

	info.closed = READ_BE_UINT32(&data[0x80]) != 0;
	info.strokeWidth = readFloat(data, 0x84);
	info.fillMode = READ_BE_UINT32(&data[0x88]);
	info.gradientType = READ_BE_UINT32(&data[0x8c]);
	info.fillScale = readFloat(data, 0x90);
	info.fillDirection = READ_BE_INT32(&data[0x94]);
	info.fillOffset = Common::Point(READ_BE_INT32(&data[0x98]), READ_BE_INT32(&data[0x9c]));
	info.fillCycles = READ_BE_INT32(&data[0xa0]);
	if (!(info.strokeWidth >= 0.0f && info.strokeWidth < 1000.0f))
		info.strokeWidth = 1.0f;

	Reader r(data, 0xa4);
	if (!readColor(r, info.strokeColor) || !readColor(r, info.fillColor) ||
			!readColor(r, info.backgroundColor) || !readColor(r, info.endColor))
		return false;

	Value list;
	if (!r.readValue(list) || list.type != kTypeList)
		return false;

	bool newCurve = false;
	for (auto &item : list.items) {
		// [#newCurve] separates subpaths
		if (item.type == kTypeList && item.items.size() == 1 && item.items[0].type == kTypeSymbol &&
				item.items[0].symbol.equalsIgnoreCase("newCurve")) {
			newCurve = true;
			continue;
		}
		if (item.type != kTypePropList)
			continue;

		Vertex vtx;
		bool hasVertex = false;
		for (uint i = 0; i < item.keys.size(); i++) {
			const Common::String &key = item.keys[i];
			if (key.equalsIgnoreCase("newCurve")) {
				newCurve = true;
				continue;
			}
			if (item.items[i].type != kTypePoint)
				continue;
			if (key.equalsIgnoreCase("vertex")) {
				vtx.vertex = item.items[i].point;
				hasVertex = true;
			} else if (key.equalsIgnoreCase("handle1")) {
				vtx.handle1 = item.items[i].point;
			} else if (key.equalsIgnoreCase("handle2")) {
				vtx.handle2 = item.items[i].point;
			}
		}
		if (!hasVertex)
			continue;
		vtx.newCurve = newCurve;
		newCurve = false;
		info.vertices.push_back(vtx);
	}
	return true;
}

CastMember *createCastMember(Cast *cast, uint16 castId, XtraCastMember *xtra) {
	return new VectorShapeCastMember(cast, castId, *xtra);
}

} // End of namespace VectorShape

VectorShapeCastMember::VectorShapeCastMember(Cast *cast, uint16 castId, XtraCastMember &source)
		: CastMember(cast, castId) {
	_type = kCastXtra;
	_valid = VectorShape::parseXtraData(source.getXtraData(), _info);
	if (_valid) {
		_initialRect = _info.rect;
		debugC(3, kDebugLoading, "VectorShapeCastMember: %s", formatInfo().c_str());
	} else {
		warning("VectorShapeCastMember: could not parse vector shape %d", castId);
	}
}

VectorShapeCastMember::VectorShapeCastMember(Cast *cast, uint16 castId, VectorShapeCastMember &source)
		: CastMember(cast, castId) {
	_type = kCastXtra;
	_initialRect = source._initialRect;
	_boundingRect = source._boundingRect;
	_info = source._info;
	_valid = source._valid;
	_loaded = source._loaded;
	if (cast == source._cast)
		_children = source._children;
}

Common::Point VectorShapeCastMember::getRegistrationOffset() {
	return Common::Point(_info.regPoint.x - _initialRect.left, _info.regPoint.y - _initialRect.top);
}

Common::Point VectorShapeCastMember::getRegistrationOffset(int16 currentWidth, int16 currentHeight) {
	Common::Point offset = getRegistrationOffset();
	if (!_initialRect.width() || !_initialRect.height())
		return offset;
	return Common::Point(offset.x * currentWidth / _initialRect.width(), offset.y * currentHeight / _initialRect.height());
}

namespace {

struct FPoint {
	float x, y;
};

static uint32 lerpColor(uint32 a, uint32 b, float t) {
	t = CLIP(t, 0.0f, 1.0f);
	uint32 res = 0;
	for (int shift = 0; shift <= 16; shift += 8) {
		int ca = (a >> shift) & 0xff;
		int cb = (b >> shift) & 0xff;
		res |= (uint32)(ca + (cb - ca) * t + 0.5f) << shift;
	}
	return res;
}

class ColorMapper {
public:
	uint32 map(uint32 rgb) {
		if (_cache.contains(rgb))
			return _cache[rgb];
		Graphics::MacWindowManager *wm = g_director->_wm;
		byte r = (rgb >> 16) & 0xff, g = (rgb >> 8) & 0xff, b = rgb & 0xff;
		uint32 c = wm->_pixelformat.bytesPerPixel == 1 ? wm->findBestColor(r, g, b) : wm->_pixelformat.RGBToColor(r, g, b);
		_cache[rgb] = c;
		return c;
	}
private:
	Common::HashMap<uint32, uint32> _cache;
};

} // End of anonymous namespace

Graphics::MacWidget *VectorShapeCastMember::createWidget(Common::Rect &bbox, Channel *channel, SpriteType spriteType) {
	if (!bbox.width() || !bbox.height())
		return nullptr;

	Graphics::MacWidget *widget = new Graphics::MacWidget(g_director->getCurrentWindow()->getMacWindow(),
		bbox.left, bbox.top, bbox.width(), bbox.height(), g_director->_wm, false);
	Graphics::ManagedSurface *surf = widget->getSurface();
	ColorMapper colors;
	if (g_director->_wm->_pixelformat.bytesPerPixel == 1) {
		memcpy(_builtPalette, g_director->getPalette(), sizeof(_builtPalette));
		_hasBuiltPalette = true;
	}
	surf->fillRect(Common::Rect(bbox.width(), bbox.height()), colors.map(_info.backgroundColor));

	if (!_valid || _info.vertices.empty() || !_initialRect.width() || !_initialRect.height())
		return widget;

	// Vertices are relative to the centre of their bounds, which sits at
	// the centre of the member rect; the sprite stretches the member rect.
	int minX = _info.vertices[0].vertex.x, maxX = minX;
	int minY = _info.vertices[0].vertex.y, maxY = minY;
	for (auto &v : _info.vertices) {
		minX = MIN<int>(minX, v.vertex.x);
		maxX = MAX<int>(maxX, v.vertex.x);
		minY = MIN<int>(minY, v.vertex.y);
		maxY = MAX<int>(maxY, v.vertex.y);
	}
	float sx = (float)bbox.width() / _initialRect.width();
	float sy = (float)bbox.height() / _initialRect.height();
	float cx = (minX + maxX) / 2.0f, cy = (minY + maxY) / 2.0f;
	auto toPixel = [&](float x, float y) {
		FPoint p;
		p.x = (x - cx) * sx + bbox.width() / 2.0f;
		p.y = (y - cy) * sy + bbox.height() / 2.0f;
		return p;
	};

	// Flatten the path into polylines, one per subpath
	Common::Array<Common::Array<FPoint> > paths;
	uint n = _info.vertices.size();
	uint start = 0;
	while (start < n) {
		uint end = start + 1;
		while (end < n && !_info.vertices[end].newCurve)
			end++;

		Common::Array<FPoint> path;
		uint segs = (_info.closed && end - start > 2) ? end - start : end - start - 1;
		path.push_back(toPixel(_info.vertices[start].vertex.x, _info.vertices[start].vertex.y));
		for (uint s = 0; s < segs; s++) {
			const VectorShape::Vertex &a = _info.vertices[start + s];
			const VectorShape::Vertex &b = _info.vertices[start + (s + 1) % (end - start)];
			// handle1/handle2 are offsets from their vertex; the curve
			// leaves a towards a + handle2 and enters b from b + handle1.
			// TODO: verify the handle roles against the Flash Asset Xtra
			bool curved = a.handle2.x || a.handle2.y || b.handle1.x || b.handle1.y;
			if (!curved) {
				path.push_back(toPixel(b.vertex.x, b.vertex.y));
				continue;
			}
			float p0x = a.vertex.x, p0y = a.vertex.y;
			float p1x = a.vertex.x + a.handle2.x, p1y = a.vertex.y + a.handle2.y;
			float p2x = b.vertex.x + b.handle1.x, p2y = b.vertex.y + b.handle1.y;
			float p3x = b.vertex.x, p3y = b.vertex.y;
			const int steps = 16;
			for (int k = 1; k <= steps; k++) {
				float t = (float)k / steps, u = 1.0f - t;
				float x = u * u * u * p0x + 3 * u * u * t * p1x + 3 * u * t * t * p2x + t * t * t * p3x;
				float y = u * u * u * p0y + 3 * u * u * t * p1y + 3 * u * t * t * p2y + t * t * t * p3y;
				path.push_back(toPixel(x, y));
			}
		}
		paths.push_back(path);
		start = end;
	}

	// Fill: even-odd rule, sampled at pixel centres
	if (_info.closed && _info.fillMode != VectorShape::kFillNone) {
		float w = bbox.width(), h = bbox.height();
		float dirRad = _info.fillDirection * (float)M_PI / 180.0f;
		float dx = cosf(dirRad), dy = sinf(dirRad);
		float gcx = w / 2.0f + _info.fillOffset.x * sx, gcy = h / 2.0f + _info.fillOffset.y * sy;
		float scale = _info.fillScale > 0.0f ? _info.fillScale / 100.0f : 1.0f;
		float halfLen = (fabsf(dx) * w + fabsf(dy) * h) / 2.0f * scale;
		float radius = sqrtf(w * w + h * h) / 2.0f * scale;
		int cycles = MAX(1, _info.fillCycles);

		for (int y = 0; y < bbox.height(); y++) {
			float py = y + 0.5f;
			Common::Array<float> xs;
			for (auto &path : paths) {
				uint m = path.size();
				for (uint i = 0; i < m; i++) {
					const FPoint &a = path[i];
					const FPoint &b = path[(i + 1) % m];
					if ((a.y <= py) == (b.y <= py))
						continue;
					xs.push_back(a.x + (py - a.y) * (b.x - a.x) / (b.y - a.y));
				}
			}
			Common::sort(xs.begin(), xs.end());
			for (uint i = 0; i + 1 < xs.size(); i += 2) {
				int x0 = MAX(0, (int)ceilf(xs[i] - 0.5f));
				int x1 = MIN(bbox.width() - 1, (int)ceilf(xs[i + 1] - 0.5f) - 1);
				for (int x = x0; x <= x1; x++) {
					uint32 rgb = _info.fillColor;
					if (_info.fillMode == VectorShape::kFillGradient) {
						float t;
						if (_info.gradientType == 1) {
							float ox = x + 0.5f - gcx, oy = py - gcy;
							t = sqrtf(ox * ox + oy * oy) / radius;
						} else {
							t = (((x + 0.5f - gcx) * dx + (py - gcy) * dy) / halfLen + 1.0f) / 2.0f;
						}
						t = CLIP(t, 0.0f, 1.0f) * cycles;
						int cycle = MIN((int)t, cycles - 1);
						t -= cycle;
						if (cycle & 1)
							t = 1.0f - t;
						rgb = lerpColor(_info.fillColor, _info.endColor, t);
					}
					surf->setPixel(x, y, colors.map(rgb));
				}
			}
		}
	}

	// Stroke
	if (_info.strokeWidth > 0.0f) {
		int pen = MAX(1, (int)(_info.strokeWidth * MIN(sx, sy) + 0.5f));
		uint32 color = colors.map(_info.strokeColor);
		for (auto &path : paths) {
			for (uint i = 0; i + 1 < path.size(); i++) {
				int x0 = (int)floorf(path[i].x), y0 = (int)floorf(path[i].y);
				int x1 = (int)floorf(path[i + 1].x), y1 = (int)floorf(path[i + 1].y);
				int off = (pen - 1) / 2;
				surf->drawThickLine(x0 - off, y0 - off, x1 - off, y1 - off, pen, pen, color);
			}
		}
	}

	return widget;
}

bool VectorShapeCastMember::isModified() {
	if (CastMember::isModified())
		return true;
	return _hasBuiltPalette && memcmp(_builtPalette, g_director->getPalette(), sizeof(_builtPalette)) != 0;
}

Datum VectorShapeCastMember::getField(int field) {
	Datum d;

	switch (field) {
	case kTheCastType:
	case kTheType:
		d = Common::String("vectorShape");
		d.type = SYMBOL;
		break;
	default:
		d = CastMember::getField(field);
		break;
	}

	return d;
}

Common::String VectorShapeCastMember::formatInfo() {
	return Common::String::format("rect: %d,%d,%d,%d, reg: %d,%d, %d vertices, closed: %d, stroke: %.1f #%06x, fillMode: %d #%06x->#%06x, bg #%06x",
		_info.rect.left, _info.rect.top, _info.rect.right, _info.rect.bottom,
		_info.regPoint.x, _info.regPoint.y, _info.vertices.size(), _info.closed,
		_info.strokeWidth, _info.strokeColor, _info.fillMode, _info.fillColor, _info.endColor, _info.backgroundColor);
}

} // End of namespace Director
