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
 * Safecracker (Mac)
 *
 *************************************/

/*
 * PanoMovie and NavMovie are copies of Apple's QTMovie XCMD (1995) with the
 * QuickTime VR extensions, linked into the projector's resource fork.
 * PanoMovie shows the panoramas, NavMovie the object movies.
 *
 *   id = PanoMovie("OpenMovie", "Direct", fileName, "l,t,r,b" | "l,t", options...)
 *   PanoMovie("Direct", id, command)
 *   PanoMovie("Direct", id, "Set", property, value)
 *   value = PanoMovie("Direct", id, "Get", property)
 *
 * Commands: Update, Idle, MouseOver, ExitMouseOver, MouseDown, dispose.
 * Properties: nodeID, zoomAngle, hPanAngle, vPanAngle, quality, warpMode,
 * transitionMode, transitionSpeed, updateMode, mouseOverHandler,
 * mouseDownHandler, nodeLeaveHandler, CollapseToHotSpotRgn, MovieRect.
 *
 * MouseOver blocks while the mouse is over the movie, calling the
 * mouseOverHandler with "h,v" on every pass. A click is passed to the QTVR
 * controller, which pans or follows the hotspot, and the call returns
 * "type,id" of the clicked hotspot (also in the result).
 */

#include "common/system.h"
#include "common/tokenizer.h"
#include "common/formats/quicktime.h"

#include "graphics/macgui/macwindowmanager.h"

#include "video/qt_decoder.h"

#include "director/director.h"
#include "director/window.h"
#include "director/util.h"
#include "director/lingo/lingo.h"
#include "director/lingo/lingo-builtins.h"
#include "director/lingo/lingo-object.h"
#include "director/lingo/xlibs/p/panomovie.h"

