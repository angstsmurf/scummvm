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

#ifndef DIRECTOR_CHANNEL_H
#define DIRECTOR_CHANNEL_H

#include "director/cursor.h"
#include "director/sprite.h"

namespace Common {
struct KeyState;
}

namespace Graphics {
	struct Surface;
	class ManagedSurface;
	class MacWidget;
	class MacText;
}

namespace Director {

class Sprite;
class Cursor;
class Score;

class Channel {
public:
	Channel(Score *sc, Sprite *sp, int priority = 0);
	Channel(const Channel &channel);
	Channel& operator=(const Channel &channel);
	~Channel();

	DirectorPlotData getPlotData();
	const Graphics::Surface *getMask(bool forceMatte = false);
	const Graphics::Surface *getMaskColor();

	// The stored size, which is not always the size drawn; see getBbox().
	// A text or button widget may resize the sprite: Lingo reads the size
	// from before.
	inline int getWidth() { return _sprite->_reportedWidth >= 0 ? _sprite->_reportedWidth : _sprite->_width; };
	inline int getHeight() { return _sprite->_reportedHeight >= 0 ? _sprite->_reportedHeight : _sprite->_height; };
	inline Common::Point getPosition() { return _sprite->getPosition(); };
	// Return the area of screen to be used for drawing content.
	inline Common::Rect getBbox(bool unstretched = false) { return _sprite->getBbox(unstretched); };
	Common::Rect getRollOverBbox();

	bool isStretched();
	bool isDirty(Sprite *nextSprite = nullptr);
	bool isEmpty();
	bool isTextSprite();
	bool isActiveText();
	CollisionTest isMouseIn(const Common::Point &pos);
	bool isMatteIntersect(Channel *channel);
	bool isMatteBoxIntersect(Channel *channel);
	bool isMatteWithin(Channel *channel);
	bool isMatteBoxWithin(Channel *channel);
	bool isActiveVideo();
	bool isVideoDirectToStage();

	void setWidth(int w);
	void setHeight(int h);
	void setBbox(int l, int t, int r, int b);
	// The rect Lingo reads, which follows a new size only once it is drawn
	Common::Rect getLingoBbox() { return _hasLingoBbox ? _lingoBbox : getBbox(); }
	void keepLingoBbox();
	void setDirty();
	void setNeedsDraw() { _needsDraw = true; }
	void setPosition(int x, int y, bool force = false);
	void updateDragPosition(Sprite *nextSprite);
	void setCast(CastMemberID memberID);
	void setAutoType();
	void setClean(Sprite *nextSprite, bool partial = false);
	void setStretch(bool enabled);
	bool getEditable();
	void setEditable(bool editable);
	void replaceSprite(Sprite *nextSprite);
	void replaceWidget(CastMemberID previousCastId = CastMemberID(0, 0), bool force = false);
	bool updateWidget();
	void sizeSpriteFromWidget();
	bool needsSrcAlpha(const Graphics::ManagedSurface *srf) const;
	void updateTextCast();

	bool isTrail();

	void updateGlobalAttr();

	bool canKeepWidget(CastMemberID castId);
	bool canKeepWidget(Sprite *currentSprite, Sprite *nextSprite);

	bool dropsTypedKey(const Common::KeyState &kbd);

	int getMouseChar(int x, int y);
	int getMouseWord(int x, int y);
	int getMouseItem(int x, int y);
	int getMouseLine(int x, int y);

	void updateVideoTime();

	// used for film loops
	bool hasSubChannels();
	Common::Array<Channel> *getSubChannels();
	CastMemberID getSubChannelSound1();
	CastMemberID getSubChannelSound2();

	Common::String formatInfo();

public:
	Sprite *_sprite;
	Cursor _cursor;
	Graphics::MacWidget *_widget;

	bool _widgetDirty;
	bool _needsDraw;
	bool _visible;
	bool _hideFromStage; // Used in DT for hiding the channel from rendering
	uint _constraint;
	Graphics::ManagedSurface *_mask;
	// The colours of a deeper mask ink mask, laid out like _mask
	Graphics::ManagedSurface *_maskColor = nullptr;

	int _priority;

	// Used in digital movie sprites
	double _movieRate;
	int _movieTime;
	int _startTime;
	int _stopTime;

	// Used in film loops
	uint _filmLoopFrame;

	Common::Rect _rollOverBbox;
	Common::Rect _lastRenderedBbox;
	bool _hasLingoBbox;
	Common::Rect _lingoBbox;
	bool _lastTrail;

	// D2-D5: position a moveable sprite was dragged to, kept across frames
	bool _hasDragPosition;
	Common::Point _dragPosition;

	int _startFrame;
	int _endFrame;
	// the scriptInstanceList of the sprite: a Lingo list, so a script can
	// add(the scriptInstanceList of sprite n, obj) to the live list
	Datum _scriptInstanceList;
	Common::Array<Datum> &scriptInstances() { return _scriptInstanceList.u.farr->arr; }

private:
	Graphics::MacText *mouseTextWidget(const char *query);
	Graphics::ManagedSurface *getSurface();
	const Graphics::Surface *getSourceMask(bool forceMatte, int w = 0, int h = 0);
	void buildQuad(Graphics::ManagedSurface *srf, bool forceMatte);

	// The member warped to 'the quad of sprite' (D7), in the bbox, with
	// a mask of the pixels inside the quad
	Graphics::ManagedSurface *_quadSurface = nullptr;
	Graphics::ManagedSurface *_quadMask = nullptr;
	// What _quadSurface/_quadMask were last built from
	bool _quadValid = false;
	CastMemberID _widgetCastId;	// member the widget was built from
	float _quadKeyQuad[4][2];
	Common::Rect _quadKeyBbox;
	const Graphics::ManagedSurface *_quadKeySrf = nullptr;
	int _quadKeyInk = 0;
	bool _quadKeyMatte = false;
	Score *_score;
};

} // End of namespace Director

#endif
