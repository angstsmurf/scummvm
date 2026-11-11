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

#ifndef DIRECTOR_SAVED_ARCHIVE_H
#define DIRECTOR_SAVED_ARCHIVE_H

#include "common/archive.h"
#include "common/array.h"
#include "common/hash-str.h"
#include "common/hashmap.h"
#include "common/path.h"

namespace Common {
class SeekableReadStream;
class WriteStream;
}

namespace Director {

/**
 * The movies and files the game wrote, found ahead of the game files.
 *
 * Each file is keyed by the name (or, for a movie saved over its own file,
 * the path) a lookup uses. It can be in up to three places, and the first
 * one it is in wins:
 *  - open:    changes to a cast library that is still open, in memory
 *  - session: a movie kept in memory for the rest of the session
 *  - disk:    a save file, used only while it is trusted (see isTrusted())
 */
class SavedArchive : public Common::Archive {
public:
	SavedArchive(const Common::String &target);

	// The instance registered with SearchMan
	static SavedArchive *get();

	bool hasFile(const Common::Path &path) const override;
	int listMembers(Common::ArchiveMemberList &list) const override;
	const Common::ArchiveMemberPtr getMember(const Common::Path &path) const override;
	Common::SeekableReadStream *createReadStreamForMember(const Common::Path &path) const override;

	// Register a save file found on disk
	bool _addFile(const Common::String &path);
	// A save was just written: register it and trust it without checking
	void addWrittenFile(const Common::String &fileName);
	// Keep a save in memory instead, for the rest of the session. It takes
	// the place of any save on disk for the same file until one is written.
	void addSessionFile(const Common::String &fileName, const byte *data, uint32 size);
	// Keep the changes to an open cast library in memory, so the next movie
	// that uses the same file gets them, like Director, which keeps the
	// library open across the movie switch. It takes the place of the
	// session and disk saves until a save is made or the library closes.
	void addOpenFile(const Common::String &fileName, const Common::Path &path, const byte *data, uint32 size);
	// Close every library that kept changes but is not among the files
	// still in use, dropping its changes
	void closeOpenFiles(const Common::Array<Common::Path> &inUse);

	static Common::String encodeKey(const Common::String &key);
	static Common::String decodeKey(const Common::String &name);

	// The stamp appended to every saved movie, after the RIFX data, which
	// readers ignore since they go by the size in the header:
	//   version, original size, original CRC-32, trailer size, 'SVMS'
	// The original is the game file the save stands in for, as found
	// beneath the saves; its size is kNoOriginal if there was none.
	// An overlay save is stamped with the game file it was made from.
	struct Stamp {
		uint32 version = 0;
		uint32 origSize = 0;
		uint32 origCrc = 0;
	};
	static const uint32 kNoOriginal = 0xFFFFFFFF;
	// Stamp the original the save at 'key' would stand in for
	static Stamp stampFor(const Common::String &key);
	static void writeStamp(Common::WriteStream *stream, const Stamp &stamp);

	// The game file beneath the saves, read into memory (free() it), or nullptr
	static byte *readOriginal(const Common::String &key, uint32 &size);

	// An overlay save holds only what changed in a game file:
	//   'SVMO', base key length, base key, base size, base CRC-32,
	//   movie size, patch count, then each patch: offset, length, data
	// followed by the stamp. All numbers are big endian. The movie is the
	// base with the patches applied: a new RIFX header and imap, and a
	// tail after the base with the changed resources and a new mmap.
	static const uint32 kOverlayTag = MKTAG('S', 'V', 'M', 'O');
	static void writeOverlayHeader(Common::WriteStream *stream, const Common::String &baseKey, uint32 baseSize, uint32 baseCrc, uint32 movieSize, uint32 patchCount);
	// The game file an existing save of 'key' was made from: its base if it
	// is an overlay, otherwise the game file at 'key' itself
	Common::String baseKeyFor(const Common::String &key) const;

private:
	// What the header and stamp of a save say
	struct SaveInfo {
		bool overlay = false;
		Common::String baseKey;
		uint32 baseSize = 0;
		uint32 baseCrc = 0;
		uint32 movieSize = 0;
		uint32 patchCount = 0;
		bool stamped = false;
		Stamp stamp;
	};
	// Read an overlay's header, leaving the stream at its first patch
	static bool readOverlayHeader(Common::SeekableReadStream *stream, SaveInfo &info);
	static SaveInfo readSaveInfo(Common::SeekableReadStream *stream);
	Common::SeekableReadStream *buildOverlay(Common::SeekableReadStream *save) const;

	enum Trust {
		kTrustUnknown,
		kTrusted,
		kUntrusted
	};

	struct Entry {
		// The save file on disk, if there is one
		Common::String diskName;
		mutable Trust trust = kTrustUnknown;
		bool inSession = false;
		Common::Array<byte> sessionData;
		bool isOpen = false;
		Common::Path openPath;
		Common::Array<byte> openData;

		// The data kept in memory, which takes the place of the disk save
		const Common::Array<byte> *memoryData() const {
			return isOpen ? &openData : inSession ? &sessionData : nullptr;
		}
	};
	typedef Common::HashMap<Common::String, Entry, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> EntryMap;

	// Whether a lookup of the entry finds anything
	bool isVisible(const Common::String &key, const Entry &entry) const;
	// The save for 'key', from memory or disk
	Common::SeekableReadStream *openSave(const Common::String &key) const;
	Common::String keyFor(const Common::String &fileName) const;
	bool isTrusted(const Common::String &key, const Entry &entry) const;

	EntryMap _entries;
	Common::String _target;
};

} // End of namespace Director

#endif
