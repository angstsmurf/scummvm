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

#ifndef DIRECTOR_LINGO_XLIBS_PANOMOVIE_H
#define DIRECTOR_LINGO_XLIBS_PANOMOVIE_H

#include "graphics/macgui/macwidget.h"

namespace Video {
class QuickTimeDecoder;
}

namespace Director {

struct PanoMovieState;

class PanoMovieWidget : public Graphics::MacWidget {
public:
	PanoMovieWidget(PanoMovieState *movie, Graphics::MacWidget *parent, int x, int y, int w, int h, Graphics::MacWindowManager *wm);

	bool processEvent(Common::Event &event) override;

	PanoMovieState *_movie;
};

struct PanoMovieState {
	~PanoMovieState();

	Video::QuickTimeDecoder *_video = nullptr;
	PanoMovieWidget *_widget = nullptr;
	Common::Rect _rect;
	bool _exitMouseOver = false;
	// The picture rendered by an update in "offscreenOnly" mode, shown by
	// the next update in "fromOffscreen" mode
	Graphics::Surface _offscreen;

	Common::String _mouseOverHandler;
	Common::String _mouseDownHandler;
	Common::String _nodeLeaveHandler;
};

namespace PanoMovieXCMD {

extern const char *const xlibName;
extern const XlibFileDesc fileNames[];

void open(ObjectType type, const Common::Path &path);
void close(ObjectType type);

void m_panoMovie(int nargs);
void m_panoMovieCmd(int nargs);

} // End of namespace PanoMovieXCMD

namespace NavMovieXCMD {

extern const char *const xlibName;
extern const XlibFileDesc fileNames[];

void open(ObjectType type, const Common::Path &path);
void close(ObjectType type);

void m_navMovie(int nargs);
void m_navMovieCmd(int nargs);

} // End of namespace NavMovieXCMD

} // End of namespace Director

#endif