namespace Director {

const char *const PanoMovieXCMD::xlibName = "PanoMovie";
const XlibFileDesc PanoMovieXCMD::fileNames[] = {
	{ "PanoMovie",	nullptr },
	{ nullptr,		nullptr },
};

const char *const NavMovieXCMD::xlibName = "NavMovie";
const XlibFileDesc NavMovieXCMD::fileNames[] = {
	{ "NavMovie",	nullptr },
	{ nullptr,		nullptr },
};

const char *const RunningInForegroundXFCN::xlibName = "RunningInForeground";
const XlibFileDesc RunningInForegroundXFCN::fileNames[] = {
	{ "RunningInForeground",	nullptr },
	{ nullptr,					nullptr },
};

// XCMDs can be used both as commands and as functions. Register separate
// entries so the command form sets the result without returning a value.
static const BuiltinProto panoBuiltins[] = {
	{ "PanoMovie", PanoMovieXCMD::m_panoMovie,		-1, 0, 400, FBLTIN },
	{ "PanoMovie", PanoMovieXCMD::m_panoMovieCmd,	-1, 0, 400, CBLTIN },
	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

static const BuiltinProto navBuiltins[] = {
	{ "NavMovie", NavMovieXCMD::m_navMovie,			-1, 0, 400, FBLTIN },
	{ "NavMovie", NavMovieXCMD::m_navMovieCmd,		-1, 0, 400, CBLTIN },
	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

static const BuiltinProto foregroundBuiltins[] = {
	{ "RunningInForeground", RunningInForegroundXFCN::m_runningInForeground, 0, 0, 400, HBLTIN },
	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

typedef Common::HashMap<int, PanoMovieState *> PanoMovieMap;

static PanoMovieMap *s_panoMovies = nullptr;
static PanoMovieMap *s_navMovies = nullptr;
static int s_nextMovieID = 1;

static void disposeAll(PanoMovieMap *&movies) {
	if (!movies)
		return;
	for (auto &it : *movies)
		delete it._value;
	delete movies;
	movies = nullptr;
}

PanoMovieState::~PanoMovieState() {
	_offscreen.free();
	delete _widget;
	if (_video) {
		_video->close();
		delete _video;
	}
}

static Common::Array<int> parseNumbers(const Common::String &str) {
	Common::Array<int> res;
	Common::StringTokenizer tok(str, ",");
	while (!tok.empty())
		res.push_back(atoi(tok.nextToken().c_str()));
	return res;
}

static void drawMovie(PanoMovieState *m) {
	if (!m->_video)
		return;

	const Common::String &mode = m->_video->getUpdateMode();
	const Graphics::Surface *frame;

	if (mode.equalsIgnoreCase("fromOffscreen") && m->_offscreen.getPixels()) {
		// Show what the last offscreenOnly update rendered, without
		// rendering again: Safecracker unlocks the stage between the two,
		// and a fresh render of a panorama can take a while
		frame = &m->_offscreen;
	} else {
		frame = m->_video->decodeNextFrame();
		if (!frame)
			return;

		if (mode.equalsIgnoreCase("offscreenOnly")) {
			m->_offscreen.free();
			m->_offscreen.copyFrom(*frame);
			return;
		}
	}

	Graphics::Surface *dither = frame->convertTo(g_director->_wm->_pixelformat, m->_video->getPalette(), 256, g_director->getPalette(), 256, Graphics::kDitherNaive);

	Window *window = g_director->getCurrentWindow();
	Graphics::ManagedSurface *surface = window->getSurface();

	Common::Rect dst(m->_rect.left, m->_rect.top, m->_rect.left + dither->w, m->_rect.top + dither->h);
	dst.clip(Common::Rect(surface->w, surface->h));
	if (!dst.isEmpty()) {
		surface->copyRectToSurface(dither->getBasePtr(dst.left - m->_rect.left, dst.top - m->_rect.top),
			dither->pitch, dst.left, dst.top, dst.width(), dst.height());
		window->setDirty(true);
	}

	dither->free();
	delete dither;

	if (frame == &m->_offscreen)
		m->_offscreen.free();
}

static Datum openMovie(PanoMovieMap *&movies, const Common::Array<Datum> &args) {
	if (args.size() < 4)
		return Datum("error: OpenMovie needs a window type, file name and location");

	Common::String pathStr = args[2].asString();
	Common::Array<int> loc = parseNumbers(args[3].asString());
	if (loc.size() != 2 && loc.size() != 4)
		return Datum(Common::String::format("error: bad location '%s'", args[3].asString().c_str()));

	Common::Path path = findMoviePath(pathStr);
	if (path.empty())
		return Datum(Common::String::format("error: movie file '%s' not found", pathStr.c_str()));

	PanoMovieState *m = new PanoMovieState();
	m->_video = new Video::QuickTimeDecoder();
	if (!m->_video->loadFile(path)) {
		delete m;
		return Datum(Common::String::format("error: failed to load movie file '%s'", path.toString().c_str()));
	}

	if (loc.size() == 4) {
		m->_rect = Common::Rect(loc[0], loc[1], loc[2], loc[3]);
		m->_video->setTargetSize(m->_rect.width(), m->_rect.height());
	} else {
		m->_rect = Common::Rect(loc[0], loc[1], loc[0] + m->_video->getWidth(), loc[1] + m->_video->getHeight());
	}
	m->_video->setOrigin(m->_rect.left, m->_rect.top);

	m->_widget = new PanoMovieWidget(m, g_director->getCurrentWindow()->getMacWindow(),
		m->_rect.left, m->_rect.top, m->_rect.width(), m->_rect.height(),
		g_director->getMacWindowManager());

	bool showPoster = false;
	for (uint i = 4; i < args.size(); i++) {
		if (args[i].asString().equalsIgnoreCase("ShowPoster"))
			showPoster = true;
	}

	if (!movies)
		movies = new PanoMovieMap();

	int id = s_nextMovieID++;
	(*movies)[id] = m;

	debugC(5, kDebugXObj, "PanoMovie: opened '%s' as %d at %d,%d,%d,%d", path.toString().c_str(), id,
		m->_rect.left, m->_rect.top, m->_rect.right, m->_rect.bottom);

	// Without ShowPoster nothing is drawn until the first Update, Idle or
	// MouseOver, but the movie is live at once: Safecracker opens a new
	// panorama on a 'cmov' hotspot and relies on the next MouseOver to show it
	if (showPoster)
		drawMovie(m);

	return Datum(id);
}

// Ask the QTVR controller what was clicked, as "type,id"
static Datum clickResult(PanoMovieState *m, int nodeBefore) {
	Video::QuickTimeDecoder *video = m->_video;

	if (video->getQTVRType() != Common::QuickTimeParser::QTVRType::PANORAMA)
		return Datum(0);

	int node = video->getCurrentNodeID();
	bool nodeChanged = node != nodeBefore;

	if (nodeChanged && !m->_nodeLeaveHandler.empty()) {
		g_lingo->push(Datum(nodeBefore));
		g_lingo->executeHandler(m->_nodeLeaveHandler, 1);
	}

	int hotspotID = video->getClickedHotspotID();
	if (!hotspotID)
		return Datum(nodeChanged ? Common::String::format("jump,%d", node) : Common::String("pan ,0"));

	const Common::QuickTimeParser::PanoHotSpot *hotspot = video->getHotSpotByID(hotspotID);
	if (nodeChanged || (hotspot && hotspot->type == MKTAG('l', 'i', 'n', 'k')))
		return Datum(Common::String::format("jump,%d", node));

	return Datum(Common::String::format("%s,%d", hotspot ? tag2str((uint32)hotspot->type) : "undf", hotspotID));
}

// Track the mouse until the button goes up, letting the controller pan or
// follow the hotspot.
static Datum trackClick(PanoMovieState *m, const Common::Point &pos) {
	Window *window = g_director->getCurrentWindow();
	int nodeBefore = m->_video->getCurrentNodeID();

	if (!m->_mouseDownHandler.empty()) {
		g_lingo->executeHandler(m->_mouseDownHandler);
		if (!m->_video)
			return Datum(0);
	}

	m->_video->handleMouseButton(true, pos.x - m->_rect.left, pos.y - m->_rect.top);

	while (true) {
		drawMovie(m);

		Common::Event event;
		bool released = false;
		while (g_system->getEventManager()->pollEvent(event)) {
			if (event.type == Common::EVENT_QUIT) {
				g_director->processEventQUIT();
				return Datum(0);
			}
			if (Common::isMouseEvent(event))
				event.mouse = window->getMousePos();
			m->_widget->processEvent(event);
			if (event.type == Common::EVENT_LBUTTONUP) {
				released = true;
				break;
			}
		}

		if (!released && !(g_system->getEventManager()->getButtonState() & Common::EventManager::LBUTTON)) {
			Common::Point p = window->getMousePos();
			m->_video->handleMouseButton(false, p.x - m->_rect.left, p.y - m->_rect.top);
			released = true;
		}

		if (released)
			break;

		LB::b_updateStage(0);
		g_director->delayMillis(10);
	}

	drawMovie(m);
	LB::b_updateStage(0);

	return clickResult(m, nodeBefore);
}

static Datum mouseOver(PanoMovieState *m) {
	Window *window = g_director->getCurrentWindow();
	Common::Point pos = window->getMousePos();

	if (!m->_rect.contains(pos))
		return Datum(0);

	m->_exitMouseOver = false;

	// Director may already have taken the mouseDown event off the queue
	if (g_system->getEventManager()->getButtonState() & Common::EventManager::LBUTTON)
		return trackClick(m, pos);

	while (true) {
		drawMovie(m);

		Common::Event event;
		while (g_system->getEventManager()->pollEvent(event)) {
			if (event.type == Common::EVENT_QUIT) {
				g_director->processEventQUIT();
				return Datum(0);
			}

			if (Common::isMouseEvent(event)) {
				pos = window->getMousePos();
				event.mouse = pos;
			}

			if (event.type == Common::EVENT_LBUTTONDOWN && m->_rect.contains(pos))
				return trackClick(m, pos);

			m->_widget->processEvent(event);

			if (!m->_rect.contains(pos))
				return Datum(0);
		}

		if (!m->_mouseOverHandler.empty()) {
			g_lingo->push(Datum(Common::String::format("%d,%d", pos.x, pos.y)));
			g_lingo->executeHandler(m->_mouseOverHandler, 1);
			if (!m->_video)
				return Datum(0);
		}

		if (m->_exitMouseOver) {
			m->_exitMouseOver = false;
			return Datum(0);
		}

		LB::b_updateStage(0);
		g_director->delayMillis(10);
	}
}

static Datum getProperty(PanoMovieState *m, const Common::String &prop) {
	Video::QuickTimeDecoder *video = m->_video;

	if (prop.equalsIgnoreCase("nodeID"))
		return Datum(video->getCurrentNodeID());
	if (prop.equalsIgnoreCase("hPanAngle"))
		return Datum(video->getPanAngle());
	if (prop.equalsIgnoreCase("vPanAngle"))
		return Datum(video->getTiltAngle());
	if (prop.equalsIgnoreCase("zoomAngle"))
		return Datum(video->getFOV());
	if (prop.equalsIgnoreCase("quality"))
		return Datum((int)video->getQuality());
	if (prop.equalsIgnoreCase("warpMode"))
		return Datum(video->getWarpMode());
	if (prop.equalsIgnoreCase("transitionMode"))
		return Datum(video->getTransitionMode());
	if (prop.equalsIgnoreCase("transitionSpeed"))
		return Datum(video->getTransitionSpeed());
	if (prop.equalsIgnoreCase("updateMode"))
		return Datum(video->getUpdateMode());
	if (prop.equalsIgnoreCase("mouseOverHandler"))
		return Datum(m->_mouseOverHandler);
	if (prop.equalsIgnoreCase("mouseDownHandler"))
		return Datum(m->_mouseDownHandler);
	if (prop.equalsIgnoreCase("nodeLeaveHandler"))
		return Datum(m->_nodeLeaveHandler);
	if (prop.equalsIgnoreCase("MovieRect") || prop.equalsIgnoreCase("WindowRect"))
		return Datum(Common::String::format("%d,%d,%d,%d", m->_rect.left, m->_rect.top, m->_rect.right, m->_rect.bottom));
	if (prop.equalsIgnoreCase("hotSpotID"))
		return Datum(video->getClickedHotspotID());

	warning("PanoMovie: STUB: get '%s'", prop.c_str());
	return Datum("");
}

static void setProperty(PanoMovieState *m, const Common::String &prop, const Datum &value) {
	Video::QuickTimeDecoder *video = m->_video;
	Common::String str = value.asString();

	if (prop.equalsIgnoreCase("nodeID"))
		video->goToNode(value.asInt());
	else if (prop.equalsIgnoreCase("hPanAngle"))
		video->setPanAngle(atof(str.c_str()));
	else if (prop.equalsIgnoreCase("vPanAngle"))
		video->setTiltAngle(atof(str.c_str()));
	else if (prop.equalsIgnoreCase("zoomAngle"))
		video->setFOV(atof(str.c_str()));
	else if (prop.equalsIgnoreCase("quality"))
		video->setQuality(atof(str.c_str()));
	else if (prop.equalsIgnoreCase("warpMode"))
		video->setWarpMode(value.asInt());
	else if (prop.equalsIgnoreCase("transitionMode"))
		video->setTransitionMode(str);
	else if (prop.equalsIgnoreCase("transitionSpeed"))
		video->setTransitionSpeed(atof(str.c_str()));
	else if (prop.equalsIgnoreCase("updateMode"))
		video->setUpdateMode(str);
	else if (prop.equalsIgnoreCase("mouseOverHandler"))
		m->_mouseOverHandler = str;
	else if (prop.equalsIgnoreCase("mouseDownHandler"))
		m->_mouseDownHandler = str;
	else if (prop.equalsIgnoreCase("nodeLeaveHandler"))
		m->_nodeLeaveHandler = str;
	else if (prop.equalsIgnoreCase("hotSpotID"))
		video->setClickedHotSpot(value.asInt());
	else
		warning("PanoMovie: STUB: set '%s' to '%s'", prop.c_str(), str.c_str());
}

static Datum directCommand(PanoMovieMap *movies, const Common::Array<Datum> &args) {
	if (args.size() < 3)
		return Datum("error: missing command");

	int id = args[1].asInt();
	if (!movies || !movies->contains(id))
		return Datum(Common::String::format("error: no movie %s", args[1].asString().c_str()));

	PanoMovieState *m = (*movies)[id];
	Common::String cmd = args[2].asString();

	if (cmd.equalsIgnoreCase("dispose")) {
		movies->erase(id);
		delete m;
		g_director->getCurrentWindow()->setDirty(true);
		return Datum("");
	}
	if (cmd.equalsIgnoreCase("Update")) {
		drawMovie(m);
		return Datum("");
	}
	if (cmd.equalsIgnoreCase("Idle")) {
		drawMovie(m);
		return Datum("");
	}
	if (cmd.equalsIgnoreCase("MouseOver"))
		return mouseOver(m);
	if (cmd.equalsIgnoreCase("ExitMouseOver")) {
		m->_exitMouseOver = true;
		return Datum("");
	}
	if (cmd.equalsIgnoreCase("MouseDown")) {
		Common::Point pos = g_director->getCurrentWindow()->getMousePos();
		if (!m->_rect.contains(pos))
			return Datum(0);
		return trackClick(m, pos);
	}
	if (cmd.equalsIgnoreCase("Get")) {
		if (args.size() < 4)
			return Datum("error: missing property");
		return getProperty(m, args[3].asString());
	}
	if (cmd.equalsIgnoreCase("Set")) {
		if (args.size() < 5)
			return Datum("error: missing property value");
		setProperty(m, args[3].asString(), args[4]);
		return Datum("");
	}

	warning("PanoMovie: STUB: command '%s'", cmd.c_str());
	return Datum("");
}

static Datum runXCMD(const char *name, PanoMovieMap *&movies, int nargs) {
	Common::Array<Datum> args;
	args.resize(nargs);
	for (int i = nargs - 1; i >= 0; i--)
		args[i] = g_lingo->pop();

	if (debugChannelSet(5, kDebugXObj)) {
		Common::String s;
		for (auto &a : args)
			s += a.asString(true) + " ";
		debugC(5, kDebugXObj, "%s(%s)", name, s.c_str());
	}

	Datum res;
	if (nargs >= 1 && args[0].asString().equalsIgnoreCase("OpenMovie"))
		res = openMovie(movies, args);
	else if (nargs >= 1 && args[0].asString().equalsIgnoreCase("Direct"))
		res = directCommand(movies, args);
	else
		res = Datum(Common::String::format("error: unknown %s call", name));

	g_lingo->_theResult = res;
	return res;
}

void PanoMovieXCMD::open(ObjectType type, const Common::Path &path) {
	g_lingo->initBuiltIns(panoBuiltins);
}

void PanoMovieXCMD::close(ObjectType type) {
	g_lingo->cleanupBuiltIns(panoBuiltins);
	disposeAll(s_panoMovies);
}

void PanoMovieXCMD::m_panoMovie(int nargs) {
	g_lingo->push(runXCMD("PanoMovie", s_panoMovies, nargs));
}

void PanoMovieXCMD::m_panoMovieCmd(int nargs) {
	runXCMD("PanoMovie", s_panoMovies, nargs);
}

void NavMovieXCMD::open(ObjectType type, const Common::Path &path) {
	g_lingo->initBuiltIns(navBuiltins);
}

void NavMovieXCMD::close(ObjectType type) {
	g_lingo->cleanupBuiltIns(navBuiltins);
	disposeAll(s_navMovies);
}

void NavMovieXCMD::m_navMovie(int nargs) {
	g_lingo->push(runXCMD("NavMovie", s_navMovies, nargs));
}

void NavMovieXCMD::m_navMovieCmd(int nargs) {
	runXCMD("NavMovie", s_navMovies, nargs);
}

void RunningInForegroundXFCN::open(ObjectType type, const Common::Path &path) {
	g_lingo->initBuiltIns(foregroundBuiltins);
}

void RunningInForegroundXFCN::close(ObjectType type) {
	g_lingo->cleanupBuiltIns(foregroundBuiltins);
}

void RunningInForegroundXFCN::m_runningInForeground(int nargs) {
	g_lingo->push(Datum("true"));
}

///////////////
// Widget
///////////////

PanoMovieWidget::PanoMovieWidget(PanoMovieState *movie, Graphics::MacWidget *parent, int x, int y, int w, int h, Graphics::MacWindowManager *wm) :
	Graphics::MacWidget(parent, x, y, w, h, wm, true), _movie(movie) {

	_priority = 10000; // We stay on top of everything
}

bool PanoMovieWidget::processEvent(Common::Event &event) {
	Video::QuickTimeDecoder *video = _movie->_video;
	const Common::Rect &r = _movie->_rect;

	switch (event.type) {
	case Common::EVENT_LBUTTONDOWN:
		video->handleMouseButton(true, event.mouse.x - r.left, event.mouse.y - r.top);
		return true;
	case Common::EVENT_LBUTTONUP:
		video->handleMouseButton(false, event.mouse.x - r.left, event.mouse.y - r.top);
		return true;
	case Common::EVENT_MOUSEMOVE:
		video->handleMouseMove(event.mouse.x - r.left, event.mouse.y - r.top);
		return true;
	case Common::EVENT_KEYDOWN:
	case Common::EVENT_KEYUP:
		video->handleKey(event.kbd, event.type == Common::EVENT_KEYDOWN);
		return true;
	default:
		return false;
	}
}

} // End of namespace Director
