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

#include "common/events.h"
#include "common/file.h"
#include "common/substream.h"
#include "common/system.h"
#include "graphics/cursorman.h"
#include "graphics/macgui/macwindow.h"
#include "video/qt_decoder.h"

#include "director/director.h"
#include "director/lingo/lingo.h"
#include "director/lingo/lingo-object.h"
#include "director/lingo/lingo-utils.h"
#include "director/movie.h"
#include "director/window.h"
#include "director/lingo/xlibs/q/qtcatmovieplayerxobj.h"

/**************************************************
 *
 * USED IN:
 * Jewels of the Oracle - Win
 * Jewels of the Oracle - Mac
 *
 **************************************************/

/*
-- CatPlayr External Factory. 16/03/95 - AAF - Changed to play concatenated movies
CatPlayr
IS         mNew, fileName --Creates a new instance of the XObject
X          mDispose --Disposes of XObject instance
IIIIIIIIIS mPlay, movieOffset, startTime, duration, interruptable, horizontal, vertical, hideCursor, everyFrame, siblingWindowName --Plays QT


-- QTCatMoviePlayer, QuickTime Player XObject - js 95.05.01
IS         mNew, fileName, -- Open movie and place at h,v
X          mDispose -- Dispose of movie and close file
XIIIIIIIIS mPlay, filesOffset, startTime, duration, interruptable, h, v, hideCursor, playEveryFrame, windowName --  Play movie from start to end

 */

/*
 * fileName is a .CMV file: QuickTime movies (each a 'moov' atom followed
 * by its 'mdat') concatenated one after the other. The MovieIdx XObject
 * gives the offset of each one. Their chunk offsets are absolute positions
 * in the .CMV file.
 *
 * mPlay blocks until the movie has finished (or is clicked through, if it
 * is interruptable), drawing it at h,v in the content area of the window
 * named windowName ("Stage" for the stage).
 */

