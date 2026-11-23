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

#include "common/hashmap.h"
#include "common/macresman.h"
#include "common/memstream.h"

#include "graphics/macgui/macwidget.h"
#include "graphics/macgui/macwindowmanager.h"
#include "graphics/surface.h"

#include "image/jpeg.h"

#include "director/director.h"
#include "director/cast.h"
#include "director/castmember/xtra.h"
#include "director/lingo/lingo-the.h"
#include "director/lingo/xtras-cast/mmbigimg.h"
#include "director/util.h"
#include "director/window.h"

namespace Director {

namespace MMBigImg {

CastMember *createCastMember(Cast *cast, uint16 castId, XtraCastMember *xtra) {
	return new MMBigImgCastMember(cast, castId);
}

} // End of namespace MMBigImg

MMBigImgCastMember::MMBigImgCastMember(Cast *cast, uint16 castId)
		: CastMember(cast, castId) {
	_type = kCastXtra;
}

MMBigImgCastMember::MMBigImgCastMember(Cast *cast, uint16 castId, MMBigImgCastMember &source)
		: CastMember(cast, castId) {
	_type = kCastXtra;
	_initialRect = source._initialRect;
	_boundingRect = source._boundingRect;
	if (cast == source._cast)
		_children = source._children;
	_viewW = source._viewW;
	_viewH = source._viewH;
	if (!source._fileName.empty() && loadFile(source._fileName)) {
		_orgX = source._orgX;
		_orgY = source._orgY;
	}
}

MMBigImgCastMember::~MMBigImgCastMember() {
	freeImage();
}

void MMBigImgCastMember::freeImage() {
	for (auto &tile : _tiles) {
		if (tile.image) {
			tile.image->free();
			delete tile.image;
		}
	}
	_tiles.clear();
	_data.clear();
	_loaded = false;
	_width = _height = 0;
	_cols = _rows = 0;
}

// Setting MMFileName drops the old image first, even if the new file
// cannot be read (Egypt clears it with EMPTY before each new one)
bool MMBigImgCastMember::loadFile(const Common::String &fileName) {
	freeImage();
	_fileName = fileName;
	setModified(true);

	if (fileName.empty())
		return false;

	Common::Path location = findPath(fileName);
	Common::SeekableReadStream *stream = location.empty() ? nullptr : Common::MacResManager::openFileOrDataFork(location);
	if (!stream) {
		warning("MMBigImgCastMember: cannot open '%s'", fileName.c_str());
		return false;
	}
	_data.resize(stream->size());
	stream->read(_data.data(), _data.size());
	delete stream;

	Common::MemoryReadStream header(_data.data(), _data.size());
	if (header.readUint32BE() != MKTAG('I', 'B', 'M', 'M')) {
		warning("MMBigImgCastMember: '%s' is not an MBI file", fileName.c_str());
		_data.clear();
		return false;
	}
	uint32 count = header.readUint32LE();
	_width = header.readUint32LE();
	_height = header.readUint32LE();
	header.readUint32LE();	// tile width
	header.readUint32LE();	// tile height
	_cols = header.readUint32LE();
	_rows = header.readUint32LE();
	if (count != _cols * _rows || header.size() < 32 + count * 16) {
		warning("MMBigImgCastMember: bad tile table in '%s'", fileName.c_str());
		freeImage();
		return false;
	}
	_tiles.resize(count);
	for (auto &tile : _tiles) {
		tile.size = header.readUint32LE();
		tile.offset = header.readUint32LE();
		tile.w = header.readUint32LE();
		tile.h = header.readUint32LE();
	}
	_loaded = true;
	_initialRect = Common::Rect(_width, _height);
	debugC(3, kDebugLoading, "MMBigImgCastMember %d: loaded '%s', %dx%d in %dx%d tiles",
		_castId, fileName.c_str(), _width, _height, _cols, _rows);

	setOrigin(0, 0);
	return true;
}

Graphics::Surface *MMBigImgCastMember::getTile(uint index) {
	Tile &tile = _tiles[index];
	if (tile.image)
		return tile.image;
	if (tile.offset + tile.size > _data.size())
		return nullptr;

	Common::MemoryReadStream stream(_data.data() + tile.offset, tile.size);
	Image::JPEGDecoder decoder;
	decoder.setOutputPixelFormat(Graphics::PixelFormat::createFormatRGBA32());
	if (!decoder.loadStream(stream) || !decoder.getSurface()) {
		warning("MMBigImgCastMember: cannot decode tile %d of '%s'", index, _fileName.c_str());
		return nullptr;
	}
	tile.image = decoder.getSurface()->convertTo(Graphics::PixelFormat::createFormatRGBA32());
	return tile.image;
}

// MMBigImg.x32 FUN_100017b0
void MMBigImgCastMember::setOrigin(int x, int y) {
	if (!_loaded) {
		_orgX = _orgY = 0;
		_viewW = 800;
		_viewH = 600;
		return;
	}
	_orgX = MIN(x, _width - 1);
	_orgY = MIN(y, _height - 1);
	if (_viewW < 1)
		_viewW = 800;
	if (_viewH < 1)
		_viewH = 600;
	clampOrigin();
	setModified(true);
}

// FUN_10001830: the view takes the sprite size at draw time
void MMBigImgCastMember::setView(int w, int h) {
	_viewW = w < 1 ? 800 : w;
	_viewH = h < 1 ? 600 : h;
	if (_loaded) {
		_viewW = MIN(_viewW, _width);
		_viewH = MIN(_viewH, _height);
		clampOrigin();
	}
}

// FUN_10001890 (which also decodes the tiles in view)
void MMBigImgCastMember::clampOrigin() {
	if (_orgX < 0)
		_orgX = 0;
	if (_orgY < 0)
		_orgY = 0;
	if (_orgX + _viewW > _width)
		_orgX = _width - _viewW;
	if (_orgY + _viewH > _height)
		_orgY = _height - _viewH;
}

// Like the other asset Xtras, the sprite's loc is the centre of its box
Common::Point MMBigImgCastMember::getRegistrationOffset() {
	return Common::Point(_initialRect.width() / 2, _initialRect.height() / 2);
}

Common::Point MMBigImgCastMember::getRegistrationOffset(int16 currentWidth, int16 currentHeight) {
	return Common::Point(currentWidth / 2, currentHeight / 2);
}

// FUN_10001a40
Graphics::MacWidget *MMBigImgCastMember::createWidget(Common::Rect &bbox, Channel *channel, SpriteType spriteType) {
	if (!bbox.width() || !bbox.height())
		return nullptr;

	Graphics::MacWindowManager *wm = g_director->_wm;
	Graphics::MacWidget *widget = new Graphics::MacWidget(g_director->getCurrentWindow()->getMacWindow(),
		bbox.left, bbox.top, bbox.width(), bbox.height(), wm, false);
	Graphics::ManagedSurface *surf = widget->getSurface();
	uint32 black = wm->_pixelformat.bytesPerPixel == 1 ? wm->findBestColor(0, 0, 0) : wm->_pixelformat.RGBToColor(0, 0, 0);
	// Without an image the Xtra fills with the window text brush (black);
	// with one it leaves anything past the image's edge unpainted
	surf->fillRect(Common::Rect(bbox.width(), bbox.height()), black);
	if (!_loaded)
		return widget;

	if (_viewW != bbox.width() || _viewH != bbox.height())
		setView(bbox.width(), bbox.height());

	Common::HashMap<uint32, uint32> cache;
	Common::Rect dest(bbox.width(), bbox.height());
	int tileY = 0;
	for (uint row = 0; row < _rows; row++) {
		int tileX = 0;
		int rowHeight = _tiles[row * _cols].h;
		for (uint col = 0; col < _cols; col++) {
			uint index = row * _cols + col;
			Common::Rect tileRect(tileX - _orgX, tileY - _orgY, tileX - _orgX + _tiles[index].w, tileY - _orgY + _tiles[index].h);
			tileX += _tiles[index].w;
			if (!tileRect.intersects(dest))
				continue;
			Graphics::Surface *image = getTile(index);
			if (!image)
				continue;
			Common::Rect clip = tileRect.findIntersectingRect(dest);
			for (int y = clip.top; y < clip.bottom; y++) {
				int sy = y - tileRect.top;
				if (sy >= image->h)
					break;
				for (int x = clip.left; x < clip.right; x++) {
					int sx = x - tileRect.left;
					if (sx >= image->w)
						break;
					byte a, r, g, b;
					image->format.colorToARGB(image->getPixel(sx, sy), a, r, g, b);
					uint32 color;
					if (wm->_pixelformat.bytesPerPixel == 1) {
						uint32 key = (r << 16) | (g << 8) | b;
						if (cache.contains(key)) {
							color = cache[key];
						} else {
							color = wm->findBestColor(r, g, b);
							cache[key] = color;
						}
					} else {
						color = wm->_pixelformat.RGBToColor(r, g, b);
					}
					surf->setPixel(x, y, color);
				}
			}
		}
		tileY += rowHeight;
	}
	return widget;
}

bool MMBigImgCastMember::hasProp(const Common::String &propName) {
	return propName.equalsIgnoreCase("MMFileName") || propName.equalsIgnoreCase("MMOrgX") ||
		propName.equalsIgnoreCase("MMOrgY") || propName.equalsIgnoreCase("MMWidth") ||
		propName.equalsIgnoreCase("MMHeight") || CastMember::hasProp(propName);
}

Datum MMBigImgCastMember::getProp(const Common::String &propName) {
	if (propName.equalsIgnoreCase("MMFileName"))
		return Datum(_fileName);
	if (propName.equalsIgnoreCase("MMOrgX"))
		return Datum(_orgX);
	if (propName.equalsIgnoreCase("MMOrgY"))
		return Datum(_orgY);
	if (propName.equalsIgnoreCase("MMWidth"))
		return Datum(_width);
	if (propName.equalsIgnoreCase("MMHeight"))
		return Datum(_height);
	return CastMember::getProp(propName);
}

void MMBigImgCastMember::setProp(const Common::String &propName, const Datum &value, bool force) {
	if (propName.equalsIgnoreCase("MMFileName")) {
		loadFile(value.asString());
	} else if (propName.equalsIgnoreCase("MMOrgX")) {
		setOrigin(value.asInt(), _orgY);
	} else if (propName.equalsIgnoreCase("MMOrgY")) {
		setOrigin(_orgX, value.asInt());
	} else if (propName.equalsIgnoreCase("MMWidth") || propName.equalsIgnoreCase("MMHeight")) {
		warning("MMBigImgCastMember: %s is read-only", propName.c_str());
	} else {
		CastMember::setProp(propName, value, force);
	}
}

bool MMBigImgCastMember::callSpriteMethod(const Common::String &name, const Common::Array<Datum> &args, Datum &result) {
	if (!name.equalsIgnoreCase("MMSetOrg"))
		return false;
	if (args.size() < 2) {
		warning("MMBigImgCastMember: MMSetOrg needs 2 arguments, got %d", args.size());
		return true;
	}
	setOrigin(args[0].asInt(), args[1].asInt());
	return true;
}

Datum MMBigImgCastMember::getField(int field) {
	switch (field) {
	case kTheCastType:
	case kTheType: {
		Datum d = Common::String("MMBigImg");
		d.type = SYMBOL;
		return d;
	}
	default:
		return CastMember::getField(field);
	}
}

Common::String MMBigImgCastMember::formatInfo() {
	return Common::String::format("fileName: '%s', size: %dx%d, origin: %d,%d, view: %dx%d", _fileName.c_str(),
		_width, _height, _orgX, _orgY, _viewW, _viewH);
}

} // End of namespace Director
