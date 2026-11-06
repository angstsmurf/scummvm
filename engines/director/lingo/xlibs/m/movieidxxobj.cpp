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

#include "common/file.h"
#include "common/macresman.h"
#include "common/memstream.h"
#include "common/system.h"

#include "director/director.h"
#include "director/lingo/lingo.h"
#include "director/lingo/lingo-object.h"
#include "director/lingo/lingo-utils.h"
#include "director/lingo/xlibs/m/movieidxxobj.h"

/**************************************************
 *
 * USED IN:
 * Jewels of the Oracle - Win
 * Jewels of the Oracle - Mac
 *
 **************************************************/

/*
-- MovieIdx External Factory. 16/03/95 - AAF - New
MovieIdx
I     mNew --Creates a new instance of the XObject
X     mDispose --Disposes of XObject instance
SS    mMovieInfo, movieName --Returns movie information for a given name

-- MovieIdx, Movie Index XObject - aep 95.05.01
I mNew --  read map given file name
X mDispose --  dispose of map
SS mMovieInfo -- return info given name
 */

namespace Director {

const char *const MovieIdxXObj::xlibName = "MovieIdx";
const XlibFileDesc MovieIdxXObj::fileNames[] = {
	{ "MovieIdx",		nullptr }, // Jewels of the Oracle - Win
	{ "MovieIdx.XObj",	nullptr }, // Jewels of the Oracle - Mac
	{ nullptr,			nullptr },
};

static const MethodProto xlibMethods[] = {
	{ "new",				MovieIdxXObj::m_new,		 0, 0,	400 },
	{ "dispose",				MovieIdxXObj::m_dispose,		 0, 0,	400 },
	{ "movieInfo",				MovieIdxXObj::m_movieInfo,		 1, 1,	400 },
	{ nullptr, nullptr, 0, 0, 0 }
};

struct MovieIdxEntry {
	int cmvIndex;
	Common::String name;
	uint32 offset;
	uint32 length;
	int sound;
	int playEveryFrame;
};

static Common::Array<MovieIdxEntry> *movieIndex = nullptr;

static bool isEndName(const Common::String &name) {
	return name == "END";
}

static bool parseMacTable(const byte *data, uint32 size) {
	const uint32 recSize = 22;
	// Find the first offset where a run of well-formed records leads up to
	// the END record.
	for (uint32 start = 0; start + recSize <= size; start++) {
		Common::Array<MovieIdxEntry> entries;
		uint32 pos = start;
		bool ok = false;
		while (pos + recSize <= size) {
			const byte *r = data + pos;
			byte len = r[2];
			if (len == 0 || len > 9)
				break;
			bool valid = true;
			for (byte i = 0; i < len; i++) {
				if (r[3 + i] < 0x20 || r[3 + i] >= 0x7f)
					valid = false;
			}
			for (byte i = len; i < 9; i++) {
				if (r[3 + i] != 0)
					valid = false;
			}
			if (!valid || r[20] > 1 || r[21] > 1)
				break;
			MovieIdxEntry e;
			e.cmvIndex = READ_BE_UINT16(r);
			e.name = Common::String((const char *)r + 3, len);
			e.offset = READ_BE_UINT32(r + 12);
			e.length = READ_BE_UINT32(r + 16);
			e.sound = r[20];
			e.playEveryFrame = r[21];
			pos += recSize;
			if (isEndName(e.name)) {
				ok = true;
				break;
			}
			entries.push_back(e);
		}
		if (ok && entries.size() >= 2) {
			*movieIndex = entries;
			return true;
		}
	}
	return false;
}

static bool parseWinTable(const byte *data, uint32 size) {
	const uint32 recSize = 18;
	for (uint32 start = 0; start + 3 * recSize <= size; start++) {
		if (READ_LE_UINT32(data + start + 2) != 0xffff ||
			READ_LE_UINT32(data + start + recSize + 2) != 0xffff ||
			READ_LE_UINT32(data + start + 2 * recSize + 2) != 0xffff)
			continue;
		uint32 count = 0;
		uint32 pos = start;
		while (pos + recSize <= size && READ_LE_UINT32(data + pos + 2) == 0xffff) {
			count++;
			pos += recSize;
		}
		// Name pool, after a 0xffffffff marker
		uint32 names = pos;
		while (names + 4 < size && READ_LE_UINT32(data + names) != 0xffffffff)
			names++;
		names += 4;
		Common::Array<MovieIdxEntry> entries;
		for (uint32 i = 0; i < count && names < size; i++) {
			if (data[names] == 'p')
				names++;
			uint32 end = names;
			while (end < size && data[end])
				end++;
			const byte *r = data + start + i * recSize;
			MovieIdxEntry e;
			e.cmvIndex = READ_LE_UINT16(r);
			e.name = Common::String((const char *)data + names, end - names);
			e.offset = READ_LE_UINT32(r + 6);
			e.length = READ_LE_UINT32(r + 10);
			e.sound = READ_LE_UINT16(r + 14);
			e.playEveryFrame = READ_LE_UINT16(r + 16);
			names = end + 1;
			if (isEndName(e.name)) {
				*movieIndex = entries;
				return true;
			}
			entries.push_back(e);
		}
		return false;
	}
	return false;
}

static void loadMovieIndex(const Common::Path &path) {
	if (!movieIndex)
		movieIndex = new Common::Array<MovieIdxEntry>();
	movieIndex->clear();
	if (path.empty())
		return;

	Common::MacResManager resMan;
	if (resMan.open(path) && resMan.hasResFork()) {
		Common::MacResIDArray ids = resMan.getResIDArray(MKTAG('X', 'C', 'O', 'D'));
		for (uint i = 0; i < ids.size(); i++) {
			Common::SeekableReadStream *res = resMan.getResource(MKTAG('X', 'C', 'O', 'D'), ids[i]);
			if (!res)
				continue;
			uint32 size = res->size();
			byte *data = (byte *)malloc(size);
			res->read(data, size);
			delete res;
			bool found = parseMacTable(data, size);
			free(data);
			if (found)
				break;
		}
	} else {
		Common::File f;
		if (f.open(path)) {
			uint32 size = f.size();
			byte *data = (byte *)malloc(size);
			f.read(data, size);
			parseWinTable(data, size);
			free(data);
		}
	}

	if (movieIndex->empty())
		warning("MovieIdxXObj: No movie index found in '%s'", path.toString().c_str());
	else
		debugC(5, kDebugXObj, "MovieIdxXObj: Read %d movie index entries from '%s'", movieIndex->size(), path.toString().c_str());
}

MovieIdxXObject::MovieIdxXObject(ObjectType ObjectType) :Object<MovieIdxXObject>("MovieIdx") {
	_objType = ObjectType;
}

void MovieIdxXObj::open(ObjectType type, const Common::Path &path) {
	if (type == kXObj) {
		loadMovieIndex(path);
		MovieIdxXObject::initMethods(xlibMethods);
		MovieIdxXObject *xobj = new MovieIdxXObject(kXObj);
		g_lingo->exposeXObject(xlibName, xobj);
	}
}

void MovieIdxXObj::close(ObjectType type) {
	if (type == kXObj) {
		MovieIdxXObject::cleanupMethods();
		delete movieIndex;
		movieIndex = nullptr;
		g_lingo->_globalvars[xlibName] = Datum();
	}
}

void MovieIdxXObj::m_new(int nargs) {
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

void MovieIdxXObj::m_dispose(int nargs) {
	g_lingo->dropStack(nargs);
}

void MovieIdxXObj::m_movieInfo(int nargs) {
	Common::String name = g_lingo->pop().asString();

	if (!movieIndex) {
		g_lingo->push(Datum(""));
		return;
	}

	for (auto &e : *movieIndex) {
		if (e.name.equalsIgnoreCase(name)) {
			Common::String info = Common::String::format("%d,%u,%u,%d,%d",
				e.cmvIndex, e.offset, e.length, e.sound, e.playEveryFrame);
			debugC(5, kDebugXObj, "MovieIdxXObj::m_movieInfo(\"%s\"): %s", name.c_str(), info.c_str());
			g_lingo->push(Datum(info));
			return;
		}
	}

	debugC(5, kDebugXObj, "MovieIdxXObj::m_movieInfo(\"%s\"): not found", name.c_str());
	g_lingo->push(Datum(""));
}

}