namespace Director {

const char *const QTCatMoviePlayerXObj::xlibNames[] = {
	"CatPlayr",			// Jewels of the Oracle - Win
	"QTCatMov",			// Jewels of the Oracle - Mac
	nullptr
};
const XlibFileDesc QTCatMoviePlayerXObj::fileNames[] = {
	{ "CATPLAYR",				nullptr }, // Jewels of the Oracle - Win
	{ "QTCatMov",				nullptr }, // Jewels of the Oracle - Mac (XCOD resource name)
	{ "QTCatMoviePlayer.XObj",	nullptr }, // Jewels of the Oracle - Mac
	{ nullptr,					nullptr },
};

static const MethodProto xlibMethods[] = {
	{ "new",				QTCatMoviePlayerXObj::m_new,		 1, 1,	400 },
	{ "dispose",			QTCatMoviePlayerXObj::m_dispose,	 0, 0,	400 },
	{ "play",				QTCatMoviePlayerXObj::m_play,		 9, 9,	400 },
	{ nullptr, nullptr, 0, 0, 0 }
};

QTCatMoviePlayerXObject::QTCatMoviePlayerXObject(ObjectType ObjectType) :Object<QTCatMoviePlayerXObject>("CatPlayr") {
	_objType = ObjectType;
}

void QTCatMoviePlayerXObj::open(ObjectType type, const Common::Path &path) {
	if (type == kXObj) {
		QTCatMoviePlayerXObject::initMethods(xlibMethods);
		QTCatMoviePlayerXObject *xobj = new QTCatMoviePlayerXObject(kXObj);
		for (uint i = 0; xlibNames[i]; i++)
			g_lingo->exposeXObject(xlibNames[i], xobj);
	}
}

void QTCatMoviePlayerXObj::close(ObjectType type) {
	if (type == kXObj) {
		QTCatMoviePlayerXObject::cleanupMethods();
		for (uint i = 0; xlibNames[i]; i++)
			g_lingo->_globalvars[xlibNames[i]] = Datum();
	}
}

static QTCatMoviePlayerXObject *getMe() {
	return static_cast<QTCatMoviePlayerXObject *>(g_lingo->_state->me.u.obj);
}

void QTCatMoviePlayerXObj::m_new(int nargs) {
	Common::String fileName = g_lingo->pop().asString();
	QTCatMoviePlayerXObject *me = getMe();

	me->_path = findPath(fileName);
	if (me->_path.empty())
		warning("QTCatMoviePlayerXObj::m_new: Could not find movie file '%s'", fileName.c_str());
	else
		debugC(5, kDebugXObj, "QTCatMoviePlayerXObj::m_new: '%s'", me->_path.toString().c_str());

	g_lingo->push(g_lingo->_state->me);
}

void QTCatMoviePlayerXObj::m_dispose(int nargs) {
	g_lingo->dropStack(nargs);
	getMe()->_path = Common::Path();
}

// Returns the end of the movie starting at offset: the end of the first
// 'mdat' atom after its 'moov'. 0 on failure.
static uint32 findMovieEnd(Common::SeekableReadStream *stream, uint32 offset) {
	uint32 pos = offset;
	bool seenMoov = false;
	while (pos + 8 <= stream->size()) {
		stream->seek(pos);
		uint32 size = stream->readUint32BE();
		uint32 tag = stream->readUint32BE();
		if (size < 8 || pos + size > stream->size())
			return 0;
		pos += size;
		if (tag == MKTAG('m', 'o', 'o', 'v')) {
			if (seenMoov)
				return 0;
			seenMoov = true;
		} else if (tag == MKTAG('m', 'd', 'a', 't') && seenMoov) {
			return pos;
		}
	}
	return 0;
}

static Window *findWindow(const Common::String &name) {
	if (name.empty() || name.equalsIgnoreCase("stage"))
		return g_director->getStage();
	const Common::Array<Window *> *windowList = g_director->getWindowList();
	for (auto &w : *windowList) {
		if (w->getName().equalsIgnoreCase(name))
			return w;
	}
	return nullptr;
}

void QTCatMoviePlayerXObj::m_play(int nargs) {
	Common::String windowName = g_lingo->pop().asString();
	bool playEveryFrame = g_lingo->pop().asInt() != 0;
	bool hideCursor = g_lingo->pop().asInt() != 0;
	int v = g_lingo->pop().asInt();
	int h = g_lingo->pop().asInt();
	bool interruptable = g_lingo->pop().asInt() != 0;
	int duration = g_lingo->pop().asInt();
	int startTime = g_lingo->pop().asInt();
	uint32 offset = (uint32)g_lingo->pop().asInt();

	QTCatMoviePlayerXObject *me = getMe();

	debugC(5, kDebugXObj, "QTCatMoviePlayerXObj::m_play(%u, %d, %d, %d, %d, %d, %d, %d, \"%s\") on '%s'",
		offset, startTime, duration, interruptable, h, v, hideCursor, playEveryFrame, windowName.c_str(),
		me->_path.toString().c_str());

	Common::File *file = new Common::File();
	if (me->_path.empty() || !file->open(me->_path)) {
		warning("QTCatMoviePlayerXObj::m_play: Could not open '%s'", me->_path.toString().c_str());
		delete file;
		g_lingo->push(Datum(0));
		return;
	}

	uint32 end = findMovieEnd(file, offset);
	if (!end) {
		warning("QTCatMoviePlayerXObj::m_play: No movie at offset %u in '%s'", offset, me->_path.toString().c_str());
		delete file;
		g_lingo->push(Datum(0));
		return;
	}

	Video::QuickTimeDecoder *video = new Video::QuickTimeDecoder();
	video->setChunkBeginOffset(offset);
	if (!video->loadStream(new Common::SeekableSubReadStream(file, offset, end, DisposeAfterUse::YES))) {
		warning("QTCatMoviePlayerXObj::m_play: Could not load movie at offset %u in '%s'", offset, me->_path.toString().c_str());
		delete video;
		g_lingo->push(Datum(0));
		return;
	}

	if (g_director->_pixelformat.bytesPerPixel == 1)
		video->setDitheringPalette(g_director->getPalette());

	// startTime and duration are in the movie's time scale; duration -1
	// plays to the end.
	uint32 timeScale = video->getTimeScale();
	if (startTime > 0 && timeScale)
		video->seek(Audio::Timestamp(0, startTime, timeScale));
	if (duration >= 0 && timeScale)
		video->setEndTime(Audio::Timestamp(0, startTime + duration, timeScale));

	Common::Point origin(h, v);
	Window *window = findWindow(windowName);
	if (!window) {
		warning("QTCatMoviePlayerXObj::m_play: No window named '%s', using the stage", windowName.c_str());
		window = g_director->getStage();
	}
	const Common::Rect &inner = window->getMacWindow()->getInnerDimensions();
	origin.x += inner.left;
	origin.y += inner.top;

	if (hideCursor)
		CursorMan.showMouse(false);

	video->start();

	Graphics::Surface *converted = nullptr;
	Graphics::Surface lastFrame;
	const Common::Rect screen(g_system->getWidth(), g_system->getHeight());
	Common::Event event;
	bool keepPlaying = true;
	while (keepPlaying && !video->endOfVideo()) {
		bool quit = false;
		while (g_director->pollModalEvent(event, quit)) {
			switch (event.type) {
			case Common::EVENT_LBUTTONDOWN:
			case Common::EVENT_RBUTTONDOWN:
			case Common::EVENT_KEYDOWN:
				if (interruptable)
					keepPlaying = false;
				break;
			default:
				break;
			}
		}
		if (quit || !keepPlaying)
			break;

		if (video->needsUpdate()) {
			const Graphics::Surface *frame = video->decodeNextFrame();
			if (frame) {
				if (frame->format != g_director->_pixelformat) {
					if (converted) {
						converted->free();
						delete converted;
					}
					converted = frame->convertTo(g_director->_pixelformat, video->getPalette());
					frame = converted;
				}
				lastFrame.free();
				lastFrame.copyFrom(*frame);
				Common::Rect dst(origin.x, origin.y, origin.x + frame->w, origin.y + frame->h);
				dst.clip(screen);
				if (!dst.isEmpty()) {
					g_director->_wm->copyRectToScreen(
						frame->getBasePtr(dst.left - origin.x, dst.top - origin.y), frame->pitch,
						dst.left, dst.top, dst.width(), dst.height());
				}
			}
		}
		g_system->updateScreen();
		g_director->delayMillis(10);
	}

	if (converted) {
		converted->free();
		delete converted;
	}
	video->close();
	delete video;

	if (hideCursor)
		CursorMan.showMouse(true);

	// QuickTime leaves the last frame on screen until Director paints over
	// it. Keep it in the window, so that a redraw before the game updates
	// the stage doesn't flash the window's old (often black) contents.
	if (lastFrame.getPixels()) {
		Graphics::ManagedSurface *surface = window->getSurface();
		Common::Rect dst(h, v, h + lastFrame.w, v + lastFrame.h);
		dst.clip(Common::Rect(surface->w, surface->h));
		if (!dst.isEmpty())
			surface->copyRectToSurface(lastFrame, dst.left, dst.top,
				Common::Rect(dst.left - h, dst.top - v, dst.right - h, dst.bottom - v));
		window->getMacWindow()->setContentDirty(true);
		lastFrame.free();
	}

	g_lingo->push(Datum(0));
}

}
