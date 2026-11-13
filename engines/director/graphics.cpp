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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common/system.h"
#include "common/memstream.h"
#include "graphics/macgamma.h"
#include "graphics/paletteman.h"
#include "graphics/macgui/macwindowmanager.h"

#include "director/director.h"
#include "director/cast.h"
#include "director/movie.h"
#include "director/images.h"
#include "director/picture.h"
#include "director/window.h"
#include "director/castmember/bitmap.h"

namespace Director {

#include "director/graphics-data.h"

/**
 * Used to upgrade to 32-bit color if required.
 **/
uint32 DirectorEngine::transformColor(uint32 color) {
	if (_pixelformat.bytesPerPixel == 1)
		return color;

	// A palette index can never exceed 0xff (max 256 colors); a larger value
	// must be a packed RGB int from a Lingo color assignment (e.g. rgb()).
	if (color > 0xff)
		return _wm->findBestColor((color >> 16) & 0xff, (color >> 8) & 0xff, color & 0xff);

	return _wm->findBestColor(_currentPalette[color * 3], _currentPalette[color * 3 + 1], _currentPalette[color * 3 + 2]);
}

/**
 * Turns a color from transformColor() back into a palette index, for
 * returning to Lingo.
 */
uint32 DirectorEngine::reverseTransformColor(uint32 color) {
	if (_pixelformat.bytesPerPixel == 1)
		return color;

	byte r, g, b;
	_pixelformat.colorToRGB(color, r, g, b);

	uint32 best = 0;
	uint32 bestDistance = 0xffffffff;
	for (uint32 i = 0; i < 256; i++) {
		int dr = _currentPalette[i * 3] - r;
		int dg = _currentPalette[i * 3 + 1] - g;
		int db = _currentPalette[i * 3 + 2] - b;
		uint32 distance = dr * dr + dg * dg + db * db;
		if (distance < bestDistance) {
			best = i;
			bestDistance = distance;
			if (distance == 0)
				break;
		}
	}

	return best;
}

void DirectorEngine::loadPatterns() {
	for (int i = 0; i < ARRAYSIZE(director3Patterns); i++)
		_director3Patterns.push_back(director3Patterns[i]);

	for (int i = 0; i < ARRAYSIZE(director3QuickDrawPatterns); i++)
		_director3QuickDrawPatterns.push_back(director3QuickDrawPatterns[i]);

	// We must set it here for correct work of BITDDecoder.
	// It is set later in Director properly
	_pixelformat = Graphics::PixelFormat::createFormatCLUT8();

	for (int i = 0; i < ARRAYSIZE(builtinTiles); i++) {
		Common::MemoryReadStream stream(builtinTiles[i].ptr, builtinTiles[i].size);

		auto decoder = new BITDDecoder(builtinTiles[i].w, builtinTiles[i].h, 8, builtinTiles[i].w, macPalette, kFileVer300);
		decoder->loadStream(stream);
		_builtinTiles[i].img = new Picture(*decoder);
		delete decoder;

		_builtinTiles[i].rect = Common::Rect(0, 0, builtinTiles[i].w, builtinTiles[i].h);
	}
}

Graphics::MacPatterns &DirectorEngine::getPatterns() {
	// TOOD: implement switch and other version patterns. (use getVersion());
	return _director3QuickDrawPatterns;
}

Picture *DirectorEngine::getTile(int num) {
	TilePatternEntry *tile = &getCurrentMovie()->getCast()->_tiles[num];

	if (tile->bitmapId.isNull())
		return _builtinTiles[num].img;

	CastMember *member = getCurrentMovie()->getCastMember(tile->bitmapId);

	if (!member) {
		warning("BUILDBOT: DirectorEngine::getTile(%d) VWTL refers to non-existing cast %s", num,
				tile->bitmapId.asString().c_str());

		return _builtinTiles[num].img;
	}

	if (member->_type != kCastBitmap) {
		warning("BUILDBOT: DirectorEngine::getTile(%d) VWTL refers to incorrect cast %s type %s", num,
				tile->bitmapId.asString().c_str(), castType2str(member->_type));

		return _builtinTiles[num].img;
	}

	return ((BitmapCastMember *)member)->_picture;
}

const Common::Rect &DirectorEngine::getTileRect(int num) {
	TilePatternEntry *tile = &getCurrentMovie()->getCast()->_tiles[num];

	if (tile->bitmapId.isNull())
		return _builtinTiles[num].rect;

	return tile->rect;
}

void DirectorEngine::loadDefaultPalettes() {
	_loadedPalettes[CastMemberID(kClutSystemMac, -1)] = PaletteV4(CastMemberID(kClutSystemMac, -1), macPalette, 256);
	_loadedPalettes[CastMemberID(kClutRainbow, -1)] = PaletteV4(CastMemberID(kClutRainbow, -1), rainbowPalette, 256);
	_loadedPalettes[CastMemberID(kClutGrayscale, -1)] = PaletteV4(CastMemberID(kClutGrayscale, -1), grayscalePalette, 256);
	_loadedPalettes[CastMemberID(kClutPastels, -1)] = PaletteV4(CastMemberID(kClutPastels, -1), pastelsPalette, 256);
	_loadedPalettes[CastMemberID(kClutVivid, -1)] = PaletteV4(CastMemberID(kClutVivid, -1), vividPalette, 256);
	_loadedPalettes[CastMemberID(kClutNTSC, -1)] = PaletteV4(CastMemberID(kClutNTSC, -1), ntscPalette, 256);
	_loadedPalettes[CastMemberID(kClutMetallic, -1)] = PaletteV4(CastMemberID(kClutMetallic, -1), metallicPalette, 256);
	_loadedPalettes[CastMemberID(kClutSystemWin, -1)] = PaletteV4(CastMemberID(kClutSystemWin, -1), winPalette, 256);
	_loadedPalettes[CastMemberID(kClutSystemWinD5, -1)] = PaletteV4(CastMemberID(kClutSystemWinD5, -1), winD5Palette, 256);

	_loaded16Palettes[CastMemberID(kClutSystemMac, -1)] = PaletteV4(CastMemberID(kClutSystemMac, -1), mac16Palette, 16);
	_loaded16Palettes[CastMemberID(kClutRainbow, -1)] = PaletteV4(CastMemberID(kClutRainbow, -1), rainbow16Palette, 16);
	_loaded16Palettes[CastMemberID(kClutGrayscale, -1)] = PaletteV4(CastMemberID(kClutGrayscale, -1), grayscale16Palette, 16);
	_loaded16Palettes[CastMemberID(kClutPastels, -1)] = PaletteV4(CastMemberID(kClutPastels, -1), pastels16Palette, 16);
	_loaded16Palettes[CastMemberID(kClutVivid, -1)] = PaletteV4(CastMemberID(kClutVivid, -1), vivid16Palette, 16);
	_loaded16Palettes[CastMemberID(kClutNTSC, -1)] = PaletteV4(CastMemberID(kClutNTSC, -1), ntsc16Palette, 16);
	_loaded16Palettes[CastMemberID(kClutMetallic, -1)] = PaletteV4(CastMemberID(kClutMetallic, -1), metallic16Palette, 16);
	_loaded16Palettes[CastMemberID(kClutSystemWin, -1)] = PaletteV4(CastMemberID(kClutSystemWin, -1), win16Palette, 16);
	_loaded16Palettes[CastMemberID(kClutSystemWinD5, -1)] = PaletteV4(CastMemberID(kClutSystemWinD5, -1), winD516Palette, 16);

	_loaded4Palette = PaletteV4(CastMemberID(kClutGrayscale, -1), grayscale4Palette, 4);
}

PaletteV4 *DirectorEngine::getPalette(CastMemberID id) {
	if (id.isNull())
		return nullptr;

	// Reference to internal palettes
	if (id.member < 0)
		id.castLib = -1; // Ensure we use the default palette set

	if (!_loadedPalettes.contains(id)) {
		warning("DirectorEngine::getPalette(): Palette %s not found, hash %x", id.asString().c_str(), id.hash());
		return nullptr;
	}

	return &_loadedPalettes[id];
}

bool DirectorEngine::hasPalette(CastMemberID id) {
	// Reference to internal palettes
	if (id.member < 0)
		id.castLib = -1; // Ensure we use the default palette set

	return _loadedPalettes.contains(id);
}

void DirectorEngine::addPalette(CastMemberID &id, const byte *palette, int length) {
	if (id.castLib < 0) {
		warning("DirectorEngine::addPalette(): Negative cast library ids reserved for default palettes");
		return;
	} else if (_loadedPalettes.contains(id)) {
		delete[] _loadedPalettes[id].palette;
	}

	debugC(3, kDebugLoading, "DirectorEngine::addPalette(): Registered palette %s of size %d, hash: %x", id.asString().c_str(), length, id.hash());
	byte *palCopy = new byte[length * 3]; // freed by clearPalettes()
	memcpy(palCopy, palette, length * 3);

	_loadedPalettes[id] = PaletteV4(id, palCopy, length);
}

bool DirectorEngine::setPalette(const CastMemberID &id) {
	if (id.isNull()) {
		// Palette id of 0 is unused
		return false;
	}

	PaletteV4 *pal = getPalette(id);
	if (!pal)
		return false;
	debugC(5, kDebugImages, "DirectorEngine::setPalettes(): setting palette %d, %d", id.member, id.castLib);
	setPalette(pal->palette, pal->length);
	return true;
}

namespace {

struct InverseBox {
	byte index;
	bool active;
	bool faceActive[6];	// rMin, rMax, gMin, gMax, bMin, bMax
	byte face[6];
};

byte inverseCell(byte c) {
	return MIN(c + 4, 255) >> 3;
}

bool claimInverseCells(Common::Array<byte> &table, Common::Array<bool> &claimed, byte index, int r0, int r1, int g0, int g1, int b0, int b1) {
	bool got = false;
	for (int r = r0; r <= r1; r++)
		for (int g = g0; g <= g1; g++)
			for (int b = b0; b <= b1; b++) {
				int k = (r << 10) | (g << 5) | b;
				if (!claimed[k]) {
					claimed[k] = true;
					table[k] = index;
					got = true;
				}
			}
	return got;
}


// The points of an oval as Windows Director plots them: one every 4 degrees,
// clockwise from the top, around the box with the bottom and right edges
// pulled in by shrink pixels. inset shrinks both radii.
void windowsOvalPoints(const Common::Rect &rect, int shrink, int inset, Common::Array<Common::Point> &points) {
	double rx = (rect.right - shrink - rect.left) * 0.5;
	double ry = (rect.bottom - shrink - rect.top) * 0.5;
	double cx = rect.left + rx;
	double cy = rect.top + ry;
	rx -= inset;
	ry -= inset;
	for (int a = 0; a <= 360; a += 4) {
		int q = a % 90;
		// Director's x87 table has sin 30 and cos 60 at exactly 0.5; a point
		// can land on a .5 there
		double s = q == 30 ? 0.5 : q ? sin(q * M_PI / 180.0) : 0.0;
		double c = q == 60 ? 0.5 : q ? cos(q * M_PI / 180.0) : 1.0;
		double sn, cs;
		switch (a / 90 % 4) {
		case 0: sn = s; cs = c; break;
		case 1: sn = c; cs = -s; break;
		case 2: sn = -s; cs = -c; break;
		default: sn = -c; cs = s; break;
		}
		points.push_back(Common::Point((int)(sn * rx + cx + 0.5), (int)(cy - cs * ry + 0.5)));
	}
}

// Fills polygons like GDI does: alternate rule, sampled at whole scanlines,
// from the first pixel at or right of an edge to the last one left of the next
void fillWindowsPolygons(Graphics::Primitives &primitives, const Common::Array<Common::Point> *polys, int numPolys, uint32 color, void *data) {
	int yMin = polys[0][0].y, yMax = yMin;
	for (int i = 0; i < numPolys; i++)
		for (auto &pt : polys[i]) {
			yMin = MIN<int>(yMin, pt.y);
			yMax = MAX<int>(yMax, pt.y);
		}
	Common::Array<double> xs;
	for (int y = yMin; y < yMax; y++) {
		xs.clear();
		for (int i = 0; i < numPolys; i++) {
			const Common::Array<Common::Point> &p = polys[i];
			for (uint j = 0; j < p.size(); j++) {
				const Common::Point &p0 = p[j];
				const Common::Point &p1 = p[(j + 1) % p.size()];
				if ((p0.y <= y && y < p1.y) || (p1.y <= y && y < p0.y))
					xs.push_back(p0.x + (double)(y - p0.y) * (p1.x - p0.x) / (p1.y - p0.y));
			}
		}
		Common::sort(xs.begin(), xs.end());
		for (uint j = 0; j + 1 < xs.size(); j += 2) {
			int x0 = (int)ceil(xs[j]);
			int x1 = (int)ceil(xs[j + 1]) - 1;
			if (x0 <= x1)
				primitives.drawHLine(x0, x1, y, color, data);
		}
	}
}

// Rounds num / den (den > 0) to the nearest integer; up decides the halves
int roundWindowsLine(int num, int den, bool up) {
	int q = num >= 0 ? num / den : -((-num + den - 1) / den);
	int rem2 = 2 * (num - q * den);
	if (rem2 > den || (rem2 == den && up))
		q++;
	return q;
}

// A line as the Windows GDI draws it with a one pixel pen, from a to b without
// b: it steps from the upper end, and a halfway minor coordinate goes the way
// the line heads
void drawWindowsLine(Graphics::Primitives &primitives, Common::Point a, Common::Point b, uint32 color, void *data) {
	Common::Point end = b;
	if (b.y < a.y)
		SWAP(a, b);
	int dx = b.x - a.x, dy = b.y - a.y;
	int adx = ABS(dx), ady = ABS(dy);
	int n = MAX(adx, ady);
	for (int i = 0; i <= n; i++) {
		Common::Point p;
		if (adx >= ady) {
			p.x = a.x + (dx > 0 ? i : -i);
			p.y = a.y + (adx ? roundWindowsLine(dy * i, adx, dy > 0) : 0);
		} else {
			p.y = a.y + i;
			p.x = a.x + roundWindowsLine(dx * i, ady, dx > 0);
		}
		if (p != end)
			primitives.drawPoint(p.x, p.y, color, data);
	}
}

} // End of anonymous namespace

/**
 * Draws an oval as Windows Director does (sc_eng.exe FUN_00439f20). A filled
 * oval is a polygon of the 4-degree points and gets no frame. A frame one
 * pixel thick is a polyline through the points of a box one pixel smaller, a
 * thicker one the ring between the points and a second set inset by the line
 * size. Neither matches QuickDraw's ovals: a 60x60 oval covers 59x59 pixels.
 */
void drawWindowsOval(Graphics::Primitives &primitives, const Common::Rect &rect, int lineSize, bool filled, uint32 color, void *data) {
	if (rect.width() < 2 || rect.height() < 2)
		return;
	Common::Array<Common::Point> polys[2];
	if (filled) {
		windowsOvalPoints(rect, 1, 0, polys[0]);
		fillWindowsPolygons(primitives, polys, 1, color, data);
		return;
	}
	if (lineSize <= 0)
		return;
	if (lineSize == 1) {
		windowsOvalPoints(rect, 2, 0, polys[0]);
		for (uint i = 0; i + 1 < polys[0].size(); i++)
			drawWindowsLine(primitives, polys[0][i], polys[0][i + 1], color, data);
		return;
	}
	windowsOvalPoints(rect, 1, 0, polys[0]);
	if (rect.width() - 1 < lineSize * 2 || rect.height() - 1 < lineSize * 2) {
		// GDI Ellipse with a thick pen; close enough to fill it
		fillWindowsPolygons(primitives, polys, 1, color, data);
		return;
	}
	windowsOvalPoints(rect, 1, lineSize, polys[1]);
	fillWindowsPolygons(primitives, polys, 2, color, data);
}

/**
 * The colour nearest to an RGB value, as Windows Director picks it for
 * blend ink in 8-bit: a 32x32x32 table, where every palette entry claims
 * its own cell and then grows a box one cell a round, taking the cells
 * nobody owns yet. Black seeds first, then white, then the rest in palette
 * order, so a tie goes to the earlier (brighter) entry. sc_eng.exe (D5)
 * FUN_0042a540; the colours match the Win D4 projector (T_SPRT17).
 */
uint32 DirectorEngine::findBlendColor(byte r, byte g, byte b) {
	if (_pixelformat.bytesPerPixel != 1 || getPlatform() != Common::kPlatformWindows || _currentPaletteLength < 2)
		return _wm->findBestColor(r, g, b);

	if (_inverseTable.empty()) {
		int n = _currentPaletteLength;
		_inverseTable.resize(32768);
		Common::Array<bool> claimed(32768, false);
		Common::Array<InverseBox> boxes;

		auto seed = [&](int i) {
			byte cr = inverseCell(_currentPalette[i * 3]);
			byte cg = inverseCell(_currentPalette[i * 3 + 1]);
			byte cb = inverseCell(_currentPalette[i * 3 + 2]);
			if (!claimInverseCells(_inverseTable, claimed, i, cr, cr, cg, cg, cb, cb))
				return false;
			InverseBox box;
			box.index = i;
			box.active = true;
			for (int f = 0; f < 6; f++)
				box.faceActive[f] = true;
			box.face[0] = box.face[1] = cr;
			box.face[2] = box.face[3] = cg;
			box.face[4] = box.face[5] = cb;
			boxes.push_back(box);
			return true;
		};

		bool lastSeeded = seed(n - 1);
		InverseBox last;
		if (lastSeeded) {
			last = boxes.back();
			boxes.pop_back();
		}
		seed(0);
		for (int i = 1; i < n - 1; i++)
			seed(i);
		if (lastSeeded)
			boxes.push_back(last);

		bool grew = true;
		while (grew) {
			grew = false;
			for (auto &box : boxes) {
				if (!box.active)
					continue;
				for (int f = 0; f < 6; f++) {
					if (!box.faceActive[f])
						continue;
					if (f % 2 == 0) {
						if (box.face[f] == 0)
							box.faceActive[f] = false;
						else
							box.face[f]--;
					} else {
						if (box.face[f] == 31)
							box.faceActive[f] = false;
						else
							box.face[f]++;
					}
				}
				const byte *c = box.face;
				bool any = false;
				for (int f = 0; f < 6; f++) {
					if (!box.faceActive[f])
						continue;
					int lo[3] = { c[0], c[2], c[4] };
					int hi[3] = { c[1], c[3], c[5] };
					lo[f / 2] = hi[f / 2] = c[f];
					if (claimInverseCells(_inverseTable, claimed, box.index, lo[0], hi[0], lo[1], hi[1], lo[2], hi[2]))
						any = true;
					else
						box.faceActive[f] = false;
				}
				if (any)
					grew = true;
				else
					box.active = false;
			}
		}
		_inverseTable[0] = n - 1;
		_inverseTable[0x7fff] = 0;
	}

	return _inverseTable[(inverseCell(r) << 10) | (inverseCell(g) << 5) | inverseCell(b)];
}

void DirectorEngine::setPalette(const byte *palette, uint16 count) {

	memset(_currentPalette, 0, 768);
	memmove(_currentPalette, palette, count * 3);
	_currentPaletteLength = count;

	if (debugChannelSet(8, kDebugImages)) {
		Common::String palData;
		for (size_t i = 0; i < (size_t)_currentPaletteLength; i++) {
			palData += Common::String::format("%02X%02X%02X", _currentPalette[3 * i], _currentPalette[3 * i + 1], _currentPalette[3 * i + 2]);
		}
		debugC(8, kDebugImages, "DirectorEngine::setPalette(): Setting current palette: %s", palData.c_str());
	}

	syncPalette();
}

void DirectorEngine::shiftPalette(int startIndex, int endIndex, bool reverse) {
	if (startIndex == endIndex)
		return;

	if (startIndex > endIndex)
		return;

	byte temp[3] = { 0, 0, 0 };
	int span = endIndex - startIndex + 1;
	if (reverse) {
		memcpy(temp, _currentPalette + 3 * startIndex, 3);
		memmove(_currentPalette + 3 * startIndex,
			_currentPalette + 3 * startIndex + 3,
			(span - 1) * 3);
		memcpy(_currentPalette + 3 * endIndex, temp, 3);
	} else {
		memcpy(temp, _currentPalette + 3 * endIndex, 3);
		memmove(_currentPalette + 3 * startIndex + 3,
			_currentPalette + 3 * startIndex,
			(span - 1) * 3);
		memcpy(_currentPalette + 3 * startIndex, temp, 3);
	}

	if (debugChannelSet(8, kDebugImages)) {
		Common::String palData;
		for (size_t i = 0; i < (size_t)_currentPaletteLength; i++) {
			palData += Common::String::format("%02X%02X%02X", _currentPalette[3 * i], _currentPalette[3 * i + 1], _currentPalette[3 * i + 2]);
		}
		debugC(8, kDebugImages, "DirectorEngine::shiftPalette(): Rotating current palette (start: %d, end: %d, reverse: %d): %s", startIndex, endIndex, reverse, palData.c_str());
	}

	syncPalette();
}

void DirectorEngine::syncPalette() {
	_inverseTable.clear();

	byte paletteBuf[768];
	// In true color mode, gamma correction is applied to the screen output
	// instead, see DirectorEngine::run()
	if (_gammaCorrection && _pixelformat.bytesPerPixel == 1) {
		for (size_t i = 0; i < (size_t)_currentPaletteLength*3; i++) {
			paletteBuf[i] = Graphics::macGammaCorrectionLookUp[_currentPalette[i]];
		}
	} else {
		memcpy(paletteBuf, _currentPalette, _currentPaletteLength*3);
	}

	if (_pixelformat.bytesPerPixel == 1) {
		// Pass the palette to OSystem only for 8bpp mode
		_system->getPaletteManager()->setPalette(paletteBuf, 0, _currentPaletteLength);
	}

	_wm->passPalette(paletteBuf, _currentPaletteLength);
}

void DirectorEngine::clearPalettes() {
	// FIXME: Ideally we would run this every time we switch movie; but that would need to take
	// into account e.g. shared casts that haven't changed, multiple windows...
	for (auto it = _loadedPalettes.begin(); it != _loadedPalettes.end(); ++it) {
		if (it->_value.id.castLib > 0) {
			debugC(5, kDebugImages, "DirectorEngine::clearPalettes(): erasing palette %d, %d", it->_value.id.member, it->_value.id.castLib);
			delete[] it->_value.palette;
			_loadedPalettes.erase(it);
		}
	}
	_lastPalette = CastMemberID();
}

void DirectorEngine::setCursor(DirectorCursor type) {
	switch (type) {
	case kCursorMouseDown:
		_wm->replaceCustomCursor(mouseDown, 16, 16, 0, 0, 3);
		break;
	case kCursorMouseUp:
		_wm->replaceCustomCursor(mouseUp, 16, 16, 0, 0, 3);
		break;
	}
}

void DirectorEngine::draw() {
	_wm->renderZoomBox(true);
	_wm->draw();
	g_system->updateScreen();
}

template <typename T>
class InkPrimitives final : public Graphics::Primitives {
public:
	constexpr InkPrimitives() {}
	void drawPoint(int x, int y, uint32 src, void *data) override;
private:
	inline void decomposeColor(Graphics::MacWindowManager *wm, uint32 color, byte &r, byte &g, byte &b) {
		if (sizeof(T) == sizeof(byte)) {
			wm->getPaletteEntry(color, r, g, b);
		} else {
			wm->_pixelformat.colorToRGB(color, r, g, b);
		}
	}
};

template <typename T>
void InkPrimitives<T>::drawPoint(int x, int y, uint32 src, void *data) {
	DirectorPlotData *p = (DirectorPlotData *)data;
	Graphics::MacWindowManager *wm = p->d->_wm;

	if (!p->destRect.contains(x, y))
		return;


	T *dst;
	uint32 tmpDst;

	dst = (T *)p->dst->getBasePtr(x, y);

	if (p->ms) {
		if (p->ms->pd->thickness.x > 1 || p->ms->pd->thickness.y > 1) {
			Common::Point prevThickness = p->ms->pd->thickness;
			int x1 = x;
			int x2 = x1 + prevThickness.x;
			int y1 = y;
			int y2 = y1 + prevThickness.y;

			p->ms->pd->thickness = Common::Point(1, 1);	// We do not want recursive loops

			for (y = y1; y < y2; y++)
				for (x = x1; x < x2; x++)
					if (x >= 0 && x < p->ms->pd->surface->w && y >= 0 && y < p->ms->pd->surface->h) {
						drawPoint(x, y, src, data);
					}

			p->ms->pd->thickness = prevThickness;
			return;
		}

		if (p->ms->tile) {
			int x1 = (p->ms->tileRect->left + p->ms->pd->fillOriginX + x) % p->ms->tileRect->width();
			if (x1 < 0)
				x1 += p->ms->tileRect->width();
			int y1 = (p->ms->tileRect->top  + p->ms->pd->fillOriginY + y) % p->ms->tileRect->height();
			if (y1 < 0)
				y1 += p->ms->tileRect->height();

			src = p->ms->tile->_surface.getPixel(x1, y1);
		} else {
			// Get the pixel that macDrawPixel will give us, but store it to apply the
			// ink later
			tmpDst = *dst;
			wm->getDrawPrimitives().drawPoint(x, y, src, p->ms->pd);
			src = *dst;

			*dst = tmpDst;
		}

		if (p->ms->blendBuf) {
			// Collect the pixel; inkBlitShape blends each one once
			if (p->ms->blendRect.contains(x, y))
				(*p->ms->blendBuf)[(y - p->ms->blendRect.top) * p->ms->blendRect.width() + (x - p->ms->blendRect.left)] = src | (1ULL << 32);
			return;
		}
	}

	// Shapes blend too (Alice: the yellow circles in STONE.DIR)
	if (p->alpha) {
		// Sprite blend does not respect colourization; defaults to matte ink
		byte rSrc, gSrc, bSrc;
		byte rDst, gDst, bDst;

		decomposeColor(wm, src, rSrc, gSrc, bSrc);
		decomposeColor(wm, *dst, rDst, gDst, bDst);

		rDst = lerpByte(rSrc, rDst, p->alpha, 255);
		gDst = lerpByte(gSrc, gDst, p->alpha, 255);
		bDst = lerpByte(bSrc, bDst, p->alpha, 255);
		*dst = g_director->findBlendColor(rDst, gDst, bDst);
		return;
	}

 	switch (p->ink) {
	case kInkTypeBackgndTrans:
		if (p->srfMask) {
			// If there's a mask, we already dealing with transparency, so just copy the pixel.
			 *dst = src;
		} else {
			if (p->oneBitImage) {
				// One-bit images have a slightly different rendering algorithm for BackgndTrans.
				// Foreground colour is used, and background colour is ignored.
				*dst = (src == p->colorBlack) ? p->foreColor : *dst;
			} else {
				*dst = (src == p->backColor) ? *dst : src;
			}
		}
		break;
	case kInkTypeMatte:
		// fall through
	case kInkTypeMask:
		// Only unmasked pixels make it here, so copy them straight
	case kInkTypeBlend:
		// If there's a blend factor set, it's dealt with in the alpha handling block.
		// Otherwise, treat it like a Matte image.
	case kInkTypeCopy: {
		if (p->applyColor) {
			*dst = (src == p->colorBlack) ? p->foreColor : ((src == p->colorWhite) ? p->backColor : *dst);
		} else {
			*dst = src;
		}
		break;
	}
	case kInkTypeNotCopy:
		if (p->applyColor) {
			if (sizeof(T) == 1) {
				*dst = (src == p->colorBlack) ? p->backColor : ((src == p->colorWhite) ? p->foreColor : src);
			} else if (sizeof(T) == 4) {
				// In 32-bit, apply color mode seems to just return the original src
				// with no changes. This is different to kInkTypeCopy.
				*dst = src;
			}
		} else {
			// Find the inverse of the colour and match it back to the palette if required
			byte rSrc, gSrc, bSrc;
			decomposeColor(wm, src, rSrc, gSrc, bSrc);

			*dst = wm->findBestColor(~rSrc, ~gSrc, ~bSrc);
		}
		break;
	case kInkTypeTransparent:
		if (p->oneBitImage || p->applyColor) {
			*dst = (src == p->colorBlack) ? p->foreColor : *dst;
		} else {
			if (sizeof(T) == 1) {
				// OR dst palette index with src.
				// Originally designed for 1-bit mode to make white pixels
				// transparent.
				*dst = *dst | src;
			} else {
				// In 32-bit mode, this is an AND.
				*dst = *dst & src;
			}
		}
		break;
	case kInkTypeNotTrans:
		if (p->oneBitImage || p->applyColor) {
			*dst = (src == p->colorWhite) ? p->foreColor : *dst;
		} else {
			if (sizeof(T) == 1) {
				// OR dst palette index with the inverse of src.
				*dst = *dst | ~src;
			} else {
				// In 32-bit mode, this is an AND.
				*dst = *dst & ~(src & 0xffffff00);
			}
		}
		break;
	case kInkTypeReverse:
		if (sizeof(T) == 1) {
			// XOR dst palette index with src, for 8-bit bitmaps
			// too: the Mac player passes the ink to CopyBits as
			// srcXor, and Windows D5 does the same (T_REVI01).
			// Spaceship Warlock's prologue text relies on this:
			// index ^ 255 gives a grey ramp on the black stage.
			*dst ^= src;
		} else {
			// In 32-bit mode, this is the opposite??
			*dst ^= ~(src);
		}
		break;
	case kInkTypeNotReverse:
		if (sizeof(T) == 1) {
			// XOR dst palette index with the inverse of src.
			*dst ^= ~(src);
		} else {
			// In 32-bit mode, this is the opposite??
			*dst ^= src & 0xffffff00;
		}
		break;
	case kInkTypeGhost:
		if (p->oneBitImage || p->applyColor) {
			*dst = (src == p->colorBlack) ? p->backColor : *dst;
		} else {
			if (sizeof(T) == 1) {
				// AND dst palette index with the inverse of src.
				// Originally designed for 1-bit mode so that
				// black pixels would be invisible until they were
				// over a black background, showing as white.
				*dst = *dst & ~src;
			} else {
				// In 32-bit mode, OR dst RGBA with inverse src
				*dst = *dst | ~src;
			}
		}
		break;
	case kInkTypeNotGhost:
		if (p->oneBitImage || p->applyColor) {
			*dst = (src == p->colorWhite) ? p->backColor : *dst;
		} else {
			if (sizeof(T) == 1) {
				// AND dst palette index with src.
				*dst = *dst & src;
			} else {
				// In 32-bit mode, OR dst RGBA with src
				*dst = *dst | src;
			}
		}
		break;
	default: {
		// Arithmetic ink types, based on real color values
		byte rSrc, gSrc, bSrc;
		byte rDst, gDst, bDst;

		decomposeColor(wm, src, rSrc, gSrc, bSrc);
		decomposeColor(wm, *dst, rDst, gDst, bDst);

		switch (p->ink) {
		case kInkTypeAddPin:
			// Add src to dst, but pinning each channel so it can't go above 0xff.
			*dst = wm->findBestColor(rDst + MIN(0xff - rDst, (int)rSrc), gDst + MIN(0xff - gDst, (int)gSrc), bDst + MIN(0xff - bDst, (int)bSrc));
			break;
		case kInkTypeAdd:
			// Add src to dst, allowing each channel to overflow and wrap around.
			*dst = wm->findBestColor(rDst + rSrc, gDst + gSrc, bDst + bSrc);
			break;
		case kInkTypeSubPin:
			// Subtract src from dst, but pinning each channel so it can't go below 0x00.
			*dst = wm->findBestColor(MAX(rDst - rSrc, 1) - 1, MAX(gDst - gSrc, 1) - 1, MAX(bDst - bSrc, 1) - 1);
			break;
		case kInkTypeLight:
			// Pick the higher of src and dst for each channel, lightening the image.
			*dst = wm->findBestColor(MAX(rSrc, rDst), MAX(gSrc, gDst), MAX(bSrc, bDst));
			break;
		case kInkTypeSub:
			// Subtract src from dst, allowing each channel to underflow and wrap around.
			*dst = wm->findBestColor(rDst - rSrc, gDst - gSrc, bDst - bSrc);
			break;
		case kInkTypeDark:
			// Pick the lower of src and dst for each channel, darkening the image.
			*dst = wm->findBestColor(MIN(rSrc, rDst), MIN(gSrc, gDst), MIN(bSrc, bDst));
			break;
		default:
			break;
		}
	}
	}
}

Graphics::Primitives *DirectorEngine::getInkPrimitives() {
	if (!_primitives) {
		if (_pixelformat.bytesPerPixel == 1)
			_primitives = new InkPrimitives<byte>();
		else if (_pixelformat.bytesPerPixel == 2)
			_primitives = new InkPrimitives<uint16>();
		else
			_primitives = new InkPrimitives<uint32>();
	}

	return _primitives;
}

uint32 DirectorEngine::getColorBlack() {
	if (_pixelformat.bytesPerPixel == 1)
		// needs to be the last entry in the palette.
		// we can't use findBestColor, as it might
		// pick a different entry that's also black.
		return 0xff;
	else
		// send RGB through findBestColor to avoid endian issues
		return _wm->findBestColor(0, 0, 0);
}

uint32 DirectorEngine::getColorWhite() {
	if (_pixelformat.bytesPerPixel == 1)
		// needs to be the first entry in the palette.
		// we can't use findBestColor, as it might
		// pick a different entry that's also white.
		return 0x00;
	else
		// send RGB through findBestColor to avoid endian issues
		return _wm->findBestColor(255, 255, 255);
}

void DirectorPlotData::setApplyColor() {
	// Director has two ways of rendering an ink setting.
	// The default is to incorporate the full range of colors in the image.
	// "applyColor" is used to denote the other option; reduce the image
	// to some combination of the currently set foreground and background color.
	applyColor = false;

	switch (ink) {
	case kInkTypeMatte:
	case kInkTypeMask:
	case kInkTypeCopy:
	case kInkTypeNotCopy:
		applyColor = (foreColor != colorBlack) || (backColor != colorWhite);
		break;
	case kInkTypeTransparent:
	case kInkTypeNotTrans:
	case kInkTypeBackgndTrans:
	case kInkTypeGhost:
	case kInkTypeNotGhost:
		applyColor = !((foreColor == colorBlack) && (backColor == colorWhite));
		break;
	default:
		break;
	}
}

uint32 DirectorPlotData::preprocessColor(uint32 src) {
	// HACK: Right now this method is just used for adjusting the colourization on text
	// sprites, as it would be costly to colourize the chunks on the fly each
	// time a section needs drawing. It's ugly but mostly works.
	if (sprite == kTextSprite || sprite == kButtonSprite || sprite == kCheckboxSprite || sprite == kRadioButtonSprite) {
		switch(ink) {
		case kInkTypeMask:
			src = (src == backColor ? foreColor : 0xff);
			break;
		// 8-bit reverse inks XOR the raw palette index, like CopyBits
		// srcXor. Mapping the pixels here would turn every one of them
		// into 0 (white), which XORs to nothing: The Apartment's
		// "Testing User Entry" buttons and text vanished.
		case kInkTypeReverse:
			if (d->_wm->_pixelformat.bytesPerPixel != 1)
				src = (src == foreColor ? 0 : colorWhite);
			break;
		case kInkTypeNotReverse:
			if (d->_wm->_pixelformat.bytesPerPixel != 1)
				src = (src == backColor ? colorWhite : 0);
			break;
			// looks like this part is wrong, maybe it's very same as reverse?
			// check warlock/DATA/WARLOCKSHIP/ENG/ABOUT to see more detail.
//		case kInkTypeGhost:
//			src = (src == foreColor ? backColor : colorWhite);
//			break;
		case kInkTypeNotGhost:
			src = (src == backColor ? colorWhite : backColor);
			break;
		case kInkTypeNotCopy:
			src = (src == foreColor ? backColor : foreColor);
			break;
		case kInkTypeNotTrans:
			src = (src == foreColor ? backColor : colorWhite);
			break;
		default:
			break;
		}
	}

	return src;
}

void DirectorPlotData::inkBlitShape(Common::Rect &srcRect) {
	if (!ms)
		return;

	// Preprocess shape colours
	switch (ink) {
	case kInkTypeNotTrans:
	case kInkTypeNotReverse:
	case kInkTypeNotGhost:
		return;
	case kInkTypeReverse:
		ms->foreColor = 255;
		ms->backColor = 0;
		break;
	default:
		break;
	}

	Common::Point wpos;
	Movie *movie = g_director->getCurrentMovie();

	if (g_director->_wm->_mode & Graphics::kWMModeNoDesktop)
		wpos = Common::Point(movie->getCast()->_movieRect.left, movie->getCast()->_movieRect.top);
	else
		wpos = movie->getWindow()->getAbsolutePos();

	Common::Rect fillAreaRect((int)srcRect.width(), (int)srcRect.height());
	fillAreaRect.moveTo(srcRect.left, srcRect.top);
	Graphics::MacPlotData plotFill(dst, nullptr, &d->getPatterns(), ms->pattern, srcRect.left + wpos.x, srcRect.top + wpos.y, {1, 1}, ms->backColor);

	uint strokePattern = 1;

	bool outline = (ms->spriteType == kOutlinedRectangleSprite || ms->spriteType == kOutlinedRoundedRectangleSprite
			|| ms->spriteType == kOutlinedOvalSprite);

	if (outline)
		strokePattern = ms->pattern;

	Common::Rect strokeRect(MAX((int)srcRect.width() - ms->lineSize, 0), MAX((int)srcRect.height() - ms->lineSize, 0));
	strokeRect.moveTo(srcRect.left, srcRect.top);
	Graphics::MacPlotData plotStroke(dst, nullptr, &d->getPatterns(), strokePattern, strokeRect.left + wpos.x, strokeRect.top + wpos.y, {(int16)ms->lineSize, (int16)ms->lineSize}, ms->backColor);

	// Windows ovals draw their frame in single pixels inside the oval
	Graphics::MacPlotData plotFrame(dst, nullptr, &d->getPatterns(), strokePattern, fillAreaRect.left + wpos.x, fillAreaRect.top + wpos.y, {1, 1}, ms->backColor);

	Graphics::Primitives *primitives = g_director->getInkPrimitives();

	// A blended shape is drawn into a buffer first: the fill spans of an oval
	// overlap, and the stroke goes over the fill, which would blend those
	// pixels twice
	Common::Array<uint64> blendBuf;
	if (alpha) {
		ms->blendRect = srcRect.findIntersectingRect(destRect);
		if (ms->blendRect.isEmpty())
			return;
		blendBuf.resize(ms->blendRect.width() * ms->blendRect.height());
		for (uint i = 0; i < blendBuf.size(); i++)
			blendBuf[i] = 0;
		ms->blendBuf = &blendBuf;
	}

	switch (ms->spriteType) {
	case kRectangleSprite:
		ms->pd = &plotFill;
		primitives->drawFilledRect1(fillAreaRect, ms->foreColor, this);
		// fall through
	case kOutlinedRectangleSprite:
		// if we have lineSize <= 0, means we are not drawing anything. so we may return directly.
		if (ms->lineSize <= 0)
			break;
		ms->pd = &plotStroke;

		if (!outline)
			ms->tile = nullptr;

		primitives->drawRect1(strokeRect, ms->foreColor, this);
		break;
	case kRoundedRectangleSprite:
		ms->pd = &plotFill;
		primitives->drawRoundRect1(fillAreaRect, 12, ms->foreColor, true, this);
		// fall through
	case kOutlinedRoundedRectangleSprite:
		if (ms->lineSize <= 0)
			break;
		ms->pd = &plotStroke;

		if (!outline)
			ms->tile = nullptr;

		primitives->drawRoundRect1(strokeRect, 12, ms->foreColor, false, this);
		break;
	case kOvalSprite:
		ms->pd = &plotFill;
		if (d->getPlatform() == Common::kPlatformWindows)
			drawWindowsOval(*primitives, fillAreaRect, ms->lineSize, true, ms->foreColor, this);
		else
			primitives->drawEllipse(fillAreaRect.left, fillAreaRect.top, fillAreaRect.right, fillAreaRect.bottom, ms->foreColor, true, this);
		// Windows draws no frame around a filled oval (T_SPRT18)
		if (d->getPlatform() == Common::kPlatformWindows)
			break;
		// fall through
	case kOutlinedOvalSprite:
		if (ms->lineSize <= 0)
			break;
		if (d->getPlatform() == Common::kPlatformWindows) {
			// The frame is drawn in single pixels inside the oval
			ms->pd = &plotFrame;
			if (!outline)
				ms->tile = nullptr;
			drawWindowsOval(*primitives, fillAreaRect, ms->lineSize, false, ms->foreColor, this);
			break;
		}
		ms->pd = &plotStroke;

		if (!outline)
			ms->tile = nullptr;

		primitives->drawEllipse(strokeRect.left, strokeRect.top, strokeRect.right, strokeRect.bottom, ms->foreColor, false, this);
		break;
	case kLineTopBottomSprite:
		ms->pd = &plotStroke;
		primitives->drawLine(strokeRect.left, strokeRect.top, strokeRect.right, strokeRect.bottom, ms->foreColor, this);
		break;
	case kLineBottomTopSprite:
		ms->pd = &plotStroke;
		primitives->drawLine(strokeRect.left, strokeRect.bottom, strokeRect.right, strokeRect.top, ms->foreColor, this);
		break;
	default:
		warning("DirectorPlotData::inkBlitShape: Expected shape type but got type %d", ms->spriteType);
	}

	if (alpha) {
		ms->blendBuf = nullptr;
		MacShape *shape = ms;
		ms = nullptr;	// drawPoint blends the collected colour as it is
		for (int y = shape->blendRect.top; y < shape->blendRect.bottom; y++) {
			for (int x = shape->blendRect.left; x < shape->blendRect.right; x++) {
				uint64 src = blendBuf[(y - shape->blendRect.top) * shape->blendRect.width() + (x - shape->blendRect.left)];
				if (src)
					primitives->drawPoint(x, y, (uint32)src, this);
			}
		}
		ms = shape;
	}
}

void DirectorPlotData::inkBlitSurface(Common::Rect &srcRect, const Graphics::Surface *mask) {
	if (!srf)
		return;

	if (mask && srfMask)
		error("DirectorPlotData::inkBlitSurface: Masking not supported on surfaces with separate mask");

	// TODO: Determine why colourization causes problems in Warlock
	if (sprite == kTextSprite || sprite == kButtonSprite || sprite == kCheckboxSprite || sprite == kRadioButtonSprite)
		applyColor = false;

	Common::Rect srfClip = srf->getBounds();
	bool failedBoundsCheck = false;

	// FAST PATH: if we're not doing any per-pixel ops,
	// use the stock blitter. Your CPU will thank you.
	if (!applyColor && !alpha && !ms) {
		Common::Rect offsetRect(
			Common::Point(abs(srcRect.left - destRect.left), abs(srcRect.top - destRect.top)),
			destRect.width(),
			destRect.height()
		);
		offsetRect.clip(srfClip);
		switch (ink) {
		case kInkTypeCopy:
			if (!mask) {
				// Blit to a point, not to destRect: blitFrom scales when the
				// two rects differ, and the surface can be smaller than the
				// sprite, as for a fixed field in a larger score box. The
				// slow path below doesn't scale either.
				if (!offsetRect.isEmpty())
					dst->blitFrom(*srf, offsetRect, Common::Point(destRect.left, destRect.top));
				return;
			}
			break;
		default:
			break;
		}
	}

	if (srcAlpha && !mask && !srfMask && dst->format == srf->format) {
		const Graphics::PixelFormat &fmt = srf->format;
		srcPoint.y = abs(srcRect.top - destRect.top);
		for (int i = 0; i < destRect.height(); i++, srcPoint.y++) {
			srcPoint.x = abs(srcRect.left - destRect.left);
			for (int j = 0; j < destRect.width(); j++, srcPoint.x++) {
				if (!srfClip.contains(srcPoint)) {
					failedBoundsCheck = true;
					continue;
				}
				uint32 *dp = (uint32 *)dst->getBasePtr(destRect.left + j, destRect.top + i);
				uint8 a, rS, gS, bS, rD, gD, bD;
				fmt.colorToARGB(*(const uint32 *)srf->getBasePtr(srcPoint.x, srcPoint.y), a, rS, gS, bS);
				if (a == 0)
					continue;
				if (a == 255) {
					*dp = fmt.RGBToColor(rS, gS, bS);
					continue;
				}
				fmt.colorToRGB(*dp, rD, gD, bD);
				*dp = fmt.RGBToColor(lerpByte(rD, rS, a, 255), lerpByte(gD, gS, a, 255), lerpByte(bD, bS, a, 255));
			}
		}
		if (failedBoundsCheck)
			warning("DirectorPlotData::inkBlitSurface: Out of bounds in alpha blit");
		return;
	}

	// For blit efficiency, surfaces passed here need to be the same
	// format as the window manager. Most of the time this is
	// the job of BitmapCastMember::createWidget.

	Graphics::Primitives *primitives = g_director->getInkPrimitives();

	srcPoint.y = abs(srcRect.top - destRect.top);
	for (int i = 0; i < destRect.height(); i++, srcPoint.y++) {
		srcPoint.x = abs(srcRect.left - destRect.left);
		const byte *msk = mask ? (const byte *)mask->getBasePtr(srcPoint.x, srcPoint.y) : nullptr;

		if (srfMask) {
			if (srcPoint.y >= srfMask->h)
				continue;

			msk = (const byte *)srfMask->getBasePtr(srcPoint.x, srcPoint.y);
		}

		for (int j = 0; j < destRect.width(); j++, srcPoint.x++) {
			if (!srfClip.contains(srcPoint)) {
				failedBoundsCheck = true;
				continue;
			}

			// Do not try render beyond the mask bounds
			if (srfMask && (srcPoint.x >= srfMask->w))
				continue;

			if (!(mask || srfMask) || (msk && (*msk++))) {
				if (d->_wm->_pixelformat.bytesPerPixel == 1) {
					primitives->drawPoint(destRect.left + j, destRect.top + i,
										preprocessColor(*((byte *)srf->getBasePtr(srcPoint.x, srcPoint.y))), this);
				} else if (d->_wm->_pixelformat.bytesPerPixel == 2) {
					primitives->drawPoint(destRect.left + j, destRect.top + i,
										preprocessColor(*((uint16 *)srf->getBasePtr(srcPoint.x, srcPoint.y))), this);
				} else {
					primitives->drawPoint(destRect.left + j, destRect.top + i,
										preprocessColor(*((uint32 *)srf->getBasePtr(srcPoint.x, srcPoint.y))), this);
				}
			}
		}
	}

	if (failedBoundsCheck) {
		warning("DirectorPlotData::inkBlitSurface: Out of bounds - srfClip: %d,%d,%d,%d, srcRect: %d,%d,%d,%d, dstRect: %d,%d,%d,%d",
				srfClip.left, srfClip.top, srfClip.right, srfClip.bottom,
				srcRect.left, srcRect.top, srcRect.right, srcRect.bottom,
				destRect.left, destRect.top, destRect.right, destRect.bottom);
	}

}

} // End of namespace Director
