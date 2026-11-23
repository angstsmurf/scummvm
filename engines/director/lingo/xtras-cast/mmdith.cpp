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

#include "graphics/macgui/macwidget.h"
#include "graphics/macgui/macwindowmanager.h"
#include "graphics/surface.h"

#include "image/jpeg.h"

#include "director/director.h"
#include "director/cast.h"
#include "director/castmember/xtra.h"
#include "director/lingo/lingo-the.h"
#include "director/lingo/xtras-cast/mmdith.h"
#include "director/util.h"
#include "director/window.h"

namespace Director {

namespace MMDith {

CastMember *createCastMember(Cast *cast, uint16 castId, XtraCastMember *xtra) {
	return new MMDithCastMember(cast, castId, *xtra);
}

} // End of namespace MMDith

MMDithCastMember::MMDithCastMember(Cast *cast, uint16 castId, XtraCastMember &source)
		: CastMember(cast, castId) {
	_type = kCastXtra;
	CastMemberInfo *ci = getInfo();
	if (ci && !ci->xtraRect.isEmpty())
		_initialRect = Common::Rect(ci->xtraRect.width(), ci->xtraRect.height());
}

MMDithCastMember::MMDithCastMember(Cast *cast, uint16 castId, MMDithCastMember &source)
		: CastMember(cast, castId) {
	_type = kCastXtra;
	_initialRect = source._initialRect;
	_boundingRect = source._boundingRect;
	_loaded = source._loaded;
	if (cast == source._cast)
		_children = source._children;
	if (!source._fileName.empty())
		loadImage(source._fileName);
}

MMDithCastMember::~MMDithCastMember() {
	if (_image) {
		_image->free();
		delete _image;
	}
}

void MMDithCastMember::loadImage(const Common::String &fileName) {
	_fileName = fileName;
	if (_image) {
		_image->free();
		delete _image;
		_image = nullptr;
	}
	setModified(true);

	if (fileName.empty())
		return;

	Common::Path location = findPath(fileName);
	Common::SeekableReadStream *stream = location.empty() ? nullptr : Common::MacResManager::openFileOrDataFork(location);
	if (!stream) {
		warning("MMDithCastMember: cannot open '%s'", fileName.c_str());
		return;
	}

	Image::JPEGDecoder decoder;
	decoder.setOutputPixelFormat(Graphics::PixelFormat::createFormatRGBA32());
	if (decoder.loadStream(*stream) && decoder.getSurface()) {
		_image = decoder.getSurface()->convertTo(Graphics::PixelFormat::createFormatRGBA32());
		_initialRect = Common::Rect(_image->w, _image->h);
		debugC(3, kDebugLoading, "MMDithCastMember %d: loaded '%s', %dx%d", _castId, fileName.c_str(), _image->w, _image->h);
	} else {
		warning("MMDithCastMember: cannot decode '%s'", fileName.c_str());
	}
	delete stream;
}

// The image is centred on the sprite's loc
Common::Point MMDithCastMember::getRegistrationOffset() {
	return Common::Point(_initialRect.width() / 2, _initialRect.height() / 2);
}

Common::Point MMDithCastMember::getRegistrationOffset(int16 currentWidth, int16 currentHeight) {
	return Common::Point(currentWidth / 2, currentHeight / 2);
}

Graphics::MacWidget *MMDithCastMember::createWidget(Common::Rect &bbox, Channel *channel, SpriteType spriteType) {
	if (!_image || !bbox.width() || !bbox.height())
		return nullptr;

	Graphics::MacWindowManager *wm = g_director->_wm;
	Graphics::MacWidget *widget = new Graphics::MacWidget(g_director->getCurrentWindow()->getMacWindow(),
		bbox.left, bbox.top, bbox.width(), bbox.height(), wm, false);
	Graphics::ManagedSurface *surf = widget->getSurface();

	// Scale to the sprite, nearest neighbour; the Xtra draws the image at
	// its own size, which Lingo gives the sprite
	Common::HashMap<uint32, uint32> cache;
	const Graphics::PixelFormat &src = _image->format;
	for (int y = 0; y < bbox.height(); y++) {
		int sy = y * _image->h / bbox.height();
		for (int x = 0; x < bbox.width(); x++) {
			int sx = x * _image->w / bbox.width();
			uint32 pixel = _image->getPixel(sx, sy);
			byte a, r, g, b;
			src.colorToARGB(pixel, a, r, g, b);
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
	return widget;
}

bool MMDithCastMember::hasProp(const Common::String &propName) {
	return propName.equalsIgnoreCase("mmDithFileName") || propName.equalsIgnoreCase("MMDithWidth") ||
		propName.equalsIgnoreCase("MMDithHeight") || CastMember::hasProp(propName);
}

Datum MMDithCastMember::getProp(const Common::String &propName) {
	if (propName.equalsIgnoreCase("mmDithFileName"))
		return Datum(_fileName);
	if (propName.equalsIgnoreCase("MMDithWidth"))
		return Datum(_image ? (int)_image->w : 0);
	if (propName.equalsIgnoreCase("MMDithHeight"))
		return Datum(_image ? (int)_image->h : 0);
	return CastMember::getProp(propName);
}

void MMDithCastMember::setProp(const Common::String &propName, const Datum &value, bool force) {
	if (propName.equalsIgnoreCase("mmDithFileName")) {
		loadImage(value.asString());
		return;
	}
	CastMember::setProp(propName, value, force);
}

Datum MMDithCastMember::getField(int field) {
	switch (field) {
	case kTheCastType:
	case kTheType: {
		Datum d = Common::String("MMDith");
		d.type = SYMBOL;
		return d;
	}
	default:
		return CastMember::getField(field);
	}
}

Common::String MMDithCastMember::formatInfo() {
	return Common::String::format("fileName: '%s', size: %dx%d", _fileName.c_str(),
		_image ? _image->w : 0, _image ? _image->h : 0);
}

} // End of namespace Director
