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

/*************************************
 *
 * USED IN:
 * The Apartment
 *
 *************************************/

/*
 * --PopMenu, Tool, 1.0, 4/1/90
 * --© 1989, 1990 MacroMind, Inc.
 * -- by Jeff Tanner
 * ------------------------------------------------
 * ------------------------------------------------
 * --  IMPORTANT NOTE:  In Lingo, build PopMenus after using
 * --    the command installMenu.  On the Mac, Pop-up menus are
 * --    extensions of the menu bar.  When the command installMenu
 * --    is called, this will remove all menus currently on
 * --    the menubar (including pop-up menus) and only install those
 * --    menus defined in the text window referenced by the
 * --    castNum parameter.  If installMenu is used after
 * --    creating PopMenus, these PopMenus must be disposed of
 * --    and then recreated.
 * ------------------------------------------------
 * --  MENULIST NOTE: In Lingo, there are several ways
 * --      to build the menu list for a PopUp menu.
 * --
 * --      - a menulist can be a continuous string
 * --          with items separated by semicolons.
 * --          example: "item1;item2;item3"
 * --
 * --      - a menulist can be a set of strings, each
 * --          representing an item, separated by
 * --          &return& and ending with &return
 * --          example: "item1"&return&"item2"&return
 * --
 * --      - a menulist could come from a cast member with
 * --          each menu item separated by a carrage return.
 * --          example: set menulist to the text of cast A31
 * ------------------------------------------------
 * --  MENUITEM NOTE: Use only alphanumeric characters,
 * --      0 - 9 and A - Z.  Avoid dashes.  Indicate
 * --      style by "item1;item2<B;item3<U"
 * --      To change an item's style, add < followed by a
 * --      character at the end of the menu item text:
 * --          <B  Bold
 * --          <U  Underlined
 * --          <I  Italics
 * --          <S  Shadowed
 * --          <O  Outlined
 * ------------------------------------------------
 * ------------------------------------------------
 * --=METHODS=--
 * XSI     mNew, menuList, menuID --Creates a new instance of the XObject.
 * --   menuItemList - "item1;item2<B;item3<U"
 * --      Separate all items with semicolons.
 * --      Maximum character length of menulist is 256.
 * --   menuID - to avoid resourse conflict with Director,
 * --      use a menu ID between 100 and 1000.
 * --
 * X   mDispose --Disposes of the XObject instance.
 * S   mName --Returns the name of the XObject.
 * ------------------------------------------------
 * ------------------------------------------------
 * XS  mAppendMenu, menuList --Adds items to menuList.
 * XI  mDisableItem, itemNum --Disables item in pop up menu.
 * XI  mEnableItem, itemNum --Enables item in pop up menu.
 * SI  mGetItem, itemNum --Returns item in pop up menu.
 * I   mGetMenuID --Returns the assigned Menu ID number.
 * ------------------------------------------------
 * ------------------------------------------------
 * IIII    mPopNum, Left, Top, itemNum --Returns selected item's number.
 * SIII    mPopText, Left, Top, itemNum --Returns selected item's text.
 * ------------------------------------------------
 * ------------------------------------------------
 * XIS mSetItem, itemNum, newItemText --Sets changes to an item in pop up menu
 * XI  mSetItemMark, markNum --Sets marker for pop up menu: default is check.
 * XI  mSmart, TrueOrFalse --Remembers last selection if itemNum is 0
 * XII mSetItemIcon, itemNum, iconID --Attaches an icon to menu item, id# 257 - 511
 * ------------------------------------------------
 * ------------------------------------------------
 */

#include "director/director.h"
#include "director/archive.h"
#include "director/window.h"

#include "director/lingo/lingo.h"
#include "director/lingo/lingo-object.h"
#include "director/lingo/lingo-utils.h"
#include "director/lingo/xlibs/p/popupmenuxobj.h"

#include "graphics/managed_surface.h"
#include "graphics/macgui/macpopupmenu.h"

