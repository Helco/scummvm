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

namespace Graphics {
	struct Surface;
	class ManagedSurface;
	class MacWidget;
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

	bool isBboxDeterminedByChannel();
	Common::Point getPosition(bool unstretched = false);
	// Return the area of screen to be used for drawing content.
	Common::Rect getBbox(bool unstretched = false);

	bool isStretched();
	bool isDirty(Sprite *nextSprite = nullptr);
	bool isEmpty();
	bool isActiveText();
	bool isMouseIn(const Common::Point &pos);
	bool isMatteIntersect(Channel *channel);
	bool isMatteWithin(Channel *channel);
	bool isActiveVideo();
	bool isVideoDirectToStage();

	void setWidth(int w);
	void setHeight(int h);
	void setBbox(int l, int t, int r, int b);
	void setPosition(int x, int y, bool force = false);
	void setCast(CastMemberID memberID);
	void setClean(Sprite *nextSprite, bool partial = false);
	bool getEditable();
	void setEditable(bool editable);
	void replaceSprite(Sprite *nextSprite);
	void replaceWidget(CastMemberID previousCastId = CastMemberID(0, 0), bool force = false);
	bool updateWidget();
	void updateTextCast();

	bool isTrail();

	void updateGlobalAttr();

	bool canKeepWidget(CastMemberID castId);
	bool canKeepWidget(Sprite *currentSprite, Sprite *nextSprite);

	int getMouseChar(int x, int y);
	int getMouseWord(int x, int y);
	int getMouseItem(int x, int y);
	int getMouseLine(int x, int y);

	void updateVideoTime();

	// used for film loops
	bool hasSubChannels();
	Common::Array<Channel> *getSubChannels();

public:
	Sprite *_sprite;
	Cursor _cursor;
	Graphics::MacWidget *_widget;

	bool _dirty;
	bool _visible;
	uint _constraint;
	Graphics::ManagedSurface *_mask;

	int _priority;

	// These fields are used for tracking overrides for the position, width and height of
	// the channel, as available in Lingo.
	// Basically, if the sprite -isn't- in puppet mode, Lingo will allow you to set
	// these values to whatever, but the sprite on the screen will still be the position and
	// dimensions from the score frame.
	// If you set puppet mode, the sprite on the screen will use these values instead.
	// If you set puppet mode, change things, then disable puppet mode, it will revert to the score.
	Common::Point _currentPoint;
	int _width;
	int _height;

	// Used in digital movie sprites
	double _movieRate;
	int _movieTime;
	int _startTime;
	int _stopTime;

	// Used in film loops
	uint _filmLoopFrame;

private:
	Graphics::ManagedSurface *getSurface();
	Score *_score;
};

} // End of namespace Director

#endif
