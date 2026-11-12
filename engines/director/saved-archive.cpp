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

#include "common/crc.h"
#include "common/memstream.h"
#include "common/ptr.h"
#include "common/savefile.h"

#include "director/director.h"
#include "director/saved-archive.h"
#include "director/util.h"

namespace Director {

SavedArchive::SavedArchive(const Common::String &target) {
	_target = target;
	Common::StringArray saveFileList = g_engine->getSaveFileManager()->listSavefiles(target + "-*");

	debugC(3, kDebugLoading, "DirectorEngine:: loadSaveFiles: Loading save files");
	for (auto &saveFileName : saveFileList) {
		addFile(saveFileName);
	}
}

SavedArchive *SavedArchive::get() {
	return (SavedArchive *)SearchMan.getArchive(kSavedFilesArchive);
}

bool SavedArchive::isVisible(const Common::String &key, const Entry &entry) const {
	if (entry.memoryData())
		return true;
	return !entry.diskName.empty() && isTrusted(key, entry);
}

bool SavedArchive::hasFile(const Common::Path &path) const {
	Common::String key = path.toString();
	EntryMap::const_iterator it = _entries.find(key);
	return it != _entries.end() && isVisible(key, it->_value);
}

int SavedArchive::listMembers(Common::ArchiveMemberList &list) const {
	int count = 0;

	for (EntryMap::const_iterator i = _entries.begin(); i != _entries.end(); ++i) {
		if (!isVisible(i->_key, i->_value))
			continue;
		list.push_back(Common::ArchiveMemberList::value_type(new Common::GenericArchiveMember(i->_key, *this)));
		++count;
	}

	return count;
}

const Common::ArchiveMemberPtr SavedArchive::getMember(const Common::Path &path) const {
	if (!hasFile(path))
		return Common::ArchiveMemberPtr();

	return Common::ArchiveMemberPtr(new Common::GenericArchiveMember(path, *this));
}

Common::SeekableReadStream *SavedArchive::openSave(const Common::String &key) const {
	EntryMap::const_iterator it = _entries.find(key);
	if (it == _entries.end())
		return nullptr;

	const Entry &entry = it->_value;
	if (const Common::Array<byte> *data = entry.memoryData())
		return new Common::MemoryReadStream(data->data(), data->size());

	if (entry.diskName.empty() || !isTrusted(key, entry))
		return nullptr;
	return g_engine->getSaveFileManager()->openForLoading(entry.diskName);
}

Common::SeekableReadStream *SavedArchive::createReadStreamForMember(const Common::Path &path) const {
	Common::String key = path.toString();
	Common::SeekableReadStream *stream = openSave(key);
	if (!stream)
		return nullptr;

	if (stream->size() >= 4 && stream->readUint32BE() == kOverlayTag) {
		Common::SeekableReadStream *result = buildOverlay(stream);
		delete stream;
		if (!result)
			warning("SavedArchive: can't read the saved movie for '%s'", key.c_str());
		return result;
	}
	stream->seek(0);

	// Buffer the save file data into memory.
	// We have to do this because openForLoading will return a stream backed by a
	// FSNode, and subsequently calling openForSaving on the same file will cause
	// that stream to become completely empty. RIFXArchive::writeToFile needs to
	// be able to read from that stream while saving! A session save's buffer
	// is replaced when the movie is saved again.
	byte *data = (byte *)calloc(stream->size(), sizeof(byte));
	stream->read(data, stream->size());
	Common::MemoryReadStream *result = new Common::MemoryReadStream(data, stream->size(), DisposeAfterUse::YES);
	delete stream;
	return result;
}

// A save file name can't hold a path, so a movie saved over its own file
// stores the path with '/' escaped as "%2F" (and '%' as "%25").
Common::String SavedArchive::encodeKey(const Common::String &key) {
	if (!key.contains('/'))
		return key;

	Common::String res;
	for (uint i = 0; i < key.size(); i++) {
		if (key[i] == '/')
			res += "%2F";
		else if (key[i] == '%')
			res += "%25";
		else
			res += key[i];
	}
	return res;
}

Common::String SavedArchive::decodeKey(const Common::String &name) {
	Common::String res;
	for (uint i = 0; i < name.size(); i++) {
		if (name[i] == '%' && i + 2 < name.size()) {
			Common::String code = name.substr(i + 1, 2);
			if (code.equalsIgnoreCase("2F")) {
				res += '/';
				i += 2;
				continue;
			}
			if (code == "25") {
				res += '%';
				i += 2;
				continue;
			}
		}
		res += name[i];
	}
	return res;
}

Common::String SavedArchive::keyFor(const Common::String &fileName) const {
	// Derive the original file name from the save file name
	// Save files are named target_name-save_filename
	Common::String name = fileName.substr(_target.size() + 1);
	if (name.contains("%2F") || name.contains("%2f")) {
		// A movie saved over its own file, keyed by its path
		return decodeKey(name);
	}
	// Normalise it with convertPath.
	return convertPath(name);
}

bool SavedArchive::addFile(const Common::String &fileName) {
	Common::String key = keyFor(fileName);
	Entry &entry = _entries[key];
	if (!entry.diskName.empty())
		return false;

	debugC(3, kDebugLoading, "Found save file: %s -> %s", fileName.c_str(), key.c_str());
	entry.diskName = fileName;
	return true;
}

void SavedArchive::addWrittenFile(const Common::String &fileName) {
	addFile(fileName);
	Entry &entry = _entries[keyFor(fileName)];
	entry.trust = kTrusted;
	entry.inSession = false;
	entry.sessionData.clear();
	entry.isOpen = false;
	entry.openData.clear();
}

void SavedArchive::addSessionFile(const Common::String &fileName, const byte *data, uint32 size) {
	Entry &entry = _entries[keyFor(fileName)];
	entry.inSession = true;
	entry.sessionData = Common::Array<byte>(data, size);
	entry.isOpen = false;
	entry.openData.clear();
}

void SavedArchive::addOpenFile(const Common::String &fileName, const Common::Path &path, const byte *data, uint32 size) {
	Entry &entry = _entries[keyFor(fileName)];
	entry.isOpen = true;
	entry.openPath = path;
	entry.openData = Common::Array<byte>(data, size);
}

void SavedArchive::closeOpenFiles(const Common::Array<Common::Path> &inUse) {
	Common::Array<Common::String> emptied;
	for (EntryMap::iterator i = _entries.begin(); i != _entries.end(); ++i) {
		Entry &entry = i->_value;
		if (!entry.isOpen)
			continue;

		bool used = false;
		for (auto &path : inUse) {
			if (path == entry.openPath) {
				used = true;
				break;
			}
		}
		if (used)
			continue;

		debugC(3, kDebugSaving, "SavedArchive::closeOpenFiles: Dropped the changes to %s", i->_key.c_str());
		// Have the library read from its file again the next time
		g_director->markArchiveStale(entry.openPath);
		entry.isOpen = false;
		entry.openData.clear();
		if (!entry.inSession && entry.diskName.empty())
			emptied.push_back(i->_key);
	}
	for (auto &key : emptied)
		_entries.erase(key);
}

// Bump this whenever the movie writer changes its output; raise
// kMinTrustedVersion to it when the change fixes saves that broke a game,
// so saves written before the fix are ignored and the game files used
// instead. 1: unchanged resources kept verbatim.
// 2: overlays, holding only the changes to the game file.
static const uint32 kSavedMovieVersion = 2;
static const uint32 kMinTrustedVersion = 1;
static const uint32 kStampTag = MKTAG('S', 'V', 'M', 'S');
static const uint32 kStampSize = 5 * 4;

SavedArchive::Stamp SavedArchive::stampFor(const Common::String &key) {
	Stamp stamp;
	stamp.version = kSavedMovieVersion;
	stamp.origSize = kNoOriginal;

	uint32 size;
	byte *data = readOriginal(key, size);
	if (!data)
		return stamp;
	stamp.origSize = size;
	stamp.origCrc = Common::CRC32().crcFast(data, size);
	free(data);
	return stamp;
}

byte *SavedArchive::readOriginal(const Common::String &key, uint32 &size) {
	size = 0;
	if (key.empty())
		return nullptr;

	// The original is whatever a lookup would find if there were no save
	Common::ScopedPtr<Common::SeekableReadStream> orig(SearchMan.createReadStreamForMemberNext(Common::Path(key), get()));
	if (!orig)
		return nullptr;

	size = orig->size();
	byte *data = (byte *)malloc(size ? size : 1);
	if (!data || orig->read(data, size) != size) {
		free(data);
		size = 0;
		return nullptr;
	}
	return data;
}

void SavedArchive::writeOverlayHeader(Common::WriteStream *stream, const Common::String &baseKey, uint32 baseSize, uint32 baseCrc, uint32 movieSize, uint32 patchCount) {
	stream->writeUint32BE(kOverlayTag);
	stream->writeUint32BE(baseKey.size());
	stream->writeString(baseKey);
	stream->writeUint32BE(baseSize);
	stream->writeUint32BE(baseCrc);
	stream->writeUint32BE(movieSize);
	stream->writeUint32BE(patchCount);
}

bool SavedArchive::readOverlayHeader(Common::SeekableReadStream *stream, SaveInfo &info) {
	stream->seek(0);
	if (stream->readUint32BE() != kOverlayTag)
		return false;
	uint32 keyLen = stream->readUint32BE();
	if (keyLen > 4096 || stream->eos())
		return false;
	info.baseKey = stream->readString(0, keyLen);
	info.baseSize = stream->readUint32BE();
	info.baseCrc = stream->readUint32BE();
	info.movieSize = stream->readUint32BE();
	info.patchCount = stream->readUint32BE();
	return !stream->eos() && !stream->err() && !info.baseKey.empty();
}

SavedArchive::SaveInfo SavedArchive::readSaveInfo(Common::SeekableReadStream *stream) {
	SaveInfo info;
	info.overlay = readOverlayHeader(stream, info);
	if (!info.overlay)
		info = SaveInfo();

	if (stream->size() >= kStampSize) {
		stream->seek(stream->size() - 8);
		uint32 stampSize = stream->readUint32BE();
		if (stream->readUint32BE() == kStampTag && stampSize >= kStampSize && stampSize <= stream->size()) {
			stream->seek(stream->size() - stampSize);
			info.stamp.version = stream->readUint32BE();
			info.stamp.origSize = stream->readUint32BE();
			info.stamp.origCrc = stream->readUint32BE();
			info.stamped = true;
		}
	}
	return info;
}

// The movie an overlay save stands for: its game file with the patches
Common::SeekableReadStream *SavedArchive::buildOverlay(Common::SeekableReadStream *save) const {
	SaveInfo info;
	if (!readOverlayHeader(save, info))
		return nullptr;

	uint32 size;
	byte *base = readOriginal(info.baseKey, size);
	if (!base)
		return nullptr;
	if (size != info.baseSize || Common::CRC32().crcFast(base, size) != info.baseCrc || info.movieSize < info.baseSize) {
		free(base);
		return nullptr;
	}

	uint32 movieSize = info.movieSize;
	byte *data = (byte *)realloc(base, movieSize ? movieSize : 1);
	if (!data) {
		free(base);
		return nullptr;
	}
	memset(data + info.baseSize, 0, movieSize - info.baseSize);

	for (uint32 i = 0; i < info.patchCount; i++) {
		uint32 offset = save->readUint32BE();
		uint32 len = save->readUint32BE();
		if (save->eos() || offset > movieSize || len > movieSize - offset || save->read(data + offset, len) != len) {
			free(data);
			return nullptr;
		}
	}
	return new Common::MemoryReadStream(data, movieSize, DisposeAfterUse::YES);
}

Common::String SavedArchive::baseKeyFor(const Common::String &key) const {
	Common::ScopedPtr<Common::SeekableReadStream> save(openSave(key));
	SaveInfo info;
	if (save && readOverlayHeader(save.get(), info))
		return info.baseKey;
	return key;
}

void SavedArchive::writeStamp(Common::WriteStream *stream, const Stamp &stamp) {
	stream->writeUint32BE(kSavedMovieVersion);
	stream->writeUint32BE(stamp.origSize);
	stream->writeUint32BE(stamp.origCrc);
	stream->writeUint32BE(kStampSize);
	stream->writeUint32BE(kStampTag);
}

// A save that stands in for a game file is only used while it is known to
// be good: written by a writer without known faults, and over the same
// game file it was made from. Anything else would play differently from
// the original, e.g. a Safecracker MAIN rewritten by an older writer lost
// its transitions. Rejected saves are left on disk.
bool SavedArchive::isTrusted(const Common::String &key, const Entry &entry) const {
	if (entry.trust != kTrustUnknown)
		return entry.trust == kTrusted;

	const Common::String &fileName = entry.diskName;
	Common::String reason;

	Common::ScopedPtr<Common::SeekableReadStream> save(g_engine->getSaveFileManager()->openForLoading(fileName));
	SaveInfo info;
	if (save)
		info = readSaveInfo(save.get());
	// An overlay is checked against the game file it was made from
	const Common::String &baseKey = info.overlay ? info.baseKey : key;
	const Stamp &stamp = info.stamp;

	if (!save) {
		reason = "it can't be read";
	} else if (info.overlay && !info.stamped) {
		reason = "it is damaged";
	} else if (!info.stamped) {
		// Saves keyed by path were only made by unstamped development
		// builds, some with the faulty writer. Older saves, keyed by
		// name, are kept: they hold players' progress, e.g. save slots.
		if (fileName.contains("%2F") || fileName.contains("%2f"))
			reason = "it was made by an older version of ScummVM";
	} else if (stamp.version < kMinTrustedVersion) {
		reason = "it was made by an older version of ScummVM";
	} else if (info.overlay || stamp.origSize != kNoOriginal) {
		Stamp now = stampFor(baseKey);
		if (info.overlay && now.origSize == kNoOriginal)
			reason = Common::String::format("the game file it was made from, '%s', is missing", baseKey.c_str());
		else if (now.origSize != stamp.origSize || now.origCrc != stamp.origCrc)
			reason = "the game file it stands in for has changed";
	}

	if (!reason.empty())
		warning("SavedArchive: ignoring saved movie '%s' for '%s': %s. Delete it to stop this warning", fileName.c_str(), key.c_str(), reason.c_str());
	entry.trust = reason.empty() ? kTrusted : kUntrusted;
	return reason.empty();
}

} // End of namespace Director
