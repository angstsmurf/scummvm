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

#include "director/director.h"
#include "director/movie.h"
#include "director/score.h"
#include "director/cast.h"
#include "director/channel.h"
#include "director/picture.h"
#include "director/sprite.h"
#include "director/types.h"
#include "director/window.h"
#include "director/castmember/castmember.h"
#include "director/castmember/bitmap.h"
#include "director/castmember/digitalvideo.h"
#include "director/castmember/filmloop.h"
#include "director/castmember/movie.h"
#include "director/castmember/text.h"

#include "graphics/macgui/mactext.h"
#include "graphics/macgui/macbutton.h"

namespace Director {

Channel::Channel(Score *sc, Sprite *sp, int priority) {
	_score = sc;
	if (!sp)
		_sprite = nullptr;
	else
		_sprite = new Sprite(*sp);

	_widget = nullptr;
	_constraint = 0;
	_mask = nullptr;

	_priority = priority;

	_movieRate = 0.0;
	_movieTime = 0;
	_startTime = 0;
	_stopTime = 0;

	_filmLoopFrame = 0;

	_visible = true;
	_widgetDirty = true;
	_needsDraw = false;
	_hideFromStage = false;
	_lastTrail = false;
	_hasDragPosition = false;
	_hasLingoBbox = false;

	_scriptInstanceList.type = ARRAY;
	_scriptInstanceList.u.farr = new FArray;

	if (sp) {
		_startFrame = sp->_spriteInfo.startFrame;
		_endFrame = sp->_spriteInfo.endFrame;
	} else {
		_startFrame = -1;
		_endFrame = -1;
	}
}

Channel::Channel(const Channel &channel) {
	*this = channel;
}

Channel& Channel::operator=(const Channel &channel) {
	_score = channel._score;
	_sprite = channel._sprite ? new Sprite(*channel._sprite) : nullptr;

	_widget = nullptr;
	_constraint = channel._constraint;
	_mask = nullptr;
	_quadSurface = nullptr;
	_quadMask = nullptr;
	_quadValid = false;
	_quadKeySrf = nullptr;

	_priority = channel._priority;

	_movieRate = channel._movieRate;
	_movieTime = channel._movieTime;
	_startTime = channel._startTime;
	_stopTime = channel._stopTime;

	_filmLoopFrame = channel._filmLoopFrame;

	_visible = channel._visible;
	_widgetDirty = channel._widgetDirty;
	_hideFromStage = channel._hideFromStage;
	_hasDragPosition = channel._hasDragPosition;
	_dragPosition = channel._dragPosition;
	_hasLingoBbox = channel._hasLingoBbox;
	_lingoBbox = channel._lingoBbox;

	_startFrame = channel._startFrame;
	_endFrame = channel._endFrame;

	// Script instances are not copied; each channel owns its list
	if (_scriptInstanceList.type != ARRAY) {
		_scriptInstanceList.type = ARRAY;
		_scriptInstanceList.u.farr = new FArray;
	}

	return *this;
}


Channel::~Channel() {
	// A digital video cast member can outlive this channel.
	if (_score && _sprite && _sprite->_cast && _sprite->_cast->_type == kCastDigitalVideo) {
		DigitalVideoCastMember *video = (DigitalVideoCastMember *)_sprite->_cast;
		if (video->_channel == this) {
			video->setChannel(nullptr);
		}
	}

	if (_widget) {
		delete _widget;
	}

	if (_mask)
		delete _mask;
	delete _quadSurface;
	delete _quadMask;
	if (_sprite)
		delete _sprite;
}

// Rich text drawn with Background Transparent ink has no background
// colour to key out: its anti-aliased text carries per-pixel alpha.
bool Channel::needsSrcAlpha(const Graphics::ManagedSurface *srf) const {
	return srf && _sprite->_cast && _sprite->_cast->_type == kCastRichText &&
		_sprite->_ink == kInkTypeBackgndTrans && srf->format.bytesPerPixel == 4 &&
		srf->format.aBits() > 0;
}

DirectorPlotData Channel::getPlotData() {
	int blend = (_sprite->_thickness & kTHasBlend) || _sprite->_ink == kInkTypeBlend ? _sprite->_blendAmount : 0;
	DirectorPlotData pd(g_director, _sprite->getMemberType(), _sprite->_ink, blend, _sprite->getBackColor(), _sprite->getForeColor());
	pd.colorWhite = g_director->getColorWhite();
	pd.colorBlack = g_director->getColorBlack();
	pd.dst = nullptr;

	pd.srf = getSurface();
	if (_sprite->_hasQuad && pd.srf) {
		buildQuad(pd.srf, false);
		pd.srf = _quadSurface;
	}
	if (_sprite->getMemberType() == kBitmapSprite &&
		_sprite->_cast && _sprite->_cast->_type == kCastBitmap &&
		((BitmapCastMember *)_sprite->_cast)->_bitsPerPixel == 1) {
		// Add override flag for 1-bit images
		pd.oneBitImage = true;
	}
	// Videos are rendered as an overlay, ink doesn't affect them.
	if (_sprite->_cast && _sprite->_cast->_type == kCastDigitalVideo) {
		pd.ink = kInkTypeCopy;
	}

	if (!pd.srf && _sprite->getMemberType() != kBitmapSprite) {
		// Shapes come colourized from macDrawPixel
		pd.ms = _sprite->getShape();
		pd.applyColor = false;
	// Disable custom fgColor/bgColor blits for videos
	} else if (_sprite->_cast && _sprite->_cast->_type == kCastDigitalVideo) {
		pd.applyColor = false;
	} else if (_sprite->_cast && _sprite->_cast->hasOwnColors()) {
		pd.applyColor = false;
		uint32 ownBack;
		if (_sprite->_cast->getOwnBackColor(ownBack))
			pd.backColor = ownBack;
	} else {
		pd.setApplyColor();
	}

	if (needsSrcAlpha(pd.srf))
		pd.srcAlpha = true;

	pd.srfMask = nullptr;
	// A D2/D3 text member on a button sprite is drawn as a button, which
	// keeps its frame like a button member does.
	if (_sprite->_cast && _sprite->_cast->_type == kCastText && !isButtonSprite(_sprite->_spriteType)) {
		// kInkTypeCopy -- no mask, default rendering

		if (_sprite->_ink == kInkTypeMatte || _sprite->_ink == kInkTypeNotCopy
			|| _sprite->_ink == kInkTypeNotTrans || _sprite->_ink == kInkTypeNotReverse
			|| _sprite->_ink == kInkTypeNotGhost || _sprite->_ink == kInkTypeAdd
			|| _sprite->_ink == kInkTypeAddPin || _sprite->_ink == kInkTypeSub
			|| _sprite->_ink == kInkTypeSubPin || _sprite->_ink == kInkTypeLight
			|| _sprite->_ink == kInkTypeBlend) {
			Graphics::MacText *widget = ((TextCastMember *)_sprite->_cast)->getWidget();
			if (widget)
				pd.srfMask = widget->getCharBoxMask();
		} else if (_sprite->_ink == kInkTypeTransparent || _sprite->_ink == kInkTypeBackgndTrans
				|| _sprite->_ink == kInkTypeReverse || _sprite->_ink == kInkTypeGhost
				|| _sprite->_ink == kInkTypeMask || _sprite->_ink == kInkTypeDark) {
			Graphics::MacText *widget = ((TextCastMember *)_sprite->_cast)->getWidget();
			if (widget)
				pd.srfMask = widget->getGlyphMask();
		}
	}

	return pd;
}

Graphics::ManagedSurface *Channel::getSurface() {
	if (_widget) {
		return _widget->getSurface();
	} else {
		return nullptr;
	}
}

const Graphics::Surface *Channel::getMask(bool forceMatte) {
	if (_sprite->_hasQuad && _sprite->_cast && _sprite->getMemberType() != kTextSprite) {
		Graphics::ManagedSurface *srf = getSurface();
		if (!srf)
			return getSourceMask(forceMatte);
		buildQuad(srf, forceMatte);
		return _quadMask ? &_quadMask->rawSurface() : nullptr;
	}
	return getSourceMask(forceMatte);
}

// Warps the sprite's surface, drawn at the bbox, to its quad: each pixel of
// the bbox is mapped back through the projective transform of the unit
// square onto the quad. Pixels outside the quad, or outside the ink's own
// matte, are left out of the mask.
void Channel::buildQuad(Graphics::ManagedSurface *srf, bool forceMatte) {
	Common::Rect bbox = getBbox();
	int w = bbox.width(), h = bbox.height();

	// getPlotData and getMask both ask for it on every draw
	if (_quadValid && _quadKeySrf == srf && _quadKeyBbox == bbox && _quadKeyInk == _sprite->_ink &&
			_quadKeyMatte == forceMatte && !memcmp(_quadKeyQuad, _sprite->_quad, sizeof(_quadKeyQuad)))
		return;
	_quadValid = true;
	_quadKeySrf = srf;
	_quadKeyBbox = bbox;
	_quadKeyInk = _sprite->_ink;
	_quadKeyMatte = forceMatte;
	memcpy(_quadKeyQuad, _sprite->_quad, sizeof(_quadKeyQuad));

	if (!_quadSurface || _quadSurface->w != w || _quadSurface->h != h || _quadSurface->format != srf->format) {
		delete _quadSurface;
		_quadSurface = new Graphics::ManagedSurface(MAX(w, 1), MAX(h, 1), srf->format);
	}
	if (!_quadMask || _quadMask->w != _quadSurface->w || _quadMask->h != _quadSurface->h) {
		delete _quadMask;
		_quadMask = new Graphics::ManagedSurface(_quadSurface->w, _quadSurface->h, Graphics::PixelFormat::createFormatCLUT8());
	}
	_quadSurface->clear();
	_quadMask->clear(0);

	// The widget keeps its size while the quad changes, so take the mask at
	// the source's size: it then stays cached and lines up with the pixels.
	const Graphics::Surface *srcMask = getSourceMask(forceMatte, srf->w, srf->h);

	// Square to quad (Heckbert): x = (a u + b v + c) / (g u + h v + 1)
	const float (*q)[2] = _sprite->_quad;
	double x0 = q[0][0], y0 = q[0][1], x1 = q[1][0], y1 = q[1][1];
	double x2 = q[2][0], y2 = q[2][1], x3 = q[3][0], y3 = q[3][1];
	double sx = x0 - x1 + x2 - x3, sy = y0 - y1 + y2 - y3;
	double a, b, c, d, e, f, g = 0.0, hh = 0.0;
	if (fabs(sx) < 1e-9 && fabs(sy) < 1e-9) {
		a = x1 - x0; b = x3 - x0; d = y1 - y0; e = y3 - y0;
	} else {
		double dx1 = x1 - x2, dx2 = x3 - x2, dy1 = y1 - y2, dy2 = y3 - y2;
		double den = dx1 * dy2 - dx2 * dy1;
		if (fabs(den) < 1e-9)
			return;
		g = (sx * dy2 - dx2 * sy) / den;
		hh = (dx1 * sy - sx * dy1) / den;
		a = x1 - x0 + g * x1; b = x3 - x0 + hh * x3;
		d = y1 - y0 + g * y1; e = y3 - y0 + hh * y3;
	}
	c = x0;
	f = y0;

	// Inverse through the adjugate; the scale cancels out
	double A = e - f * hh, B = c * hh - b, C = b * f - c * e;
	double D = f * g - d, E = a - c * g, F = c * d - a * f;
	double G = d * hh - e * g, H = b * g - a * hh, I = a * e - b * d;
	if (fabs(I) < 1e-9 && fabs(G) < 1e-9 && fabs(H) < 1e-9)
		return;

	int bpp = srf->format.bytesPerPixel;
	for (int y = 0; y < h; y++) {
		double py = bbox.top + y + 0.5;
		for (int x = 0; x < w; x++) {
			double px = bbox.left + x + 0.5;
			double ww = G * px + H * py + I;
			if (fabs(ww) < 1e-12)
				continue;
			double u = (A * px + B * py + C) / ww;
			double v = (D * px + E * py + F) / ww;
			if (u < 0.0 || u >= 1.0 || v < 0.0 || v >= 1.0)
				continue;
			int su = CLIP<int>((int)(u * srf->w), 0, srf->w - 1);
			int sv = CLIP<int>((int)(v * srf->h), 0, srf->h - 1);
			if (srcMask && (su >= srcMask->w || sv >= srcMask->h || !*(const byte *)srcMask->getBasePtr(su, sv)))
				continue;
			memcpy(_quadSurface->getBasePtr(x, y), srf->getBasePtr(su, sv), bpp);
			*(byte *)_quadMask->getBasePtr(x, y) = 0xff;
		}
	}
}

const Graphics::Surface *Channel::getSourceMask(bool forceMatte, int w, int h) {
	if (!_sprite->_cast || _sprite->getMemberType() == kTextSprite)
		return nullptr;

	bool needsMatte = _sprite->_ink == kInkTypeMatte ||
		_sprite->_ink == kInkTypeNotCopy ||
		_sprite->_ink == kInkTypeNotTrans ||
		_sprite->_ink == kInkTypeNotReverse ||
		_sprite->_ink == kInkTypeNotGhost ||
		_sprite->_ink == kInkTypeBlend ||
		_sprite->_ink == kInkTypeAddPin ||
		_sprite->_ink == kInkTypeAdd ||
		_sprite->_ink == kInkTypeSubPin ||
		_sprite->_ink == kInkTypeLight ||
		_sprite->_ink == kInkTypeSub ||
		_sprite->_ink == kInkTypeDark ||
		(((_sprite->_thickness & kTHasBlend) || _sprite->_ink == kInkTypeBlend) && _sprite->_blendAmount > 0);

	if (!_sprite->isQDShape() && _sprite->_ink == kInkTypeCopy && _sprite->_thickness & kTHasBlend)
		needsMatte = true;

	Common::Rect bbox(getBbox());
	if (w > 0 && h > 0) {
		bbox.setWidth(w);
		bbox.setHeight(h);
	}

	if (needsMatte || forceMatte) {
		// Mattes are only supported in bitmaps for now. Shapes don't need mattes,
		// as they already have all non-enclosed white pixels transparent.
		// Matte on text has a trivial enough effect to not worry about implementing.
		if (_sprite->_cast->_type == kCastBitmap) {
			BitmapCastMember *bitmap = ((BitmapCastMember *)_sprite->_cast);
			// 1-bit images only require a matte for the matte ink type
			if (bitmap->_bitsPerPixel == 1 && _sprite->_ink != kInkTypeMatte) {
				// 1-bit images will not blend with kInkTypeCopy, whereas 8-bit images will.
				if (_sprite->_ink == kInkTypeCopy) {
                    _sprite->_blendAmount = 0;
                }
				return nullptr;
			}
			return bitmap->getMatte(bbox);
		} else {
			return nullptr;
		}
	} else if (_sprite->_ink == kInkTypeMask) {
		CastMemberID maskID(_sprite->_castId.member + 1, _sprite->_castId.castLib);
		CastMember *member = g_director->getCurrentMovie()->getCastMember(maskID);

		if (member) {
			if (member->_type != kCastBitmap) {
				warning("Channel::getMask(): Requested cast mask %s, but type is %s, not bitmap", maskID.asString().c_str(), castType2str(member->_type));
				return nullptr;
			}
			BitmapCastMember *bitmap = (BitmapCastMember *)member;
			if (_mask) {
				delete _mask;
				_mask = nullptr;
			}
			if (bitmap->_picture) {
				// D7 movies can use a 32-bit (JPEG) mask: dark pixels let the
				// sprite through, like the black pixels of a 1-bit mask.
				// Bad Milk's portal (scrunch.dxr invertPortalmask).
				const Graphics::Surface *maskSrc = &bitmap->_picture->_surface;
				Graphics::Surface converted;
				if (bitmap->_bitsPerPixel != 1 && maskSrc->format.bytesPerPixel > 1) {
					converted.create(maskSrc->w, maskSrc->h, Graphics::PixelFormat::createFormatCLUT8());
					for (int y = 0; y < maskSrc->h; y++) {
						for (int x = 0; x < maskSrc->w; x++) {
							uint8 r, g, b;
							maskSrc->format.colorToRGB(maskSrc->getPixel(x, y), r, g, b);
							*(byte *)converted.getBasePtr(x, y) = (r * 30 + g * 59 + b * 11) / 100 < 128 ? 0xff : 0;
						}
					}
					maskSrc = &converted;
				} else if (bitmap->_bitsPerPixel != 1) {
					warning("Channel::getMask(): Requested cast mask %s, but bitmap isn't 1bpp", maskID.asString().c_str());
					return nullptr;
				}
				// reposition channel bounding box, so origin is at registration offset
				Common::Point originPos = getPosition();
				bbox.translate(-originPos.x, -originPos.y);
				// create new mask surface, with the exact dimensions of the channel.
				_mask = new Graphics::ManagedSurface(bbox.width(), bbox.height());
				// get the bounding box of the mask image (origin at registration offset)
				Common::Rect destRect = bitmap->getBbox();
				// get position of channel's registration offset (origin at top left)
				Common::Point channelRegOffset(-bbox.left, -bbox.top);
				// move destination rect to sit at the channel's registration offset
				destRect.translate(channelRegOffset.x, channelRegOffset.y);
				Common::Point destOrigin(destRect.left, destRect.top);
				// clip the destination rect so it is contained within the mask bounds
				destRect.clip(_mask->getBounds());
				// make a copy of the destination rect with the origin at the top left of the mask bitmap
				Common::Rect srcRect = destRect;
				srcRect.translate(-destOrigin.x, -destOrigin.y);
				debugC(8, kDebugImages, "Channel::getMask(): cast mask %s, orig %dx%d, dest %dx%d, crop %d,%d %dx%d",  maskID.asString().c_str(), bitmap->_picture->_surface.w, bitmap->_picture->_surface.h, bbox.width(), bbox.height(), destRect.left, destRect.top, destRect.width(), destRect.height());
				_mask->copyRectToSurface(*maskSrc, destRect.left, destRect.top, srcRect);
				converted.free();
				return &_mask->rawSurface();
			} else {
				warning("Channel::getMask(): Requested cast mask %s, but no picture found", maskID.asString().c_str());
				return nullptr;
			}
		} else {
			warning("Channel::getMask(): Requested cast mask %s, but was not found", maskID.asString().c_str());
			return nullptr;
		}
	}

	return nullptr;
}

// TODO: eliminate this function when we got the correct method to deal with sprite size
// since we didn't handle sprites very well for text cast members. thus we don't replace our text castmembers when only size changes
// for explicitly changing, we have isModified to check
bool hasTextCastMember(Sprite *sprite) {
	if (sprite && sprite->_cast)
		return sprite->_cast->_type == kCastText || sprite->_cast->_type == kCastButton;
	return false;
}

// Lingo stores a size. From D3 on this turns the stretch flag on,
// even if the value is same that the sprite already has, and the
// sprite will be drawn at the stored size. D2 goes on drawing
// an unstretched sprite at the size of its member.
void Channel::setWidth(int w) {
	keepLingoBbox();
	_sprite->setWidth(w);
	if (g_director->getVersion() >= 300)
		_sprite->_stretch = true;
	replaceWidget();
	setNeedsDraw();
}

void Channel::setHeight(int h) {
	keepLingoBbox();
	_sprite->setHeight(h);
	if (g_director->getVersion() >= 300)
		_sprite->_stretch = true;
	replaceWidget();
	setNeedsDraw();
}

// The box is given as two corners, in any order
void Channel::setBbox(int l, int t, int r, int b) {
	uint16 version = g_director->getVersion();
	bool setStretch = version >= 300;

	// The order of the corners picks the direction of a line, except in
	// D6. Before D7, a line that keeps its direction keeps its stretch.
	if (version < 600 || version >= 700) {
		bool keepsType = false;
		if (_sprite->_spriteType == kLineTopBottomSprite) {
			if ((l < r && t < b) || (r < l && b < t))
				keepsType = true;
			else
				_sprite->_spriteType = kLineBottomTopSprite;
		} else if (_sprite->_spriteType == kLineBottomTopSprite) {
			if ((l < r && b < t) || (r < l && t < b))
				keepsType = true;
			else
				_sprite->_spriteType = kLineTopBottomSprite;
		}
		if (keepsType && version < 700)
			setStretch = false;
	}

	keepLingoBbox();
	if (setStretch)
		_sprite->_stretch = true;
	_sprite->setBbox(MIN(l, r), MIN(t, b), MAX(l, r), MAX(t, b));
	replaceWidget();
	setNeedsDraw();
}

// Lingo is about to move or resize the sprite. The left, top, right and
// bottom go on returning the rect that was drawn until the sprite is
// drawn again.
void Channel::keepLingoBbox() {
	if (_score && _score->spriteRectsFollowLingo()) {
		_hasLingoBbox = false;
		return;
	}

	if (!_hasLingoBbox) {
		_lingoBbox = getBbox();
		_hasLingoBbox = true;
	}
}

bool Channel::isDirty(Sprite *nextSprite) {
	// When a sprite is puppeted setTheSprite ensures that the dirty flag here is
	// set. Otherwise, we need to rerender when the position, bounding box, or
	// cast of the sprite changes.
	if (!nextSprite)
		return false;

	bool isDirtyFlag = _widgetDirty ||
		(_sprite->_cast && _sprite->_cast->isModified());

	if (_sprite && !_sprite->_puppet && !_sprite->_autoPuppet) {
		// When puppet is set, the overall dirty flag should be set when sprite is
		// modified.
		isDirtyFlag |= _sprite->_castId != nextSprite->_castId ||
			_sprite->_ink != nextSprite->_ink ||
			// Compare palette indices: in 32bpp the converted colors depend on
			// the palette active when each frame was loaded.
			_sprite->_backColorIndex != nextSprite->_backColorIndex ||
			_sprite->_foreColorIndex != nextSprite->_foreColorIndex ||
			(_sprite->_colorcode & (kColorCodeForeRGB | kColorCodeBackRGB)) != (nextSprite->_colorcode & (kColorCodeForeRGB | kColorCodeBackRGB)) ||
			_sprite->_fgColorG != nextSprite->_fgColorG || _sprite->_fgColorB != nextSprite->_fgColorB ||
			_sprite->_bgColorG != nextSprite->_bgColorG || _sprite->_bgColorB != nextSprite->_bgColorB ||
			_sprite->_editable != nextSprite->_editable ||
			// A score frame may make a sprite moveable without changing anything
			// else about it (Alice: STONE.DIR, frames 44-47).
			_sprite->_moveable != nextSprite->_moveable ||
			_sprite->_blendAmount != nextSprite->_blendAmount ||
			_sprite->getDrawThickness() != nextSprite->getDrawThickness();
		// A score frame may change only the sprite type, e.g. to show a
		// sprite that was inactive (type 0) in the previous frame (Pingu
		// KUZIRA.MMM, frames 2-3). Only a type the score wrote counts: a
		// type set by Lingo (castNum) is kept until the score changes it.
		if (nextSprite->_copyBackMask & kSCBSpriteType)
			isDirtyFlag |= _sprite->_spriteType != nextSprite->_spriteType ||
				_sprite->_enabled != nextSprite->_enabled;
		if (g_director->getVersion() < 600) {
			// A moveable sprite with a drag position stays there, see
			// updateDragPosition(); otherwise it follows the score.
			Common::Point pos = (nextSprite->_moveable && _hasDragPosition) ? _dragPosition : nextSprite->getPosition();
			isDirtyFlag |= _sprite->getPosition() != pos;
		} else if (!_sprite->_moveable) {
			isDirtyFlag |= _sprite->getPosition() != nextSprite->getPosition();
		}
		if (isStretched() && !hasTextCastMember(_sprite))
			isDirtyFlag |= _sprite->_width != nextSprite->_width || _sprite->_height != nextSprite->_height;
	} else if (_sprite && !_sprite->_puppet) {
		// Lingo set some fields of this sprite (D6+); the score still
		// changes the others, see Sprite::replaceFrom().
		bool keepRect = _sprite->getAutoPuppet(kAPBbox) || _sprite->getAutoPuppet(kAPRect);
		bool keepLoc = keepRect || _sprite->getAutoPuppet(kAPLoc) ||
			(g_director->getVersion() >= 700 && _sprite->_moveable && nextSprite->_moveable);
		uint32 mask = nextSprite->_copyBackMask;
		if (!_sprite->getAutoPuppet(kAPCast) && !_sprite->getAutoPuppet(kAPMember) && (mask & kSCBCastId))
			isDirtyFlag |= _sprite->_castId != nextSprite->_castId;
		if (!_sprite->getAutoPuppet(kAPInk) && (mask & kSCBInk))
			isDirtyFlag |= _sprite->_ink != nextSprite->_ink;
		if (!_sprite->getAutoPuppet(kAPForeColor) && (mask & kSCBForeColor))
			isDirtyFlag |= _sprite->_foreColorIndex != nextSprite->_foreColorIndex;
		if (!_sprite->getAutoPuppet(kAPBackColor) && (mask & kSCBBackColor))
			isDirtyFlag |= _sprite->_backColorIndex != nextSprite->_backColorIndex;
		if (!keepLoc && !_sprite->getAutoPuppet(kAPLocH) && (mask & kSCBLocH))
			isDirtyFlag |= _sprite->getPosition().x != nextSprite->getPosition().x;
		if (!keepLoc && !_sprite->getAutoPuppet(kAPLocV) && (mask & kSCBLocV))
			isDirtyFlag |= _sprite->getPosition().y != nextSprite->getPosition().y;
		if (!keepRect && !_sprite->getAutoPuppet(kAPWidth) && (mask & (kSCBWidth | kSCBCastId)))
			isDirtyFlag |= _sprite->_width != nextSprite->_width;
		if (!keepRect && !_sprite->getAutoPuppet(kAPHeight) && (mask & (kSCBHeight | kSCBCastId)))
			isDirtyFlag |= _sprite->_height != nextSprite->_height;
		if (!_sprite->getAutoPuppet(kAPBlend) && (mask & kSCBBlendAmount))
			isDirtyFlag |= _sprite->_blendAmount != nextSprite->_blendAmount;
		isDirtyFlag |= _sprite->_moveable != nextSprite->_moveable;
	}

	return isDirtyFlag;
}

Common::Rect Channel::getRollOverBbox() {
	// In D4 and below, the rollOver command will check against whatever the last
	// contents of the sprite were, regardless of whether the score has zeroed it out.
	// A D2/D3 QuickDraw shape has no cast member, but is not blank.
	if (g_director->getVersion() < 500 && _sprite->_castId.member == 0 && !_sprite->isQDShape()) {
		return _rollOverBbox;
	}
	return getBbox();
}

bool Channel::isStretched() {
	return _sprite->_stretch;
}

bool Channel::isEmpty() {
	return (_sprite->_spriteType == kInactiveSprite);
}

// A sprite of type 16 takes its type from its cast member.
// Director 5 scores give field sprites this type.
bool Channel::isTextSprite() {
	return _sprite->getMemberType() == kTextSprite;
}

bool Channel::isActiveText() {
	if (!isTextSprite())
		return false;

	if (_widget && _widget->hasAllFocus())
		return true;

	return false;
}

CollisionTest Channel::isMouseIn(const Common::Point &pos) {
	if (!_visible)
		return kCollisionNo;

	const Common::Rect bbox = getBbox();

	if (_sprite->_cast) {
		return _sprite->_cast->isWithin(bbox, pos, _sprite->_ink);
	} else if (!bbox.contains(pos)) {
		return kCollisionNo;
	}

	return kCollisionYes;
}

bool Channel::isMatteIntersect(Channel *channel) {
	Common::Rect myBbox = getBbox();
	Common::Rect yourBbox = channel->getBbox();
	Common::Rect intersectRect = myBbox.findIntersectingRect(yourBbox);

	if (intersectRect.isEmpty())
		return false;
	Graphics::Surface *myMatte = nullptr;
	Graphics::Surface *yourMatte = nullptr;

	if (_sprite->_cast && _sprite->_cast->_type == kCastBitmap)
		myMatte = ((BitmapCastMember *)_sprite->_cast)->getMatte(myBbox);
	if (channel->_sprite->_cast && channel->_sprite->_cast->_type == kCastBitmap)
		yourMatte = ((BitmapCastMember *)channel->_sprite->_cast)->getMatte(yourBbox);

	// A bitmap without white pixels has no matte: all of its box is solid
	for (int i = intersectRect.top; i < intersectRect.bottom; i++) {
		const byte *my = myMatte ? (const byte *)myMatte->getBasePtr(intersectRect.left - myBbox.left, i - myBbox.top) : nullptr;
		const byte *your = yourMatte ? (const byte *)yourMatte->getBasePtr(intersectRect.left - yourBbox.left, i - yourBbox.top) : nullptr;

		for (int j = 0; j < intersectRect.width(); j++)
			if ((!my || my[j]) && (!your || your[j]))
				return true;
	}

	return false;
}

bool Channel::isMatteBoxIntersect(Channel *channel) {
	Common::Rect myBbox = getBbox();
	Common::Rect yourBbox = channel->getBbox();
	Common::Rect intersectRect = myBbox.findIntersectingRect(yourBbox);

	if (intersectRect.isEmpty())
		return false;
	Graphics::Surface *myMatte = nullptr;

	if (_sprite->_cast && _sprite->_cast->_type == kCastBitmap)
		myMatte = ((BitmapCastMember *)_sprite->_cast)->getMatte(myBbox);

	if (!myMatte)
		return true;

	for (int i = intersectRect.top; i < intersectRect.bottom; i++) {
		const byte *my = (const byte *)myMatte->getBasePtr(intersectRect.left - myBbox.left, i - myBbox.top);

		for (int j = intersectRect.left; j < intersectRect.right; j++, my++)
			if (*my)
				return true;
	}

	return false;
}


// The bounding box of channel lies within my matte
bool Channel::isMatteBoxWithin(Channel *channel) {
	Common::Rect myBbox = getBbox();
	Common::Rect yourBbox = channel->getBbox();

	// D7 only looks at the part of the box that overlaps mine
	if (g_director->getVersion() >= 700)
		yourBbox = myBbox.findIntersectingRect(yourBbox);

	if (yourBbox.isEmpty() || !myBbox.contains(yourBbox))
		return false;
	Graphics::Surface *myMatte = nullptr;

	if (_sprite->_cast && _sprite->_cast->_type == kCastBitmap)
		myMatte = ((BitmapCastMember *)_sprite->_cast)->getMatte(myBbox);

	if (!myMatte)
		return true;

	for (int i = yourBbox.top; i < yourBbox.bottom; i++) {
		const byte *my = (const byte *)myMatte->getBasePtr(yourBbox.left - myBbox.left, i - myBbox.top);

		for (int j = yourBbox.left; j < yourBbox.right; j++, my++)
			if (!*my)
				return false;
	}

	return true;
}

// this contains channel. i.e. myBox contain yourBox
bool Channel::isMatteWithin(Channel *channel) {
	Common::Rect myBbox = getBbox();
	Common::Rect yourBbox = channel->getBbox();
	Common::Rect intersectRect = myBbox.findIntersectingRect(yourBbox);

	if (!myBbox.contains(yourBbox))
		return false;
	Graphics::Surface *myMatte = nullptr;
	Graphics::Surface *yourMatte = nullptr;

	if (_sprite->_cast && _sprite->_cast->_type == kCastBitmap)
		myMatte = ((BitmapCastMember *)_sprite->_cast)->getMatte(myBbox);
	if (channel->_sprite->_cast && channel->_sprite->_cast->_type == kCastBitmap)
		yourMatte = ((BitmapCastMember *)channel->_sprite->_cast)->getMatte(yourBbox);

	for (int i = intersectRect.top; i < intersectRect.bottom; i++) {
		const byte *my = myMatte ? (const byte *)myMatte->getBasePtr(intersectRect.left - myBbox.left, i - myBbox.top) : nullptr;
		const byte *your = yourMatte ? (const byte *)yourMatte->getBasePtr(intersectRect.left - yourBbox.left, i - yourBbox.top) : nullptr;

		for (int j = 0; j < intersectRect.width(); j++)
			if (my && !my[j] && (!your || your[j]))
				return false;
	}

	return true;
}

bool Channel::isActiveVideo() {
	if (_sprite && (!_sprite->_cast || _sprite->_cast->_type != kCastDigitalVideo))
		return false;

	return true;
}

void Channel::updateVideoTime() {
	if (isActiveVideo())
		_movieTime = ((DigitalVideoCastMember *)_sprite->_cast)->getMovieCurrentTime();
}

bool Channel::isVideoDirectToStage() {
	if (!_sprite->_cast || _sprite->_cast->_type != kCastDigitalVideo)
		return false;

	return ((DigitalVideoCastMember *)_sprite->_cast)->_directToStage;
}

void Channel::setAutoType() {
	// Starting in D4, setting a sprite's castNum also sets its type
	// to an appropriate default. Before D4 it had to be set manually.
	// QDShapes have no associated cast, so their type must not change.
	if (g_director->getVersion() < 400 || _sprite->isQDShape() || _sprite->_castId == CastMemberID(0, 0))
		return;

	_sprite->_spriteType = kCastMemberSprite;
	// From D5, the type stays 16; see Sprite::getMemberType()
	if (!_sprite->_cast || g_director->getVersion() >= 500)
		return;

	// Only bitmap, text and buttons get a more specific type
	switch (_sprite->_cast->_type) {
	case kCastBitmap:
		_sprite->_spriteType = kBitmapSprite;
		break;
	case kCastText:
		_sprite->_spriteType = kTextSprite;
		break;
	case kCastButton:
		switch (((TextCastMember *)_sprite->_cast)->_buttonType) {
		case kTypeButton:
			_sprite->_spriteType = kButtonSprite;
			break;
		case kTypeCheckBox:
			_sprite->_spriteType = kCheckboxSprite;
			break;
		case kTypeRadio:
			_sprite->_spriteType = kRadioButtonSprite;
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}
}

void Channel::setCast(CastMemberID memberID) {
	// release previous widget
	if (_sprite->_cast)
		_sprite->_cast->releaseWidget();

	bool hasChanged = _sprite->_castId != memberID;

	// A video that leaves the channel must not go on changing it
	if (hasChanged && _sprite->_cast && _sprite->_cast->_type == kCastDigitalVideo) {
		DigitalVideoCastMember *old = (DigitalVideoCastMember *)_sprite->_cast;
		if (old->_channel == this)
			old->setChannel(nullptr);
	}

	// Replace the cast member in the sprite. The stored size stays:
	// a stretched sprite keeps it, an unstretched one is drawn at the
	// size of its new member.
	_sprite->setCast(memberID);

	// Duplicate of the special cases in setClean.
	// Maybe it makes sense to force setClean to use setCast instead?
	if (hasChanged && _sprite->_cast) {
		if (_sprite->_cast->_type == kCastDigitalVideo) {
			DigitalVideoCastMember *dv = (DigitalVideoCastMember *)_sprite->_cast;
			if (dv->loadVideoFromCast()) {
				_movieTime = 0;
				dv->setChannel(this);
				dv->startVideo();
			}
		} else if (_sprite->_cast->_type == kCastFilmLoop ||
					_sprite->_cast->_type == kCastMovie) {
			// brand new film loop, reset the frame counter.
			_filmLoopFrame = 1;
		}
	}

	replaceWidget();

	// Based on Director in a Nutshell, page 15
	_sprite->setAutoPuppet(kAPCast, true);
	setNeedsDraw();
}

void Channel::setClean(Sprite *nextSprite, bool partial) {
	if (!nextSprite)
		return;

	CastMemberID previousCastId(0, 0);
	bool replace = isDirty(nextSprite);

	// for dirty situation that we need to replace widget.
	// if cast are modified, then we need to replace it
	// if cast size are changed, and we may need to replace it, because we may having the scaled bitmap castmember
	// other situation, e.g. position changing, we will let channel to handle it. So we don't have to replace widget
	bool dimsChanged = !hasTextCastMember(_sprite) && (_sprite->_width != nextSprite->_width || _sprite->_height != nextSprite->_height);

	// if spriteType is changing, then we may need to re-create the widget since spriteType will guide when we creating widget
	bool spriteTypeChanged = _sprite->_spriteType != nextSprite->_spriteType;

	if (nextSprite) {
		// for the non-puppet QDShape, since we won't use isDirty to check whether the QDShape is changed.
		// so we may always keep the sprite info because we need it to draw QDShape.
		// A sprite with fields set from Lingo (D6+) takes the others from
		// the score, see Sprite::replaceFrom()
		if (_sprite->_puppet || (!nextSprite->isQDShape() && partial)) {
			// Updating scripts, etc. does not require a full re-render
			_sprite->_scriptId = nextSprite->_scriptId;
		} else {
			previousCastId = _sprite->_castId;
			// isDirty() leaves these sprites alone; see what the score
			// changed in the fields Lingo did not set
			bool autoPuppet = _sprite->_autoPuppet != 0;
			Common::Point pos = _sprite->getPosition();
			int16 width = _sprite->_width, height = _sprite->_height;
			InkType ink = _sprite->_ink;
			uint32 foreColor = _sprite->_foreColor, backColor = _sprite->_backColor;
			byte blendAmount = _sprite->_blendAmount;
			replaceSprite(nextSprite);
			if (autoPuppet) {
				bool sizeChanged = width != _sprite->_width || height != _sprite->_height;
				replace |= previousCastId != _sprite->_castId || pos != _sprite->getPosition() ||
					sizeChanged || ink != _sprite->_ink || foreColor != _sprite->_foreColor ||
					backColor != _sprite->_backColor || blendAmount != _sprite->_blendAmount;
				dimsChanged |= sizeChanged && !hasTextCastMember(_sprite);
			}
		}
	}

	// FIXME: organize the logic here.
	// for the dirty puppet sprites, we will always replaceWidget since previousCastId is 0, but we shouldn't replace the widget of there are only position changing
	// e.g. we won't want a puppet editable text sprite changing because that will cause the loss of text.
	if (replace) {
		replaceWidget(previousCastId, dimsChanged || spriteTypeChanged);
	}

	updateTextCast();
	updateGlobalAttr();

	// reset the stop time when we are not playing video
	if (_stopTime && (!_sprite->_cast || (_sprite->_cast && _sprite->_cast->_type != kCastDigitalVideo)))
		_stopTime = 0;

	_widgetDirty = false;
}

void Channel::setStretch(bool enabled) {
	if (enabled == _sprite->_stretch)
		return;

	// Only the flag changes. It decides whether the sprite is drawn at
	// the stored size or at the size of its member.
	keepLingoBbox();
	_sprite->_stretch = enabled;
	setDirty();
	replaceWidget();
	setNeedsDraw();
}

// this is used to for setting and updating text castmember
// e.g. set editable, update dims for auto expanding
void Channel::updateTextCast() {
	if (!_sprite->_cast || _sprite->_cast->_type != kCastText)
		return;

	setEditable(_sprite->getEditable());

	if (_widget) {
		Graphics::MacText *textWidget = (Graphics::MacText *)_widget;
		// if we got auto expand text, then we update dims to sprite
		if (!textWidget->getFixDims() && (_sprite->_width != _widget->_dims.width() || _sprite->_height != _widget->_dims.height())) {
			sizeSpriteFromWidget();
			setDirty();
		}
	}
}

bool Channel::getEditable() {
	if (_sprite->_cast && _sprite->_cast->_type == kCastText) {
		if (_widget && (Graphics::MacText *)_widget->isEditable()) {
			return true;
		}
	}
	return false;
}

void Channel::setEditable(bool editable) {
	if (_sprite->_cast && _sprite->_cast->_type == kCastText) {
		// if the sprite is editable, then we refresh the selEnd and setStart
		if (_widget) {
			((Graphics::MacText *)_widget)->setEditable(editable);
			// we only set the first editable text member in score active
			if (editable) {
				Graphics::MacWidget *activewidget = g_director->_wm->getActiveWidget();
				if (activewidget == nullptr || !activewidget->isEditable())
					g_director->_wm->setActiveWidget(_widget);
			}
		}
	}
}

// we may optimize this by only update those attributes when we are changing them
// but not to pass them to widgets every time
void Channel::updateGlobalAttr() {
	if (!_sprite->_cast)
		return;

	// update text info, including selEnd and selStart
	if (_sprite->_cast->_type == kCastText && _sprite->_editable && _widget)
		((Graphics::MacText *)_widget)->setSelRange(g_director->getCurrentMovie()->_selStart, g_director->getCurrentMovie()->_selEnd);

	// update button info, including checkBoxType
	if ((_sprite->_cast->_type == kCastButton || isButtonSprite(_sprite->_spriteType)) && _widget) {
		((Graphics::MacButton *)_widget)->setCheckBoxType(g_director->getCurrentMovie()->_checkBoxType);
		((Graphics::MacButton *)_widget)->setCheckBoxAccess(g_director->getCurrentMovie()->_checkBoxAccess);
	}
}

void Channel::replaceSprite(Sprite *nextSprite) {
	if (!nextSprite)
		return;

	bool hasWidget = _sprite->_cast && _widget;
	// release the widget, because we may having the new one
	if (_sprite->_cast && !canKeepWidget(_sprite, nextSprite)) {
		_sprite->_cast->releaseWidget();
		hasWidget = false;
	}
	int16 width = _sprite->_width;
	int16 height = _sprite->_height;
	int16 reportedWidth = _sprite->_reportedWidth;
	int16 reportedHeight = _sprite->_reportedHeight;

	if (!(_sprite->_puppet || _sprite->getAutoPuppet(kAPCast)) && (_sprite->_castId != nextSprite->_castId)) {
		// if there's a video in the old sprite that's different, stop it before we continue
		if (_sprite->_cast && _sprite->_cast->_type == kCastDigitalVideo) {
			((DigitalVideoCastMember *)_sprite->_cast)->stopVideo();
			((DigitalVideoCastMember *)_sprite->_cast)->seekMovie(0);
			((DigitalVideoCastMember *)_sprite->_cast)->setChannel(nullptr);
		}
		// if there's a video in the new sprite that's different, start it before we continue
		if (nextSprite->_cast && nextSprite->_cast->_type == kCastDigitalVideo) {
			if (((DigitalVideoCastMember *)nextSprite->_cast)->loadVideoFromCast()) {
				_movieTime = 0;
				_movieRate = 1.0;
				((DigitalVideoCastMember *)nextSprite->_cast)->setChannel(this);
				_startTime = 0;

				_stopTime = ((DigitalVideoCastMember *)nextSprite->_cast)->getMovieTotalTime();
				((DigitalVideoCastMember *)nextSprite->_cast)->rewindVideo();
				((DigitalVideoCastMember *)nextSprite->_cast)->startVideo();
			}
		}
		// if there's a brand new film loop in the new sprite, reset the frame counter
		if (nextSprite->_cast && nextSprite->_cast->_type == kCastFilmLoop) {
			_filmLoopFrame = 1;
		}
	}

	_sprite->replaceFrom(nextSprite);
	if (!_sprite->_autoPuppet)
		_hasLingoBbox = false;

	// Before D6 there is no auto-puppeting of the location, see
	// updateDragPosition().
	if (g_director->getVersion() < 600) {
		if (!_sprite->_moveable)
			_hasDragPosition = false;
		else if (_hasDragPosition)
			_sprite->_startPoint = _dragPosition;
	}

	// TODO: auto expand text size is meaning less for us, not all text
	// since we are using initialRect for the text cast member now, then the sprite size is meaning less for us.
	// thus, we keep the _sprite size here
	if (hasTextCastMember(_sprite) && hasWidget) {
		_sprite->_width = width;
		_sprite->_height = height;
		_sprite->_reportedWidth = reportedWidth;
		_sprite->_reportedHeight = reportedHeight;
	}

	if (g_director->getVersion() >= 600) {
		_startFrame = _sprite->_spriteInfo.startFrame;
		_endFrame = _sprite->_spriteInfo.endFrame;
	}
}

// Before D6 the projector keeps a drag position for every channel that was
// moveable in the previous frame, and puts it over the score location
// (Mac D4: Score_applyDragPositions @ 0x41df8). It is taken from the score
// one frame after the sprite becomes moveable, unless Lingo or a drag set
// it first, and dropped once the sprite stops being moveable. Real D4:
// T_SPRT14. Without it a dragged sprite jumps back whenever the score
// rewrites its location, e.g. on every go("LOOP") in Alice's STONE.DIR.
void Channel::updateDragPosition(Sprite *nextSprite) {
	if (g_director->getVersion() >= 600 || !nextSprite || _sprite->_puppet)
		return;

	// _sprite still holds the previous frame here
	if (!_sprite->_moveable) {
		_hasDragPosition = false;
	} else if (!_hasDragPosition) {
		_hasDragPosition = true;
		_dragPosition = nextSprite->getPosition();
	}
}

void Channel::setDirty() {
	_widgetDirty = true;
}

void Channel::setPosition(int x, int y, bool force) {
	Common::Point newPos(x, y);
	if (_constraint > 0 && _score && _constraint <= _score->_channels.size()) {
		Common::Rect constraintBbox = _score->_channels[_constraint]->getRollOverBbox();
		newPos.x = MIN(constraintBbox.right, MAX(constraintBbox.left, newPos.x));
		newPos.y = MIN(constraintBbox.bottom, MAX(constraintBbox.top, newPos.y));
	}
	_sprite->setPosition(newPos.x, newPos.y);
	if (g_director->getVersion() < 600) {
		// Remember where a moveable sprite was dragged or moved by Lingo,
		// see replaceSprite().
		_hasDragPosition = _sprite->_moveable;
		_dragPosition = newPos;
	}
	// Update the dimensons on the widget
	if (_widget) {
		Common::Rect dims = _widget->getDimensions();
		dims.translate(newPos.x - dims.left, newPos.y - dims.top);
		_widget->setDimensions(dims);
	}
}

// here is the place for deciding whether the widget can be keep or not
// here's the definition, we first need to have widgets to keep, and the cast is not modified(modified means we need to re-create the widget)
// and the castId should be same while castId should not be zero
bool Channel::canKeepWidget(CastMemberID castId) {
	if (_widget && _sprite && _sprite->_cast && !_sprite->_cast->isModified() && castId.member && castId == _sprite->_castId) {
		return true;
	}
	return false;
}

bool Channel::canKeepWidget(Sprite *currentSprite, Sprite *nextSprite) {
	if (_widget && currentSprite && currentSprite->_cast && nextSprite && nextSprite->_cast && !currentSprite->_cast->isModified() && currentSprite->_castId == nextSprite->_castId && currentSprite->_castId.member) {
		return true;
	}
	return false;
}

// currently, when we are setting hilite, we delete the widget and the re-create it
// so we may optimize this if this operation takes much time
void Channel::replaceWidget(CastMemberID previousCastId, bool force) {
	// An embedded movie is composited via getSubChannels(); its own channels
	// must not create widgets, or the shared window would draw them at the
	// embedded movie's native position.
	if (_score && _score->getMovie() && _score->getMovie()->_isEmbedded) {
		if (_widget) {
			delete _widget;
			_widget = nullptr;
		}
		return;
	}

	// A bitmap drawn through a quad is warped from the member at its own
	// size, so moving the corners needs no new widget
	bool quadBitmap = _sprite && _sprite->_hasQuad && _sprite->_cast && _sprite->_cast->_type == kCastBitmap;
	if (!force && quadBitmap && _widget && _widgetCastId == _sprite->_castId && !_sprite->_cast->isModified()) {
		Common::Rect native = _sprite->_cast->getInitialRect();
		Graphics::ManagedSurface *srf = _widget->getSurface();
		if (srf && srf->w == native.width() && srf->h == native.height())
			return;
	}

	// if the castmember is the same, and we are not modifying anything which cannot be handle by channel. Then we don't replace the widget
	if (!force && canKeepWidget(previousCastId)) {
		debug(5, "Channel::replaceWidget(): skip deleting %s", _sprite->_castId.asString().c_str());
		return;
	}

	if (_widget) {
		delete _widget;
		_widget = nullptr;
	}
	_quadValid = false;

	if (_sprite && _sprite->_cast) {
		// use sprite type to guide us how to draw the cast
		// if the type don't match, then we will set it as transparent. i.e. don't create widget
		if (!_sprite->checkSpriteType())
			return;

		if (_sprite->_cast->needsReload()) {
			_sprite->_cast->load();
		}
		// always use the unstretched dims.
		// because only the stretched sprite will have different channel size and sprite size
		// we need the original image to scale the sprite.
		// for the scaled bitmap castmember, it has scaled dims on sprite size, so we don't have to worry about it.
		Common::Rect bbox(getBbox(true));
		if (quadBitmap) {
			Common::Rect native = _sprite->_cast->getInitialRect();
			if (native.width() > 0 && native.height() > 0)
				bbox = Common::Rect(bbox.left, bbox.top, bbox.left + native.width(), bbox.top + native.height());
		}
		_sprite->_cast->setModified(false);
		_widgetCastId = _sprite->_castId;

		_widget = _sprite->_cast->createWidget(bbox, this, _sprite->getMemberType());
		if (_widget) {
			_widget->_priority = _priority;
			_widget->draw();

			// Only auto-expanding text (and buttons) size the sprite from the widget:
			// fixed text would re-add its shadow chrome each rebuild and creep larger.
			// A D2/D3 text member on a button sprite is drawn as a button too.
			if (_sprite->_cast->_type == kCastButton || isButtonSprite(_sprite->_spriteType) ||
					(_sprite->_cast->_type == kCastText && !((Graphics::MacText *)_widget)->getFixDims())) {
				sizeSpriteFromWidget();
			}
		}
	}
}

// Draw and hit-test the sprite at its widget's size. The projector still
// reports the size from the score (or Lingo) as the sprite's width and
// height, although its left, top, right and bottom follow the drawn rect.
void Channel::sizeSpriteFromWidget() {
	if (_sprite->_reportedWidth < 0)
		_sprite->_reportedWidth = _sprite->_width;
	if (_sprite->_reportedHeight < 0)
		_sprite->_reportedHeight = _sprite->_height;
	_sprite->_width = _widget->_dims.width();
	_sprite->_height = _widget->_dims.height();
}

bool Channel::updateWidget() {
	if (_sprite->_cast && (_sprite->_cast->_type == kCastDigitalVideo) && _sprite->_cast->isModified()) {
		replaceWidget();
		return true;
	}

	if (_sprite->_cast && (_sprite->_cast->_type == kCastText) && _sprite->_editable) {
		if (_widget && _widget->_active) {
			// small hack: update the castID/scriptID used for keyDown events.
			// typing happens at the WM level, and this is one of the few places we can intercept it
			Movie *movie = g_director->getCurrentMovie();
			movie->_currentKeyDownCastID = _sprite->_castId;
			movie->_currentKeyDownSpriteScriptID = _sprite->_scriptId;
			movie->_currentKeyDownSpriteImmediate = _sprite->_immediate;
		}
	}

	if (_widget && _widget->needsRedraw()) {
		if (_sprite->_cast) {
			_sprite->_cast->updateFromWidget(_widget, _sprite->_editable);

		}
		_widget->draw();
		return true;
	}
	return false;
}

bool Channel::isTrail() {
	return _sprite->_trails;
}

// Whether a full "limit to field size" field in this channel drops a key
// that the keyDown scripts passed on
bool Channel::dropsTypedKey(const Common::KeyState &kbd) {
	if (!_sprite->_cast || _sprite->_cast->_type != kCastText || isButtonSprite(_sprite->_spriteType))
		return false;

	return ((TextCastMember *)_sprite->_cast)->dropsTypedKey((Graphics::MacText *)_widget, kbd);
}

// The text widget under a mouse query, or null (with a warning if the
// sprite is text but its widget is missing)
Graphics::MacText *Channel::mouseTextWidget(const char *query) {
	if (!isTextSprite())
		return nullptr;

	if (!_widget) {
		warning("Channel::%s: the text sprite has no widget", query);
		return nullptr;
	}

	return (Graphics::MacText *)_widget;
}

int Channel::getMouseChar(int x, int y) {
	Graphics::MacText *text = mouseTextWidget("getMouseChar");
	return text ? text->getMouseChar(x, y) : -1;
}

int Channel::getMouseWord(int x, int y) {
	Graphics::MacText *text = mouseTextWidget("getMouseWord");
	return text ? text->getMouseWord(x, y) : -1;
}

int Channel::getMouseItem(int x, int y) {
	Graphics::MacText *text = mouseTextWidget("getMouseItem");
	return text ? text->getMouseItem(x, y) : -1;
}

int Channel::getMouseLine(int x, int y) {
	Graphics::MacText *text = mouseTextWidget("getMouseLine");
	return text ? text->getMouseLine(x, y) : -1;
}

bool Channel::hasSubChannels() {
	if ((_sprite->_cast) && (_sprite->_cast->_type == kCastFilmLoop || _sprite->_cast->_type == kCastMovie)) {
		return true;
	}
	return false;
}

Common::Array<Channel> *Channel::getSubChannels() {
	if (_sprite->_cast) {
		Common::Rect bbox = getBbox();
		if (_sprite->_cast->_type == kCastFilmLoop)
			return ((FilmLoopCastMember *)_sprite->_cast)->getSubChannels(bbox, _filmLoopFrame);
		else if (_sprite->_cast->_type == kCastMovie)
			return ((MovieCastMember *)_sprite->_cast)->getSubChannels(bbox, _filmLoopFrame);
	}
	warning("Channel doesn't have any sub-channels");
	return nullptr;
}

CastMemberID Channel::getSubChannelSound1() {
	if (_sprite->_cast) {
		if (_sprite->_cast->_type == kCastFilmLoop)
			return ((FilmLoopCastMember *)_sprite->_cast)->getSubChannelSound1(_filmLoopFrame);
		else if (_sprite->_cast->_type == kCastMovie)
			return ((MovieCastMember *)_sprite->_cast)->getSubChannelSound2(_filmLoopFrame);
	}
	warning("Channel doesn't have any sub-channels");
	return CastMemberID();
}

CastMemberID Channel::getSubChannelSound2() {
	if (_sprite->_cast) {
		if (_sprite->_cast->_type == kCastFilmLoop)
			return ((FilmLoopCastMember *)_sprite->_cast)->getSubChannelSound1(_filmLoopFrame);
		else if (_sprite->_cast->_type == kCastMovie)
			return ((MovieCastMember *)_sprite->_cast)->getSubChannelSound2(_filmLoopFrame);
	}
	warning("Channel doesn't have any sub-channels");
	return CastMemberID();
}

Common::String Channel::formatInfo() {
	return Common::String::format("[sprite: %s], visible: %d, constraint: %d, movieRate: %f, movieTime: %d (%f), filmLoopFrame: %d, startTime: %d, stopTime: %d",
		_sprite->formatInfo().c_str(), _visible,
		_constraint, _movieRate, _movieTime, (float)(_movieTime/60.0f), _filmLoopFrame, _startTime, _stopTime);
}

} // End of namespace Director
