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

#include "glk/comprehend/comprehend.h"
#include "glk/comprehend/apple2.h"
#include "glk/comprehend/debugger.h"
#include "glk/comprehend/draw_surface.h"
#include "common/config-manager.h"
#include "engines/util.h"
#include "glk/comprehend/graphics_magician.h"
#include "glk/comprehend/pics.h"

namespace Glk {
namespace Comprehend {

Comprehend *g_comprehend;

Comprehend::Comprehend(OSystem *syst, const GlkGameDescription &gameDesc) : GlkAPI(syst, gameDesc),
	_saveSlot(-1), _graphicsEnabled(true), _disableSaves(false),
	_topWindow(nullptr), _statusWindow(nullptr), _bottomWindow(nullptr),
	_drawSurface(nullptr), _game(nullptr), _pics(nullptr), _drawFlags(0),
	_pixelSize(2), _slowDrawActive(false), _suppressSlowDraw(false) {
	g_comprehend = this;
}

Comprehend::~Comprehend() {
	delete _drawSurface;
	delete _game;
	delete _pics;
	gmFreeState();
	DiskImageFS::clear();
	g_comprehend = nullptr;
}

void Comprehend::createConfiguration() {
	GlkAPI::createConfiguration();
	switchToWhiteOnBlack();
}

void Comprehend::createDebugger() {
	setDebugger(new Debugger());
}

void Comprehend::initGraphicsMode() {
	// A picture takes half the width in height, which leaves too little room for
	// the text at the 16:10 shape the other interpreters use, so use 4:3.
	_conf->_width = _conf->_imageW = MAX<uint>(_conf->_width, 640);
	_conf->_height = _conf->_imageH = _conf->_width * 3 / 4;
	Graphics::PixelFormat pixelFormat(2, 5, 6, 5, 0, 11, 5, 0, 0);
	initGraphics(_conf->_width, _conf->_height, &pixelFormat);
}

bool Comprehend::loadLauncherSavegameIfNeeded() {
	if (_saveSlot != -1)
		return loadGameState(_saveSlot).getCode() == Common::kNoError;
	return false;
}

void Comprehend::runGame() {
	_saveSlot = ConfMan.hasKey("save_slot") ? ConfMan.getInt("save_slot") : -1;
	readOptions();
	_traits = findGameTraits(getGameID().c_str());
	gmResetState();

	// The renderer, and with it the surface width, depends on the disk image.
	if (hasAppleDiskImageExtension(getFilename()))
		loadAppleDiskImage(getFilename());

	initialize();
	createGame();
	if (_game) {
		_game->loadGame();
		applyPreferredGraphicsMode();
		_game->playGame();
	}
	deinitialize();
}

void Comprehend::initialize() {
	// Centred style for text the original interpreter centres on screen (e.g.
	// the OO-Topos title credits). Hints must be set before the window opens.
	glk_stylehint_set(wintype_TextBuffer, style_User1,
					  stylehint_Justification, stylehint_just_Centered);

	// Glk's default "more" prompt colour is black, which can't be seen on the
	// default black window, so use the text colour. Saved Game Options can make
	// that black too; Glk then draws the text light grey, so do the same.
	// A window takes its prompt colour when it opens.
	if (_conf->_propInfo._moreColor == _conf->_windowColor) {
		uint color = _conf->_tStyles[style_Normal].fg;
		if (color == _conf->_windowColor)
			color = Windows::_zcolor_LightGrey;
		_conf->_propInfo._moreColor = color;
		_conf->_propInfo._moreSave = color;
	}

	_bottomWindow = glk_window_open(nullptr, 0, 0, wintype_TextBuffer, GLK_BUFFER_ROCK);
	glk_set_window(_bottomWindow);
	_statusWindow = glk_window_open(_bottomWindow,
		winmethod_Above | winmethod_Fixed, 1, wintype_TextGrid, GLK_STATUS_ROCK);
	showGraphics();   // closes _statusWindow, unless graphics are off

	_drawSurface = new DrawSurface();
	_drawSurface->setRenderWidth(renderWidth());
	_pics = new Pics();
}

void Comprehend::deinitialize() {
}

void Comprehend::createGame() {
	if (!_traits)
		error("Unknown game id '%s'", getGameID().c_str());
	_game = _traits->create();

	// The Apple II hi-res renderer (graphics_magician.cpp) handles two Graphics
	// Magician dialects. Talisman and OO-Topos
	// use op15 fill-bounds sub-ops (newer); Transylvania and Crimson Crown never
	// emit op15, end images on op7, and seed background fills in the bottom text
	// rows, so they need the full-screen fill clip (the legacy flag).
	gmSetLegacyFormat(_traits->gmLegacyFormat);
}

} // namespace Comprehend
} // namespace Glk
