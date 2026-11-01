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

#include "common/language.h"
#include "engines/game.h"

namespace Glk {
namespace Comprehend {

const PlainGameDescriptor COMPREHEND_GAME_LIST[] = {
	{"transylvania", "Transylvania"},
	{"crimsoncrown", "Crimson Crown"},
	{"ootopos", "OO-Topos"},

	{"covetedmirror", "The Coveted Mirror"},
	{"transylvaniav2", "Transylvania (V2)"},
	{"talisman", "Talisman: Challenging the Sands of Time"},

	{nullptr, nullptr}
};

struct ComprehendDetectionEntry {
	const char *const _gameId;
	const char *const _filename;
	const char *const _md5;
};

const ComprehendDetectionEntry COMPREHEND_GAMES[] = {
	{ "transylvania", "tr.gda", "22e08633eea02ceee49b909dfd982d22" },
	{ "crimsoncrown", "cc1.gda", "f2abf019675ac5c9bcfd81032bc7787b" },
	{ "ootopos", "g0", "56460c1ee669c253607534155d7e9db4" },

	{ "transylvaniav2", "g0", "384cbf0cd50888310fd33574e6baf880" },
	{ "talisman", "g0", "35770d4815e610b5252e3fcd9f11def3" },

	{nullptr, nullptr, nullptr}
};

/**
 * Apple II disk images, by the MD5 of the whole file. Each game is listed by the
 * side it boots from; the engine finds the other sides in the same directory.
 */
struct ComprehendDiskEntry {
	const char *const _gameId;
	const char *const _md5;
};

const ComprehendDiskEntry COMPREHEND_DISKS[] = {
	{ "transylvania", "925e76655c6710adb80e362b20208b01" },
	{ "crimsoncrown", "e74cc9988f4800bab960fed92a24d717" },
	{ "ootopos", "54d8fad3084e513aee4db06a5464e07e" },
	{ "talisman", "819c09ad7a5b2233af2951bef8d704da" },
	{ "covetedmirror", "64e66ce16594cc745852c3abddbaf19c" },

	{nullptr, nullptr}
};

} // End of namespace Comprehend
} // End of namespace Glk
