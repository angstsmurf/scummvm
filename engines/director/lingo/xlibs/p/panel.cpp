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

#include "common/system.h"

#include "graphics/font.h"
#include "graphics/macgui/macfontmanager.h"
#include "graphics/macgui/macwindow.h"
#include "graphics/macgui/macwindowmanager.h"

#include "director/director.h"
#include "director/movie.h"
#include "director/castmember/bitmap.h"
#include "director/lingo/lingo.h"
#include "director/lingo/lingo-object.h"
#include "director/lingo/lingo-utils.h"
#include "director/lingo/xlibs/p/panel.h"
#include "director/lingo/xlibs/w/window.h"

/**************************************************
 *
 * USED IN:
 * Standard Macromedia Director XObject
 *
 **************************************************/

/*
--Panel, Tool, 1.0
--© 1989, 1990 MacroMind, Inc.
--by John Thompson and Al McNeil
------------------------------------------------------------------
------------------------------------------------------------------
--=METHODS=--
IISIO	mNew, kindSymbol, titleString, myNameSymbol, windowObj	--Creates a new instance.
--kindSymbol is one of: #pushButton, #checkBox, #radioButton, #staticText, #castBitmap
X 	mDispose	--Disposes of the instance.
S 	mName	--Returns the XObject's name.
X	mShow	--Shows instance.
X	mHide	--Hides instance.
X	mUpdate	--Updates the instance.
XII	mMove, hpos, vpos	---Moves to h, v.
XII	mSize, width, height	--Changes size to the specified width, height.
XII	mDrag, startH, startV
XI	mSetValue, value	--Sets panel value.
I	mGetValue	--Returns panel value.
XI	mSetHilite, flag	--Sets hilite state.
I	mGetHilite	--Returns panel hilite state.
XS	mSetTitle, string	--Sets title for control.
S	mGetTitle	--Returns the control's title string.
XII	mMouseDown, loch, locv	--Sends a mousedown at given location in panel.
/X	mBounds, rectPtr	--Returns the bounding rect
 */

/*
 * Reversed from XCOD 2011 of the Director 3.1 installer.
 *
 * Push buttons, check boxes and radio buttons are Toolbox controls
 * (NewControl procIDs 0, 1 and 2, initial rect 0,0,80,20, value 0..1).
 * Static text is drawn with TextBox (initial rect 0,0,100,20) and a cast
 * bitmap is CopyBits of the whole bitmap stretched to the panel rect; its
 * rect starts empty. Every panel starts hidden and adds itself to its
 * window with mAddPanel. Controls never change value on their own: the
 * handler has to call mSetValue.
 */