namespace Director {

const char *const PopUpMenuXObj::xlibName = "PopMenu";
const XlibFileDesc PopUpMenuXObj::fileNames[] = {
	{ "PopMenu",			nullptr },
	{ "PopUp Menu XObj",	nullptr },
	{ nullptr,				nullptr },
};

static const MethodProto xlibMethods[] = {
	{ "new",				PopUpMenuXObj::m_new,			 2, 2,	200 },	// D2
	{ "AppendMenu",			PopUpMenuXObj::m_appendMenu,	 1, 1,	200 },	// D2
	{ "DisableItem",		PopUpMenuXObj::m_disableItem,	 1, 1,	200 },	// D2
	{ "EnableItem",			PopUpMenuXObj::m_enableItem,	 1, 1,	200 },	// D2
	{ "GetItem",			PopUpMenuXObj::m_getItem,		 1, 1,  200 },	// D2
	{ "GetMenuID",			PopUpMenuXObj::m_getMenuID,		 0, 0,  200 },	// D2
	{ "PopNum",				PopUpMenuXObj::m_popNum,		 3, 3,  200 },	// D2
	{ "PopText",			PopUpMenuXObj::m_popText,		 3, 3,  200 },	// D2
	{ "SetItem",			PopUpMenuXObj::m_setItem,		 2, 2,  200 },	// D2
	{ "SetItemMark",		PopUpMenuXObj::m_setItemMark,	 1, 1,  200 },	// D2
	{ "Smart",				PopUpMenuXObj::m_smart,			 1, 1,  200 },	// D2
	{ "SetItemIcon",		PopUpMenuXObj::m_setItemIcon,	 2, 2,  200 },	// D2
	{ nullptr, nullptr, 0, 0, 0 }
};

void PopUpMenuXObj::open(ObjectType type, const Common::Path &path) {
	if (type == kXObj) {
		PopUpMenuXObject::initMethods(xlibMethods);
		PopUpMenuXObject *xobj = new PopUpMenuXObject(kXObj);
		g_lingo->exposeXObject(xlibName, xobj);
	}
}

void PopUpMenuXObj::close(ObjectType type) {
	if (type == kXObj) {
		PopUpMenuXObject::cleanupMethods();
		g_lingo->_globalvars[xlibName] = Datum();
	}
}


PopUpMenuXObject::PopUpMenuXObject(ObjectType ObjectType) : Object<PopUpMenuXObject>("PopMenu") {
	_objType = ObjectType;
	_menuId = 0;
	_mark = 0x12;
	_smart = false;
	_lastItem = 0;
}

void PopUpMenuXObj::m_new(int nargs) {
	PopUpMenuXObject *me = static_cast<PopUpMenuXObject *>(g_lingo->_state->me.u.obj);

	int menuId = g_lingo->pop().asInt();
	Common::String menuList = g_lingo->pop().asString();

	new Graphics::MacPopUp(menuId, g_director->_wm->getScreenBounds(), g_director->_wm, menuList.c_str());
	me->_menuId = menuId;
	me->_mark = 0x12;	// checkMark
	me->_smart = false;
	me->_lastItem = 0;

	g_lingo->push(g_lingo->_state->me);
}

static Graphics::MacPopUp *getPopUp(PopUpMenuXObject *me) {
	return static_cast<Graphics::MacPopUp *>(g_director->_wm->getMenu(me->_menuId));
}

// The XObject's DoPopMenu: mark the item to open at, call PopUpMenuSelect,
// then take the mark off again. Returns the chosen item, 0 if none.
static int doPopMenu(PopUpMenuXObject *me, int left, int top, int itemNum) {
	Graphics::MacPopUp *menu = getPopUp(me);
	if (!menu)
		return 0;
	if (itemNum < 0) {
		warning("PopUpMenuXObj: negative item number %d", itemNum);
		return 0;
	}

	int popItem = 0;
	if (itemNum > 0)
		popItem = itemNum;
	else if (me->_smart)
		popItem = me->_lastItem;
	if (popItem > 0)
		menu->setItemMark(popItem, me->_mark);

	// Convert window coordinates to screen coordinates
	Common::Rect windowRect = g_director->getCurrentWindow()->getMacWindow()->getInnerDimensions();
	int screenTop = top + windowRect.top - 1;
	int screenLeft = left + windowRect.left - 1;

	int selected = menu->drawAndSelectMenu(screenLeft, screenTop, popItem);

	if (popItem > 0)
		menu->setItemMark(popItem, 0);

	// A cancelled menu forgets the last item too
	if (me->_smart && itemNum == 0)
		me->_lastItem = selected;

	return selected;
}

void PopUpMenuXObj::m_popNum(int nargs) {
	PopUpMenuXObject *me = static_cast<PopUpMenuXObject *>(g_lingo->_state->me.u.obj);

	int itemNum = g_lingo->pop().asInt();
	int top = g_lingo->pop().asInt();
	int left = g_lingo->pop().asInt();

	g_lingo->push(Datum(doPopMenu(me, left, top, itemNum)));
}

void PopUpMenuXObj::m_popText(int nargs) {
	PopUpMenuXObject *me = static_cast<PopUpMenuXObject *>(g_lingo->_state->me.u.obj);

	int itemNum = g_lingo->pop().asInt();
	int top = g_lingo->pop().asInt();
	int left = g_lingo->pop().asInt();

	int selected = doPopMenu(me, left, top, itemNum);
	Graphics::MacPopUp *menu = getPopUp(me);
	Common::String selectedText = selected > 0 && menu ? menu->getItemText(selected) : "";

	g_lingo->push(Datum(selectedText));
}

void PopUpMenuXObj::m_smart(int nargs) {
	PopUpMenuXObject *me = static_cast<PopUpMenuXObject *>(g_lingo->_state->me.u.obj);
	me->_smart = g_lingo->pop().asInt() != 0;
}

void PopUpMenuXObj::m_setItemMark(int nargs) {
	PopUpMenuXObject *me = static_cast<PopUpMenuXObject *>(g_lingo->_state->me.u.obj);
	me->_mark = g_lingo->pop().asInt() & 0xff;
}

// ICON resources are 32x32 1-bit images, looked up like GetResource does
static Graphics::ManagedSurface *loadIcon(uint16 id) {
	const uint32 tag = MKTAG('I', 'C', 'O', 'N');
	Common::SeekableReadStreamEndian *stream = nullptr;

	for (auto &it : g_director->_allOpenResFiles) {
		Archive *archive = g_director->_allSeenResFiles[it].get();
		if (archive && archive->hasResource(tag, id)) {
			stream = archive->getResource(tag, id);
			break;
		}
	}
	if (!stream) {
		Archive *archive = g_director->getMainResourceFork();
		if (archive && archive->hasResource(tag, id))
			stream = archive->getResource(tag, id);
	}
	if (!stream)
		return nullptr;

	if (stream->size() < 128) {
		delete stream;
		return nullptr;
	}

	Graphics::ManagedSurface *icon = new Graphics::ManagedSurface(32, 32, Graphics::PixelFormat::createFormatCLUT8());
	for (int y = 0; y < 32; y++) {
		uint32 row = stream->readUint32BE();
		byte *dst = (byte *)icon->getBasePtr(0, y);
		for (int x = 0; x < 32; x++)
			dst[x] = (row >> (31 - x)) & 1;
	}
	delete stream;
	return icon;
}

void PopUpMenuXObj::m_setItemIcon(int nargs) {
	PopUpMenuXObject *me = static_cast<PopUpMenuXObject *>(g_lingo->_state->me.u.obj);

	int iconID = g_lingo->pop().asInt();
	int itemNum = g_lingo->pop().asInt();

	// The XObject takes IDs 256-512 and hands SetItemIcon (iconID - 256),
	// so the Menu Manager shows ICON iconID
	if (iconID < 0x100 || iconID > 0x200) {
		warning("PopUpMenuXObj::m_setItemIcon(): icon ID %d out of range", iconID);
		return;
	}

	Graphics::MacPopUp *menu = getPopUp(me);
	if (!menu)
		return;

	Graphics::ManagedSurface *icon = nullptr;
	if (iconID != 0x100) {
		icon = loadIcon(iconID);
		if (!icon)
			warning("PopUpMenuXObj::m_setItemIcon(): ICON %d not found", iconID);
	}
	menu->setItemIcon(itemNum, icon);
}

XOBJSTUBNR(PopUpMenuXObj::m_appendMenu)
XOBJSTUBNR(PopUpMenuXObj::m_disableItem)
XOBJSTUBNR(PopUpMenuXObj::m_enableItem)
XOBJSTUB(PopUpMenuXObj::m_getItem, "")
XOBJSTUB(PopUpMenuXObj::m_getMenuID, 0)
XOBJSTUBNR(PopUpMenuXObj::m_setItem)

} // End of namespace Director
