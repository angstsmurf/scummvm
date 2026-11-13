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

#include "graphics/macgui/macwindow.h"
#include "graphics/macgui/macwindowmanager.h"

#include "director/director.h"
#include "director/lingo/lingo.h"
#include "director/lingo/lingo-code.h"
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
--Window, Tool, 1.0
--© 1989, 1990 MacroMind, Inc.
--by John Thompson
-----------------------------------------------------------------------
--NOTE: This XObject will not create a window with scroll bars.--
-----------------------------------------------------------------------
-----------------------------------------------------------------------
--=METHODS=--
--
IIS 	mNew, kind, titleString	--Creates a new instance of the XObject.
--kind: #document #noGrowDoc, #dialog, #roundDialog, #plainDialog, #altDialog
X 	mDispose	--Disposes of the instance.
S 	mName	--Returns name of the XObject.
X	mSelect	--Brings to the front
X	mShow	--Shows instance.
X	mHide	--Hides instance.
XII	mMove, hpos, vpos	--Moves to hpos, vpos.
XII	mSize, width, height	--Changes size to width, height.
XIO	mAddPanel, symbol, panelObj	--Adds a panel.
OI	mGetPanel, panelSymbolName	--Gets a panel.
XO	mSetHandler, handlerObject	--Sets the event handler. Args:
O	mGetHandler	--Return the event handler
/X	mIdle
/XI	mActivate, flag
/X	mUpdate
/XII	mMouseUp, eventRecPtr, mdcode
/XII	mMouseDown, eventRecPtr, mdcode
/XI	mKeyDown, theChar
/X	mWindowPtr
/XX	mSetTitle
X	+mDisposeAll	--Closes all windows.
OI	+mNthWindow, n	--Returns the n'th window.
 */

/*
 * Reversed from XCOD 2010 of the Director 3.1 installer.
 *
 * Each instance owns a Toolbox window: NewWindow at the global content
 * rect (100,100,200,200), invisible, always with a close box. Director
 * hands it the events of that window through the private methods
 * mMouseDown, mMouseUp, mKeyDown and mUpdate, which forward them to the
 * handler object set with mSetHandler:
 *  - a click in the content area hit-tests the panels (window-local
 *    coordinates) and sends the handler mMouseDown(#panel, h, v); when
 *    that returns false, the panel gets mMouseDown(h, v). Releasing the
 *    button over a panel sends mMouseUp(#panel, h, v). Nothing is sent
 *    for clicks outside the panels.
 *  - a click in the close box sends the handler mGoAway. The window
 *    stays open; the handler disposes it.
 *  - mUpdate is sent to the handler before the window is redrawn.
 * mDispose closes the window but neither disposes nor notifies the
 * panels or the handler.
 */