namespace Director {

const char *const PanelXObj::xlibName = "Panel";
const XlibFileDesc PanelXObj::fileNames[] = {
	{ "Panel",	nullptr },
	{ nullptr,	nullptr },
};

static const MethodProto xlibMethods[] = {
	{ "new",				PanelXObj::m_new,		 4, 4,	200 },
	{ "dispose",				PanelXObj::m_dispose,		 0, 0,	200 },
	{ "name",				PanelXObj::m_name,		 0, 0,	200 },
	{ "show",				PanelXObj::m_show,		 0, 0,	200 },
	{ "hide",				PanelXObj::m_hide,		 0, 0,	200 },
	{ "update",				PanelXObj::m_update,		 0, 0,	200 },
	{ "move",				PanelXObj::m_move,		 2, 2,	200 },
	{ "size",				PanelXObj::m_size,		 2, 2,	200 },
	{ "drag",				PanelXObj::m_drag,		 2, 2,	200 },
	{ "setValue",				PanelXObj::m_setValue,		 1, 1,	200 },
	{ "getValue",				PanelXObj::m_getValue,		 0, 0,	200 },
	{ "setHilite",				PanelXObj::m_setHilite,		 1, 1,	200 },
	{ "getHilite",				PanelXObj::m_getHilite,		 0, 0,	200 },
	{ "setTitle",				PanelXObj::m_setTitle,		 1, 1,	200 },
	{ "getTitle",				PanelXObj::m_getTitle,		 0, 0,	200 },
	{ "mouseDown",				PanelXObj::m_mouseDown,		 2, 2,	200 },
	{ "bounds",				PanelXObj::m_bounds,		 0, 0,	200 },
	{ nullptr, nullptr, 0, 0, 0 }
};

PanelXObject::PanelXObject(ObjectType ObjectType) :Object<PanelXObject>("Panel") {
	_objType = ObjectType;
}

static const Graphics::Font *systemFont() {
	return g_director->_wm->_fontMan->getFont(Graphics::MacFont(Graphics::kMacFontSystem, 12));
}

void PanelXObject::changed() {
	if (_window)
		_window->redraw();
}

void PanelXObject::draw(Graphics::ManagedSurface *surface) {
	if (!_visible || _rect.isEmpty())
		return;

	Graphics::MacWindowManager *wm = g_director->_wm;
	uint32 black = wm->_colorBlack;
	uint32 white = wm->_colorWhite;
	const Graphics::Font *font = systemFont();
	Common::Rect r = _rect;
	bool hilited = _tracking || _hilite == 1;

	switch (_kind) {
	case kPanelPushButton: {
		// Standard button CDEF: round rect, title centered
		surface->drawRoundRect(r, 8, white, true);
		surface->drawRoundRect(r, 8, black, false);
		int textW = font->getStringWidth(_title);
		int x = r.left + (r.width() - textW) / 2;
		int y = r.top + (r.height() - font->getFontHeight()) / 2;
		font->drawString(surface, _title, x, y, r.right - x, black);
		if (hilited) {
			Common::Rect in(r.left, r.top, r.right, r.bottom);
			Graphics::ManagedSurface mask(r.width(), r.height(), surface->format);
			mask.clear(white);
			mask.drawRoundRect(Common::Rect(r.width(), r.height()), 8, black, true);
			for (int py = 0; py < r.height(); py++) {
				for (int px = 0; px < r.width(); px++) {
					if (mask.getPixel(px, py) != black)
						continue;
					if (!surface->getBounds().contains(r.left + px, r.top + py))
						continue;
					uint32 c = surface->getPixel(r.left + px, r.top + py);
					surface->setPixel(r.left + px, r.top + py, c == black ? white : black);
				}
			}
		}
		break;
	}
	case kPanelCheckBox:
	case kPanelRadioButton: {
		surface->fillRect(r, white);
		Common::Rect box(r.left + 2, r.top + (r.height() - 12) / 2, r.left + 14, r.top + (r.height() - 12) / 2 + 12);
		if (_kind == kPanelCheckBox) {
			surface->frameRect(box, black);
			if (hilited)
				surface->frameRect(Common::Rect(box.left + 1, box.top + 1, box.right - 1, box.bottom - 1), black);
			if (_value) {
				surface->drawLine(box.left, box.top, box.right - 1, box.bottom - 1, black);
				surface->drawLine(box.left, box.bottom - 1, box.right - 1, box.top, black);
			}
		} else {
			surface->drawEllipse(box.left, box.top, box.right - 1, box.bottom - 1, black, false);
			if (hilited)
				surface->drawEllipse(box.left + 1, box.top + 1, box.right - 2, box.bottom - 2, black, false);
			if (_value)
				surface->drawEllipse(box.left + 3, box.top + 3, box.right - 4, box.bottom - 4, black, true);
		}
		int y = r.top + (r.height() - font->getFontHeight()) / 2;
		font->drawString(surface, _title, r.left + 17, y, MAX(0, (int)r.right - (r.left + 17)), black);
		break;
	}
	case kPanelStaticText: {
		// TextBox: erase, then wrap the text left-justified
		surface->fillRect(r, white);
		Common::Array<Common::String> lines;
		font->wordWrapText(_title, r.width(), lines);
		int y = r.top;
		for (uint i = 0; i < lines.size() && y < r.bottom; i++) {
			Graphics::ManagedSurface sub(*surface, Common::Rect(r.left, y, r.right, MIN<int>(r.bottom, y + font->getFontHeight())));
			font->drawString(&sub, lines[i], 0, 0, r.width(), black);
			y += font->getFontHeight();
		}
		break;
	}
	case kPanelCastBitmap: {
		Movie *movie = g_director->getCurrentMovie();
		if (!movie || _cast.type == VOID)
			break;
		CastMember *member = movie->getCastMember(_cast.asMemberID(kCastBitmap));
		if (!member || member->_type != kCastBitmap)
			break;
		Common::Rect bbox(0, 0, r.width(), r.height());
		Graphics::MacWidget *widget = member->createWidget(bbox, nullptr, kBitmapSprite);
		if (!widget)
			break;
		surface->blitFrom(*widget->getSurface(), Common::Point(r.left, r.top));
		delete widget;
		break;
	}
	default:
		break;
	}
}

void PanelXObj::open(ObjectType type, const Common::Path &path) {
	if (type == kXObj) {
		PanelXObject::initMethods(xlibMethods);
		PanelXObject *xobj = new PanelXObject(kXObj);
		g_lingo->exposeXObject(xlibName, xobj);
	}
}

void PanelXObj::close(ObjectType type) {
	if (type == kXObj) {
		PanelXObject::cleanupMethods();
		g_lingo->_globalvars[xlibName] = Datum();
	}
}

#define ME PanelXObject *me = static_cast<PanelXObject *>(g_lingo->_state->me.u.obj)

void PanelXObj::m_new(int nargs) {
	ME;
	Datum window = g_lingo->pop();
	Datum name = g_lingo->pop();
	Datum title = g_lingo->pop();
	Datum kind = g_lingo->pop();

	Common::String k = kind.asString();
	if (k.equalsIgnoreCase("checkBox"))
		me->_kind = kPanelCheckBox;
	else if (k.equalsIgnoreCase("radioButton"))
		me->_kind = kPanelRadioButton;
	else if (k.equalsIgnoreCase("staticText"))
		me->_kind = kPanelStaticText;
	else if (k.equalsIgnoreCase("castBitmap"))
		me->_kind = kPanelCastBitmap;
	else
		me->_kind = kPanelPushButton;

	me->_title = title.asString();
	me->_panelName = name.asString();
	if (me->_kind == kPanelStaticText)
		me->_rect = Common::Rect(0, 0, 100, 20);
	else if (me->_kind != kPanelCastBitmap)
		me->_rect = Common::Rect(0, 0, 80, 20);

	Datum result = g_lingo->_state->me;
	if (window.type == OBJECT && window.u.obj->getName() == "Window" && !window.u.obj->isDisposed()) {
		WindowXObject *win = static_cast<WindowXObject *>(window.u.obj);
		me->_window = win;
		win->addPanel(me->_panelName, me);
	} else {
		warning("PanelXObj::m_new: %s is not a window", window.asString(true).c_str());
	}
	g_lingo->push(result);
}

void PanelXObj::m_dispose(int nargs) {
	ME;
	// DisposeControl: the panel stays in the window's table
	me->_visible = false;
	me->changed();
	me->dispose();
}

void PanelXObj::m_name(int nargs) {
	g_lingo->push(Datum("Panel"));
}

void PanelXObj::m_show(int nargs) {
	ME;
	me->_visible = true;
	me->changed();
}

void PanelXObj::m_hide(int nargs) {
	ME;
	me->_visible = false;
	me->changed();
}

void PanelXObj::m_update(int nargs) {
	ME;
	// Only the cast bitmap redraws; for the others this does nothing
	if (me->_kind == kPanelCastBitmap)
		me->changed();
}

void PanelXObj::m_move(int nargs) {
	ME;
	int v = g_lingo->pop().asInt();
	int h = g_lingo->pop().asInt();
	me->_rect.moveTo(h, v);
	me->changed();
}

void PanelXObj::m_size(int nargs) {
	ME;
	int height = g_lingo->pop().asInt();
	int width = g_lingo->pop().asInt();
	me->_rect.setWidth(width);
	me->_rect.setHeight(height);
	me->changed();
}

XOBJSTUBNR(PanelXObj::m_drag)

void PanelXObj::m_setValue(int nargs) {
	ME;
	Datum value = g_lingo->pop();
	switch (me->_kind) {
	case kPanelCastBitmap:
		// Stored only; drawn at the next mUpdate or window update
		me->_cast = value;
		break;
	case kPanelStaticText:
		break;
	default:
		// SetCtlValue clamps to min..max and redraws the control
		me->_value = CLIP(value.asInt(), 0, 1);
		me->changed();
		break;
	}
}

void PanelXObj::m_getValue(int nargs) {
	ME;
	if (me->_kind == kPanelCastBitmap)
		g_lingo->push(me->_cast.type == VOID ? Datum(0) : me->_cast);
	else
		g_lingo->push(Datum(me->_value));
}

void PanelXObj::m_setHilite(int nargs) {
	ME;
	int hilite = g_lingo->pop().asInt();
	if (me->_kind == kPanelCastBitmap)
		return;
	me->_hilite = hilite;
	if (me->_kind != kPanelStaticText)
		me->changed();
}

void PanelXObj::m_getHilite(int nargs) {
	ME;
	g_lingo->push(Datum(me->_kind == kPanelCastBitmap ? 0 : me->_hilite));
}

void PanelXObj::m_setTitle(int nargs) {
	ME;
	Datum title = g_lingo->pop();
	// Only static text takes a new title
	if (me->_kind == kPanelStaticText) {
		me->_title = title.asString();
		me->changed();
	}
}

void PanelXObj::m_getTitle(int nargs) {
	ME;
	if (me->_kind == kPanelStaticText)
		g_lingo->push(Datum(me->_title));
	else if (me->_kind == kPanelCastBitmap)
		g_lingo->push(Datum(0));
	else
		g_lingo->push(Datum(""));
}

void PanelXObj::m_mouseDown(int nargs) {
	ME;
	g_lingo->pop();
	g_lingo->pop();
	// HiliteControl(1) until the button is released
	if (me->_kind >= 0 && g_director->_wm->_mouseDown) {
		me->_tracking = true;
		me->changed();
	}
}

XOBJSTUBNR(PanelXObj::m_bounds)

}
