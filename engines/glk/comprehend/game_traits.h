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

#ifndef GLK_COMPREHEND_GAME_TRAITS_H
#define GLK_COMPREHEND_GAME_TRAITS_H

#include "common/scummsys.h"

// What the shared loaders and renderers need to know about each game, in one
// table (game_traits.cpp), instead of comparing the game id string at every
// place a release differs.

namespace Glk {
namespace Comprehend {

class ComprehendGame;

struct GameTraits {
	const char *id;
	ComprehendGame *(*create)();

	// Game data layout (game_data.cpp)

	// All strings live in the separate string files: the game file has no
	// string table of its own, and its addr_strings_end header field points
	// straight at the replace-word table (Talisman).
	bool stringsOnlyInFiles;
	// OO-Topos pads the first and fifth string bank with a leading empty
	// string so its string indices line up.
	bool padsStringBanks;
	// Apple II disk only: the number of empty strings to put ahead of the
	// string files, which are that many entries short at the front compared
	// with the DOS data the string indices were derived from.
	uint8 appleStringLead;
	// Apple II disk only: the string banks have no 4-byte header; the offset
	// index starts at byte 0 (The Coveted Mirror).
	bool appleStringBanksHeaderless;

	// Pictures

	// Apple II: the older Graphics Magician dialect, which never emits op15,
	// ends images on op7 and needs the full-screen fill clip.
	bool gmLegacyFormat;
	// Apple II: where the title picture starts in the T0 file, past the copy
	// protection and loader stub.
	uint16 appleTitleOffset;
	// Apple II: the title picture is drawn on a black screen, not a white one.
	bool appleTitleOnBlack;
	// Apple II: the persistent logo and hourglass panel to the right of every
	// picture (The Coveted Mirror).
	bool hasHourglassPanel;

};

// The traits for a game id, or nullptr if the id is not a known game.
const GameTraits *findGameTraits(const char *id);

} // namespace Comprehend
} // namespace Glk

#endif