namespace Director {

const char *const WindowXObj::xlibName = "Window";
const XlibFileDesc WindowXObj::fileNames[] = {
	{ "Window",	nullptr },
	{ nullptr,	nullptr },
};

static const MethodProto xlibMethods[] = {
	{ "new",				WindowXObj::m_new,		 2, 2,	200 },
	{ "dispose",			WindowXObj::m_dispose,	 0, 0,	200 },
	{ "name",				WindowXObj::m_name,		 0, 0,	200 },
	{ "select",				WindowXObj::m_select,	 0, 0,	200 },
	{ "show",				WindowXObj::m_show,		 0, 0,	200 },
	{ "hide",				WindowXObj::m_hide,		 0, 0,	200 },
	{ "move",				WindowXObj::m_move,		 2, 2,	200 },
	{ "size",				WindowXObj::m_size,		 2, 2,	200 },
	{ "addPanel",			WindowXObj::m_addPanel,	 2, 2,	200 },
	{ "getPanel",			WindowXObj::m_getPanel,	 1, 1,	200 },
	{ "setHandler",			WindowXObj::m_setHandler, 1, 1,	200 },
	{ "getHandler",			WindowXObj::m_getHandler, 0, 0,	200 },
	{ "idle",				WindowXObj::m_idle,		 0, 0,	200 },
	{ "activate",			WindowXObj::m_activate,	 1, 1,	200 },
	{ "update",				WindowXObj::m_update,	 0, 0,	200 },
	{ "mouseUp",			WindowXObj::m_mouseUp,	 2, 2,	200 },
	{ "mouseDown",			WindowXObj::m_mouseDown, 2, 2,	200 },
	{ "keyDown",			WindowXObj::m_keyDown,	 1, 1,	200 },
	{ "windowPtr",			WindowXObj::m_windowPtr, 0, 0,	200 },
	{ "setTitle",			WindowXObj::m_setTitle,	 1, 1,	200 },
	{ "disposeAll",			WindowXObj::m_disposeAll, 0, 0,	200 },
	{ "nthWindow",			WindowXObj::m_nthWindow, 1, 1,	200 },
	{ nullptr, nullptr, 0, 0, 0 }
};

WindowXObject::WindowXObject(ObjectType ObjectType) :Object<WindowXObject>("Window") {
	_objType = ObjectType;
}

WindowXObject::~WindowXObject() {
	destroy();
}

enum WindowEventType {
	kWinEventMouseDown,
	kWinEventMouseUp,
	kWinEventGoAway,
	kWinEventKeyDown,
};

struct WindowEvent {
	WindowEventType type;
	Datum window;
	Common::Point pos;
	int key;
};

// Open windows in the order they were made; each holds a reference.
static Common::Array<WindowXObject *> *g_windowList = nullptr;
static Common::Queue<WindowEvent> *g_eventList = nullptr;
static WindowXObject *g_goAwayWindow = nullptr;

static Common::Array<WindowXObject *> &windows() {
	if (!g_windowList)
		g_windowList = new Common::Array<WindowXObject *>();
	return *g_windowList;
}

static Common::Queue<WindowEvent> &eventQueue() {
	if (!g_eventList)
		g_eventList = new Common::Queue<WindowEvent>();
	return *g_eventList;
}

static bool windowCallback(Graphics::WindowClick click, Common::Event &event, void *data);

static int kindToProcID(const Common::String &kind) {
	// NewWindow procIDs
	if (kind.equalsIgnoreCase("document"))
		return 0;
	if (kind.equalsIgnoreCase("noGrowDoc"))
		return 4;
	if (kind.equalsIgnoreCase("dialog"))
		return 1;
	if (kind.equalsIgnoreCase("roundDialog"))
		return 16;
	if (kind.equalsIgnoreCase("plainDialog"))
		return 2;
	if (kind.equalsIgnoreCase("altDialog"))
		return 3;
	return 0;
}

void WindowXObject::create(int procID, const Common::String &title) {
	Graphics::MacWindowManager *wm = g_director->getMacWindowManager();
	_window = wm->addWindow(false, false, false);
	_window->setVisible(false);
	_window->setDraggable(true);
	_window->setCloseable(false); // the close box is reported to the handler
	_window->setBorderType(procID);
	_window->setTitle(title);
	_window->setCallback(windowCallback, this);
	_window->setInnerDimensions(Common::Rect(100, 100, 200, 200));
	redraw();

	windows().push_back(this);
	incRefCount();
}

void WindowXObject::destroy() {
	for (uint i = 0; i < _panels.size(); i++) {
		PanelXObject *panel = static_cast<PanelXObject *>(_panels[i].u.obj);
		if (panel->_window == this)
			panel->_window = nullptr;
	}
	_panels.clear();
	_panelNames.clear();
	_handler = Datum();

	if (g_goAwayWindow == this)
		g_goAwayWindow = nullptr;

	if (_window) {
		Graphics::MacWindowManager *wm = g_director->getMacWindowManager();
		bool wasActive = wm->getActiveWindow() == _window->getId();
		wm->removeWindow(_window);
		Graphics::MacWindow *closed = _window;
		_window = nullptr;

		// Like CloseWindow, disposing the active window activates the
		// frontmost window that is left. Only our own windows take part;
		// the stage stays at the back.
		if (wasActive) {
			const Common::List<Graphics::BaseMacWindow *> &stack = wm->getWindowStack();
			for (Common::List<Graphics::BaseMacWindow *>::const_iterator it = stack.reverse_begin(); it != stack.end(); --it) {
				if (*it == closed || !(*it)->isVisible())
					continue;
				bool ours = false;
				for (uint i = 0; i < windows().size(); i++)
					if (windows()[i]->_window == *it)
						ours = true;
				if (ours)
					wm->setActiveWindow((*it)->getId());
				break;
			}
		}
	}
}

void WindowXObject::redraw() {
	if (!_window)
		return;

	Graphics::ManagedSurface *surface = _window->getWindowSurface();
	surface->fillRect(Common::Rect(surface->w, surface->h), g_director->_wm->_colorWhite);
	for (uint i = 0; i < _panels.size(); i++)
		static_cast<PanelXObject *>(_panels[i].u.obj)->draw(surface);
	_window->setContentDirty(true);
}

PanelXObject *WindowXObject::findPanel(const Common::String &name) {
	for (uint i = 0; i < _panelNames.size(); i++) {
		if (_panelNames[i].equalsIgnoreCase(name))
			return static_cast<PanelXObject *>(_panels[i].u.obj);
	}
	return nullptr;
}

PanelXObject *WindowXObject::panelAt(const Common::Point &pos) {
	// The original walks its panel hash table from the highest slot down;
	// the most recent panel first is the closest stand-in. Hidden panels
	// are hit as well.
	for (int i = (int)_panels.size() - 1; i >= 0; i--) {
		PanelXObject *panel = static_cast<PanelXObject *>(_panels[i].u.obj);
		if (panel->_rect.contains(pos))
			return panel;
	}
	return nullptr;
}

void WindowXObject::addPanel(const Common::String &name, PanelXObject *panel) {
	for (uint i = 0; i < _panelNames.size(); i++) {
		if (_panelNames[i].equalsIgnoreCase(name)) {
			_panels[i] = Datum(panel);
			redraw();
			return;
		}
	}
	_panelNames.push_back(name);
	_panels.push_back(Datum(panel));
	redraw();
}

void WindowXObject::removePanel(PanelXObject *panel) {
	for (uint i = 0; i < _panels.size(); i++) {
		if (_panels[i].u.obj == panel) {
			_panels.remove_at(i);
			_panelNames.remove_at(i);
			redraw();
			return;
		}
	}
}

static void disposeWindow(WindowXObject *me) {
	for (uint i = 0; i < windows().size(); i++) {
		if (windows()[i] == me) {
			windows().remove_at(i);
			me->destroy();
			me->dispose();
			me->decRefCount();
			return;
		}
	}
}

static bool windowCallback(Graphics::WindowClick click, Common::Event &event, void *data) {
	WindowXObject *me = (WindowXObject *)data;
	if (!me->_window || !me->_window->isVisible())
		return false;

	const Common::Rect &dims = me->_window->getDimensions();
	const Common::Rect &inner = me->_window->getInnerDimensions();
	// The event comes relative to the outer window rect
	Common::Point local(event.mouse.x + dims.left - inner.left, event.mouse.y + dims.top - inner.top);

	WindowEvent ev;
	ev.window = Datum(me);
	ev.pos = local;
	ev.key = 0;

	switch (event.type) {
	case Common::EVENT_LBUTTONDOWN:
		g_goAwayWindow = nullptr;
		if (click == Graphics::kBorderCloseButton) {
			// TrackGoAway: the close box counts if released inside it
			g_goAwayWindow = me;
		} else if (click == Graphics::kBorderInner) {
			ev.type = kWinEventMouseDown;
			eventQueue().push(ev);
		}
		return true;
	case Common::EVENT_LBUTTONUP:
		if (click == Graphics::kBorderCloseButton) {
			if (g_goAwayWindow == me) {
				ev.type = kWinEventGoAway;
				eventQueue().push(ev);
			}
		} else if (click == Graphics::kBorderInner) {
			ev.type = kWinEventMouseUp;
			eventQueue().push(ev);
		}
		g_goAwayWindow = nullptr;
		return true;
	case Common::EVENT_KEYDOWN:
		ev.type = kWinEventKeyDown;
		ev.key = event.kbd.ascii;
		eventQueue().push(ev);
		return true;
	case Common::EVENT_MOUSEMOVE:
		return false;
	default:
		return click != Graphics::kBorderNone;
	}
}

// Sends a message to a Lingo object. Returns false if it has no such method.
static bool sendMessage(const Datum &obj, const char *method, const Common::Array<Datum> &args, bool wantResult, Datum *result) {
	if (obj.type != OBJECT || obj.u.obj->isDisposed())
		return false;

	Symbol sym = obj.u.obj->getMethod(method);
	if (sym.type == VOIDSYM) {
		warning("Window: handler <%s> has no %s method", obj.asString(true).c_str(), method);
		return false;
	}

	uint stackSize = g_lingo->_state->stack.size();
	g_lingo->push(obj);
	for (uint i = 0; i < args.size(); i++)
		g_lingo->push(args[i]);

	int cframe = g_lingo->_state->callstack.size();
	LC::call(sym, args.size() + 1, wantResult);
	g_lingo->execute(cframe);

	Datum res;
	if (wantResult && g_lingo->_state->stack.size() > stackSize)
		res = g_lingo->pop();
	if (result)
		*result = res;
	return true;
}

static Datum symbolDatum(const Common::String &name) {
	Datum d(name);
	d.type = SYMBOL;
	return d;
}

static void dispatchEvent(WindowEvent &ev) {
	WindowXObject *me = static_cast<WindowXObject *>(ev.window.u.obj);
	if (me->isDisposed() || !me->_window)
		return;

	Datum handler = me->_handler;

	if (ev.type == kWinEventMouseUp) {
		// The panel's mMouseDown tracks the button until it is released
		for (uint i = 0; i < me->_panels.size(); i++) {
			PanelXObject *panel = static_cast<PanelXObject *>(me->_panels[i].u.obj);
			if (panel->_tracking) {
				panel->_tracking = false;
				panel->changed();
			}
		}
	}

	if (handler.type != OBJECT)
		return;

	switch (ev.type) {
	case kWinEventMouseDown:
	case kWinEventMouseUp: {
		PanelXObject *panel = me->panelAt(ev.pos);
		if (!panel)
			return;

		Datum panelDatum(panel);
		Common::Array<Datum> args;
		args.push_back(symbolDatum(panel->_panelName));
		args.push_back(Datum(ev.pos.x));
		args.push_back(Datum(ev.pos.y));

		if (ev.type == kWinEventMouseUp) {
			sendMessage(handler, "mMouseUp", args, false, nullptr);
			return;
		}

		Datum result;
		if (sendMessage(handler, "mMouseDown", args, true, &result) && result.asInt() == 0 && !panel->isDisposed()) {
			// Panel mMouseDown: hilite until the button is released
			if (panel->_kind >= 0 && g_director->_wm->_mouseDown) {
				panel->_tracking = true;
				panel->changed();
			}
		}
		break;
	}
	case kWinEventGoAway: {
		Common::Array<Datum> args;
		sendMessage(handler, "mGoAway", args, false, nullptr);
		break;
	}
	case kWinEventKeyDown: {
		Common::Array<Datum> args;
		args.push_back(Datum(ev.key));
		sendMessage(handler, "mKeyDown", args, false, nullptr);
		break;
	}
	}
}

void WindowXObj::processEvents() {
	if (!g_director->_wm->_mouseDown) {
		for (uint w = 0; w < windows().size(); w++) {
			WindowXObject *win = windows()[w];
			for (uint i = 0; i < win->_panels.size(); i++) {
				PanelXObject *panel = static_cast<PanelXObject *>(win->_panels[i].u.obj);
				if (panel->_tracking) {
					panel->_tracking = false;
					panel->changed();
				}
			}
		}
	}

	if (eventQueue().empty())
		return;
	if (!g_lingo->_state->callstack.empty() || g_lingo->_currentInputEvent.type != VOIDSYM)
		return;

	while (!eventQueue().empty()) {
		WindowEvent ev = eventQueue().pop();
		dispatchEvent(ev);
	}
}

void WindowXObj::open(ObjectType type, const Common::Path &path) {
	if (type == kXObj) {
		WindowXObject::initMethods(xlibMethods);
		WindowXObject *xobj = new WindowXObject(kXObj);
		g_lingo->exposeXObject(xlibName, xobj);
	}
}

void WindowXObj::close(ObjectType type) {
	if (type == kXObj) {
		while (!windows().empty())
			disposeWindow(windows().back());
		delete g_windowList;
		g_windowList = nullptr;
		delete g_eventList;
		g_eventList = nullptr;
		WindowXObject::cleanupMethods();
		g_lingo->_globalvars[xlibName] = Datum();
	}
}

void WindowXObj::m_new(int nargs) {
	WindowXObject *me = static_cast<WindowXObject *>(g_lingo->_state->me.u.obj);
	Datum title = g_lingo->pop();
	Datum kind = g_lingo->pop();

	me->create(kindToProcID(kind.asString()), title.asString());
	g_lingo->push(g_lingo->_state->me);
}

void WindowXObj::m_dispose(int nargs) {
	WindowXObject *me = static_cast<WindowXObject *>(g_lingo->_state->me.u.obj);
	disposeWindow(me);
}

void WindowXObj::m_name(int nargs) {
	g_lingo->push(Datum("Window"));
}

void WindowXObj::m_select(int nargs) {
	WindowXObject *me = static_cast<WindowXObject *>(g_lingo->_state->me.u.obj);
	if (me->_window)
		g_director->getMacWindowManager()->setActiveWindow(me->_window->getId());
}

void WindowXObj::m_show(int nargs) {
	WindowXObject *me = static_cast<WindowXObject *>(g_lingo->_state->me.u.obj);
	if (me->_window)
		me->_window->setVisible(true);
}

void WindowXObj::m_hide(int nargs) {
	WindowXObject *me = static_cast<WindowXObject *>(g_lingo->_state->me.u.obj);
	if (me->_window)
		me->_window->setVisible(false);
}

void WindowXObj::m_move(int nargs) {
	WindowXObject *me = static_cast<WindowXObject *>(g_lingo->_state->me.u.obj);
	int v = g_lingo->pop().asInt();
	int h = g_lingo->pop().asInt();
	if (!me->_window)
		return;
	// MoveWindow: h, v is the global top left of the content area
	Common::Rect inner = me->_window->getInnerDimensions();
	inner.moveTo(h, v);
	me->_window->setInnerDimensions(inner);
}

void WindowXObj::m_size(int nargs) {
	WindowXObject *me = static_cast<WindowXObject *>(g_lingo->_state->me.u.obj);
	int height = g_lingo->pop().asInt();
	int width = g_lingo->pop().asInt();
	if (!me->_window)
		return;
	Common::Rect inner = me->_window->getInnerDimensions();
	inner.setWidth(MAX(width, 0));
	inner.setHeight(MAX(height, 0));
	me->_window->setInnerDimensions(inner);
	me->redraw();
}

void WindowXObj::m_addPanel(int nargs) {
	WindowXObject *me = static_cast<WindowXObject *>(g_lingo->_state->me.u.obj);
	Datum panel = g_lingo->pop();
	Datum name = g_lingo->pop();
	if (panel.type != OBJECT || panel.u.obj->getName() != "Panel") {
		warning("WindowXObj::m_addPanel: %s is not a panel", panel.asString(true).c_str());
		return;
	}
	PanelXObject *p = static_cast<PanelXObject *>(panel.u.obj);
	p->_window = me;
	me->addPanel(name.asString(), p);
}

void WindowXObj::m_getPanel(int nargs) {
	WindowXObject *me = static_cast<WindowXObject *>(g_lingo->_state->me.u.obj);
	Datum name = g_lingo->pop();
	PanelXObject *panel = me->findPanel(name.asString());
	if (panel)
		g_lingo->push(Datum(panel));
	else
		g_lingo->push(Datum(0));
}

void WindowXObj::m_setHandler(int nargs) {
	WindowXObject *me = static_cast<WindowXObject *>(g_lingo->_state->me.u.obj);
	me->_handler = g_lingo->pop();
}

void WindowXObj::m_getHandler(int nargs) {
	WindowXObject *me = static_cast<WindowXObject *>(g_lingo->_state->me.u.obj);
	if (me->_handler.type == OBJECT)
		g_lingo->push(me->_handler);
	else
		g_lingo->push(Datum(0));
}

// The private methods are Director's way of handing the XObject the
// events of its window. They are not meant to be called from Lingo.
XOBJSTUBNR(WindowXObj::m_idle)
XOBJSTUBNR(WindowXObj::m_activate)

void WindowXObj::m_update(int nargs) {
	WindowXObject *me = static_cast<WindowXObject *>(g_lingo->_state->me.u.obj);
	Common::Array<Datum> args;
	if (me->_handler.type == OBJECT)
		sendMessage(me->_handler, "mUpdate", args, false, nullptr);
	me->redraw();
}

XOBJSTUBNR(WindowXObj::m_mouseUp)
XOBJSTUBNR(WindowXObj::m_mouseDown)
XOBJSTUBNR(WindowXObj::m_keyDown)

void WindowXObj::m_windowPtr(int nargs) {
	WindowXObject *me = static_cast<WindowXObject *>(g_lingo->_state->me.u.obj);
	g_lingo->push(Datum(me->_window ? (int)me->_window->getId() + 1 : 0));
}

void WindowXObj::m_setTitle(int nargs) {
	WindowXObject *me = static_cast<WindowXObject *>(g_lingo->_state->me.u.obj);
	Datum title = g_lingo->pop();
	if (me->_window)
		me->_window->setTitle(title.asString());
}

void WindowXObj::m_disposeAll(int nargs) {
	while (!windows().empty())
		disposeWindow(windows()[0]);
}

void WindowXObj::m_nthWindow(int nargs) {
	int n = g_lingo->pop().asInt();
	if (n >= 0 && n < (int)windows().size())
		g_lingo->push(Datum(windows()[n]));
	else
		g_lingo->push(Datum(0));
}

}
