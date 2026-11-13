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

#include "graphics/macgui/macpopupmenu.h"
#include "graphics/macgui/macwindowmanager.h"

namespace Graphics {

MacPopUp::MacPopUp(int id, const Common::Rect &bounds, MacWindowManager *wm, const char *string) : MacMenu(id, bounds, wm) {
	_menuItemId = addMenuItem(nullptr, "");
	createSubMenuFromString(0, string, 0);
	wm->addMenu(id, this);
	_menuId = id;
}

bool MacPopUp::draw(ManagedSurface *g, bool forceRedraw) {

	if (!_isVisible)
		return false;

	if (_dimensionsDirty)
		calcSubMenuBounds(_items[0]->submenu, _mouseX, _mouseY + _offsetY);

	if (!_contentIsDirty && !forceRedraw)
		return false;
	_contentIsDirty = false;

	_composeSurface->clear(_wm->_colorGreen);
	renderSubmenu(_items[0]->submenu, false);

	if (g)
		g->transBlitFrom(*_composeSurface, _wm->_colorGreen);

	if (!(_wm->_mode & kWMModeNoSystemRedraw) && !(_wm->_mode & kWMModalMenuMode) && g)
		_wm->copyRectToScreen(g->getPixels(), g->pitch, 0, 0, g->w, g->h);

	return true;
}

void MacPopUp::closeMenu() {
	// Special handling of popup closing (for example when displaying closing animation)
	int activeSubItem = getLastSelectedSubmenuItem(); // Find selected item

	if (activeSubItem != -1) {
		// Do the blinking animation
		for (int i = 0; i < kNumBlinks; i++) {
			_items[0]->submenu->highlight = -1; // No selection
			draw(_wm->_screen, true);
			g_system->updateScreen();
			g_system->delayMillis(kBlinkDelay);

			_items[0]->submenu->highlight = activeSubItem; // Selection
			draw(_wm->_screen, true);
			g_system->updateScreen();
			g_system->delayMillis(kBlinkDelay);
		}
	}

	_isModal = false;

	// Close now
	MacMenu::closeMenu();
}

// Like PopUpMenuSelect: item (1-based) is placed at y, so it opens under
// the mouse, and is highlighted. Returns the chosen item, or 0 if none.
uint32 MacPopUp::drawAndSelectMenu(int x, int y, int item) {
	_mouseX = x;
	_mouseY = y;

	// If menu is not active, then activate it!
	if (!_active)
		_wm->activateMenu();
	setActive(true);

	_contentIsDirty = true; // Set true to force refresh menu open changes
	_dimensionsDirty = true;

	_isModal = true;

	MacMenuSubMenu *submenu = _items[0]->submenu;

	// Push our submenu to stack
	_menustack.clear();
	_menustack.push_back(submenu);

	int numItems = submenu->items.size();
	int itemHeight = getItemHeight(submenu);
	int index = CLIP(item, 1, MAX(numItems, 1)) - 1;

	submenu->visStart = 0;
	submenu->visEnd = 0;
	submenu->scroll = 0;
	_offsetY = -index * itemHeight;

	// Items that would be above the screen are scrolled out of view
	while (_offsetY + _mouseY < 0 && submenu->visStart < index) {
		_offsetY += itemHeight;
		submenu->visStart++;
	}
	if (_offsetY + _mouseY < 0)
		_offsetY = -_mouseY;

	int itemsLeft = numItems - submenu->visStart;
	int spaceLeft = _composeSurface->h - (_mouseY + _offsetY);
	submenu->visEnd = MAX(0, itemsLeft - spaceLeft / itemHeight);

	// The submenu is the active one, so leaving it clears the selection
	_activeItem = 0;
	if (item > 0 && item <= numItems) {
		_activeSubItem = index;
		submenu->highlight = index;
	}

	clearLastSelectedSubmenuItem();

	// Display menu and update according to events
	this->draw(_wm->_screen);
	eventLoop();

	int activeSubItem = getLastSelectedSubmenuItem();
	if (activeSubItem == -1 || activeSubItem >= numItems || !submenu->items[activeSubItem]->enabled)
		return 0;

	// Return one indexed item!
	return activeSubItem + 1;
}

void MacPopUp::setItemMark(int item, int markChar) {
	MacMenuSubMenu *submenu = _items[0]->submenu;
	if (!submenu || item < 1 || item > (int)submenu->items.size())
		return;

	MacMenuItem *menuItem = submenu->items[item - 1];
	menuItem->checked = markChar != 0;
	menuItem->checkSymbol = markChar == 0x12 ? 0 : markChar;
	_contentIsDirty = true;
}

void MacPopUp::setItemIcon(int item, ManagedSurface *icon) {
	MacMenu::setItemIcon(_items[0]->submenu, item - 1, icon);
}

Common::String MacPopUp::getItemText(int item) {
	// Convert 1-indexed item to 0 indexed
	item = item - 1;
	MacMenuItem *menu = getMenuItem(_menuItemId);
	MacMenuItem *submenu = getSubMenuItem(menu, item);
	return getName(submenu);
}
} // end of namespace Graphics
