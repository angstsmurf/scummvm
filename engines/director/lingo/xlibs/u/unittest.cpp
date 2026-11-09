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
 * ScummVM Unit Testing framework.
 * Used in the Director Test suite - https://github.com/scummvm/director-tests
 *
 *************************************/

#include "common/events.h"
#include "director/director.h"
#include "director/archive.h"
#include "director/lingo/lingo.h"
#include "director/lingo/lingo-object.h"
#include "director/lingo/xlibs/u/unittest.h"
#include "director/movie.h"
#include "director/score.h"
#include "director/window.h"
#ifdef USE_PNG
#include "image/png.h"
#else
#include "image/bmp.h"
#endif

namespace Director {

const char *const UnitTestXObj::xlibName = "UnitTest";
const XlibFileDesc UnitTestXObj::fileNames[] = {
	{ "UnitTest",	nullptr },
	{ nullptr,		nullptr },
};
/*
-- ScummVM UnitTest XObject.
UnitTest
I      mNew                     --Creates a new instance of the XObject
X      mDispose                 --Disposes of XObject instance
I      mIsRealDirector          --Returns 1 for real Director, 0 for ScummVM
IS     mScreenshot, path        --Copy contents of stage window to file
III    mMoveMouse, x, y         --Move the mouse pointer to window position (x, y)
I      mLeftMouseDown           --Press the LMB
I      mLeftMouseUp             --Release the LMB
II     mKeyPress, code          --Type the key with this ASCII code
III    mPostKeyPress, code, delay --Queue the key after delay ms (0 = now)
II     mPostLeftClick, delay    --Queue a left click at the pointer after delay ms
 */

static const MethodProto xlibMethods[] = {
	{ "new",				UnitTestXObj::m_new,				0, 0,	400 },	// D4
	{ "dispose",			UnitTestXObj::m_dispose,			0, 0,	400 },	// D4
	{ "isRealDirector",		UnitTestXObj::m_isRealDirector,		0, 0,	400 },	// D4
	{ "screenshot",			UnitTestXObj::m_screenshot,			1, 1,	400 },	// D4
	{ "moveMouse",			UnitTestXObj::m_moveMouse,			2, 2,	400 },	// D4
	{ "leftMouseDown",		UnitTestXObj::m_leftMouseDown,		0, 0,	400 },	// D4
	{ "leftMouseUp",		UnitTestXObj::m_leftMouseUp,		0, 0,	400 },	// D4
	{ "keyPress",			UnitTestXObj::m_keyPress,			1, 1,	400 },	// D4
	{ "postKeyPress",		UnitTestXObj::m_postKeyPress,		2, 2,	400 },	// D4
	{ "postLeftClick",		UnitTestXObj::m_postLeftClick,		1, 1,	400 },	// D4
	{ nullptr, nullptr, 0, 0, 0 }
};

void UnitTestXObj::open(ObjectType type, const Common::Path &path) {
	if (type == kXObj) {
		UnitTestXObject::initMethods(xlibMethods);
		UnitTestXObject *xobj = new UnitTestXObject(kXObj);
		g_lingo->exposeXObject(xlibName, xobj);
	}
}

void UnitTestXObj::close(ObjectType type) {
	if (type == kXObj) {
		UnitTestXObject::cleanupMethods();
		g_lingo->_globalvars[xlibName] = Datum();
	}
}

UnitTestXObject::UnitTestXObject(ObjectType ObjectType) :Object<UnitTestXObject>("UnitTest") {
	_objType = ObjectType;
}

void UnitTestXObj::m_new(int nargs) {
	g_lingo->push(g_lingo->_state->me);
}

void UnitTestXObj::m_dispose(int nargs) {
}

void UnitTestXObj::m_isRealDirector(int nargs) {
	g_lingo->push(0);
}

void UnitTestXObj::m_screenshot(int nargs) {
	if (nargs == 0) {
		g_lingo->push(Datum(0));
		warning("UnitTestXObj::m_screenshot(): expected filename argument");
		return;
	}
	if (nargs > 1) {
		g_lingo->dropStack(nargs - 1);
		nargs = 1;
	}
	Datum name = g_lingo->pop();
	if (name.type != STRING) {
		warning("UnitTestXObj::m_screenshot(): expected string for arg 1");
		g_lingo->push(Datum(0));
		return;
	}
	Common::String filenameBase = *name.u.s;

	Common::FSNode gameDataDir = g_director->_gameDataDir;
	Common::FSNode screenDir = gameDataDir.getChild("scrtest");
	if (!screenDir.exists()) {
		screenDir.createDirectory();
	}

	// Fetch whatever is in the screen buffer.
	// Don't force a redraw, we do that in the script with updateStage().
	Window *window = g_director->getCurrentWindow();
	Graphics::ManagedSurface *windowSurface = window->getSurface();

#ifdef USE_PNG
	Common::FSNode file = screenDir.getChild(Common::String::format("%s.png", filenameBase.c_str()));
#else
	Common::FSNode file = screenDir.getChild(Common::String::format("%s.bmp", filenameBase.c_str()));
#endif

	Common::SeekableWriteStream *stream = file.createWriteStream();
	if (!stream) {
		warning("UnitTestXObj::m_screenshot(): could not open file %s", file.getPath().toString(Common::Path::kNativeSeparator).c_str());
		return;
	}

	bool success = false;
#ifdef USE_PNG
	if (windowSurface->format.bytesPerPixel == 1) {
		success = Image::writePNG(*stream, *windowSurface, g_director->getPalette());
	} else {
		success = Image::writePNG(*stream, *windowSurface);
	}
#else
	success = Image::writeBMP(*stream, *windowSurface);
#endif
	if (!success) {
		warning("UnitTestXObj::m_screenshot(): error writing screenshot data to file %s", file.getPath().toString(Common::Path::kNativeSeparator).c_str());
	}
	stream->finalize();
	delete stream;
}

void UnitTestXObj::m_moveMouse(int nargs) {
	if (nargs != 2) {
		warning("UnitTestXObj::m_moveMouse: expected 2 arguments");
		g_lingo->dropStack(nargs);
		g_lingo->push(0);
		return;
	}
	UnitTestXObject *me = static_cast<UnitTestXObject *>(g_lingo->_state->me.u.obj);
	int16 y = (int16)g_lingo->pop().asInt();
	int16 x = (int16)g_lingo->pop().asInt();
	Common::Event ev;
	ev.type = Common::EVENT_MOUSEMOVE;
	ev.mouse = Common::Point(x, y);
	me->_mousePos = ev.mouse;
	g_director->_injectedEvents.push_back(ev);
	g_lingo->push(0);
}

void UnitTestXObj::m_leftMouseDown(int nargs) {
	if (nargs != 0) {
		warning("UnitTestXObj::m_leftMouseDown: expected 0 arguments");
		g_lingo->dropStack(nargs);
		g_lingo->push(0);
		return;
	}
	UnitTestXObject *me = static_cast<UnitTestXObject *>(g_lingo->_state->me.u.obj);
	Common::Event ev;
	ev.type = Common::EVENT_LBUTTONDOWN;
	ev.mouse = me->_mousePos;
	g_director->_injectedEvents.push_back(ev);
	// The Windows XObject sends the click with SendMessage, so the stage
	// handles it before the call returns. Lingo still runs, so its
	// mouseDown and mouseUp handlers are queued for later.
	g_director->processSysEvents();
	g_lingo->push(0);
}

void UnitTestXObj::m_leftMouseUp(int nargs) {
	if (nargs != 0) {
		warning("UnitTestXObj::m_leftMouseDown: expected 0 arguments");
		g_lingo->dropStack(nargs);
		g_lingo->push(0);
		return;
	}
	UnitTestXObject *me = static_cast<UnitTestXObject *>(g_lingo->_state->me.u.obj);
	Common::Event ev;
	ev.type = Common::EVENT_LBUTTONUP;
	ev.mouse = me->_mousePos;
	g_director->_injectedEvents.push_back(ev);
	// Handled at once, as in m_leftMouseDown().
	g_director->processSysEvents();
	g_lingo->push(0);
}

void UnitTestXObj::m_keyPress(int nargs) {
	if (nargs != 1) {
		warning("UnitTestXObj::m_keyPress: expected 1 argument");
		g_lingo->dropStack(nargs);
		g_lingo->push(0);
		return;
	}
	// The code is an ASCII code (8 = Backspace, 9 = Tab, 13 = Return),
	// which is also the matching Common::KeyCode.
	int code = g_lingo->pop().asInt() & 0xff;
	Common::Event ev;
	ev.type = Common::EVENT_KEYDOWN;
	ev.kbd = Common::KeyState((Common::KeyCode)code, code);
	g_director->_injectedEvents.push_back(ev);
	ev.type = Common::EVENT_KEYUP;
	g_director->_injectedEvents.push_back(ev);
	// Handled at once, as in m_leftMouseDown().
	g_director->processSysEvents();
	g_lingo->push(0);
}

// The Windows XObject posts these with PostMessage (from a timer when delay > 0),
// so unlike mKeyPress and the mouse methods they wait in the queue until
// Director next looks for input.
void UnitTestXObj::m_postKeyPress(int nargs) {
	if (nargs != 2) {
		warning("UnitTestXObj::m_postKeyPress: expected 2 arguments");
		g_lingo->dropStack(nargs);
		g_lingo->push(0);
		return;
	}
	int delay = MAX(0, g_lingo->pop().asInt());
	int code = g_lingo->pop().asInt() & 0xff;
	Common::Event ev;
	ev.type = Common::EVENT_KEYDOWN;
	ev.kbd = Common::KeyState((Common::KeyCode)code, code);
	g_director->postEvent(ev, delay);
	ev.type = Common::EVENT_KEYUP;
	g_director->postEvent(ev, delay);
	g_lingo->push(0);
}

void UnitTestXObj::m_postLeftClick(int nargs) {
	if (nargs != 1) {
		warning("UnitTestXObj::m_postLeftClick: expected 1 argument");
		g_lingo->dropStack(nargs);
		g_lingo->push(0);
		return;
	}
	UnitTestXObject *me = static_cast<UnitTestXObject *>(g_lingo->_state->me.u.obj);
	int delay = MAX(0, g_lingo->pop().asInt());
	Common::Event ev;
	ev.type = Common::EVENT_LBUTTONDOWN;
	ev.mouse = me->_mousePos;
	g_director->postEvent(ev, delay);
	ev.type = Common::EVENT_LBUTTONUP;
	g_director->postEvent(ev, delay);
	g_lingo->push(0);
}

} // End of namespace Director
