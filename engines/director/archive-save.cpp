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

#include "common/memstream.h"
#include "common/savefile.h"
#include "common/config-manager.h"
#include "common/crc.h"
#include "common/ptr.h"

#include "director/director.h"
#include "director/archive.h"
#include "director/saved-archive.h"
#include "director/cast.h"
#include "director/movie.h"
#include "director/score.h"
#include "director/window.h"
#include "director/sprite.h"
#include "director/util.h"

#include "director/castmember/castmember.h"
#include "director/castmember/bitmap.h"
#include "director/castmember/text.h"
#include "director/castmember/palette.h"
#include "director/castmember/filmloop.h"


namespace Director {

// Size and writer of a child resource re-serialized from its changed owner
// (see RIFXArchive::findChangedOwner())
static uint32 changedResourceSize(CastMember *member, uint32 tag) {
	switch (tag) {
	case MKTAG('B', 'I', 'T', 'D'):
		return ((BitmapCastMember *)member)->getBITDResourceSize();
	case MKTAG('S', 'T', 'X', 'T'):
		return ((TextCastMember *)member)->getSTXTResourceSize();
	case MKTAG('C', 'L', 'U', 'T'):
		return ((PaletteCastMember *)member)->getPaletteDataSize();
	case MKTAG('S', 'C', 'V', 'W'):
		return ((FilmLoopCastMember *)member)->getSCVWResourceSize();
	default:
		error("changedResourceSize(): unexpected tag %s", tag2str(tag));
	}
}

static void writeChangedResource(CastMember *member, uint32 tag, Common::SeekableWriteStream *out, uint32 offset) {
	switch (tag) {
	case MKTAG('B', 'I', 'T', 'D'):
		((BitmapCastMember *)member)->writeBITDResource(out, offset);
		break;
	case MKTAG('S', 'T', 'X', 'T'):
		((TextCastMember *)member)->writeSTXTResource(out, offset);
		break;
	case MKTAG('C', 'L', 'U', 'T'):
		((PaletteCastMember *)member)->writePaletteData(out, offset);
		break;
	case MKTAG('S', 'C', 'V', 'W'):
		((FilmLoopCastMember *)member)->writeSCVWResource(out, offset);
		break;
	default:
		error("writeChangedResource(): unexpected tag %s", tag2str(tag));
	}
}

bool RIFXArchive::writeToFile(Common::String filename, Movie *movie, SaveMode mode) {
	if (_rifxType == MKTAG('F', 'G', 'D', 'M') || _rifxType == MKTAG('F', 'G', 'D', 'C')) {
		warning("STUB: RIFXArchive::writeToFile(): saving Afterburner movies is not supported");
		return false;
	}

	// Refuse rather than silently lose runtime changes to members whose
	// writer can't re-serialize them for this version
	for (auto &it : *movie->getCasts()) {
		if (it._value->isStoredIn(this) && it._value->hasUnsavableChanges()) {
			warning("RIFXArchive::writeToFile(): not saving '%s': modified cast members would lose their changes", movie->getMacName().c_str());
			return false;
		}
	}

	Common::String saveKey;
	if (filename.empty() && !getPathName().empty() && _startOffset == 0) {
		// Director writes the movie back over its own file. Key the save
		// by that file's path, so it only stands in for this movie and
		// isn't found by lookups for files of the same name elsewhere,
		// e.g. a cast MAIN.CST falling back to MAIN.DXR.
		saveKey = getPathName().toString('/');
	} else if (mode == kSaveWhileOpen) {
		// Only a library that is a file of its own can be found again
		return false;
	} else {
		// If the filename is empty, we save the movie with the name of the current movie
		if (filename.empty()) {
			filename = movie->getMacName();
		}

		// Normalise the name the same way the movie loader does, so the key
		// the save is stored under matches the one used when the movie is later loaded by fileName.
		// Otherwise e.g. saving "C:\SCORES.DXR" and loading it back (which resolves
		// to "SCORES.DXR") would not find the saved file.
		saveKey = convertPath(filename);
	}
	Common::String saveFileName = g_director->getTargetName() + "-" + SavedArchive::encodeKey(saveKey);

	// Update the resources, their sizes and offsets
	Common::Array<Resource *> builtResources = rebuildResources(movie);
	// ignoring the startOffset
	// For RIFX stream, moreoffset = 0, we won't be writing macbinary
	_size = getArchiveSize(builtResources);

	// Build the whole movie in memory first, zero-filled since the writers
	// seek past what has been written
	uint32 imageSize = _size + 12;
	Common::MemoryWriteStreamDynamic image(DisposeAfterUse::YES);
	image.write(Common::Array<byte>(imageSize, 0).data(), imageSize);
	image.seek(0);
	Common::SeekableWriteStream *saveFile = &image;

	saveFile->writeUint32LE(_metaTag); // The _metaTag is "RIFX" or "XFIR"

	// We need to recompute the chunk size here, to accommodate any resournce changes
	saveFile->writeUint32LE(getArchiveSize(builtResources) + 4);
	saveFile->writeUint32LE(_rifxType);	// e.g. "MV93", "MV95"

	switch (_rifxType) {
	case MKTAG('M', 'V', '9', '3'):
	case MKTAG('M', 'C', '9', '5'):
	case MKTAG('A', 'P', 'P', 'L'):
		writeMemoryMap(saveFile, builtResources);
		break;

	default:
		break;
	}

	Cast *cast = movie->getCast();
	ResourceMap castResMap = _types[MKTAG('C', 'A', 'S', 't')];

	for (auto &it : builtResources) {
		debugC(5, kDebugSaving, "RIFXArchive::writeToFile: writing resource '%s': index: %d, size: %d, offset = %d, index: %d",
			tag2str(it->tag), it->index, it->size, it->offset, it->index);

		switch (it->tag) {
		case MKTAG('R', 'I', 'F', 'X'):
		case MKTAG('X', 'F', 'I', 'R'):
			// meta resource
			break;

		case MKTAG('i', 'm', 'a', 'p'):
		case MKTAG('m', 'm', 'a', 'p'):
			// Already written
			break;

		case MKTAG('K', 'E', 'Y', '*'):
			writeKeyTable(saveFile, it->offset);
			break;

		case MKTAG('C', 'A', 'S', '*'):
			writeCast(saveFile, it->offset, it->libResourceId);
			break;

		case MKTAG('C', 'A', 'S', 't'):
			// A 'CASt' that no 'CAS*' lists (e.g. in Mac Safecracker's
			// MAIN_*.DXR) belongs to no cast: keep its original bytes
			cast = it->libResourceId ? findCast(movie, it->libResourceId) : nullptr;
			if (cast) {
				cast->saveCastData(saveFile, it);
			} else {
				saveFile->seek(it->offset, SEEK_SET);
				saveFile->writeUint32LE(it->tag);
				saveFile->writeUint32LE(it->size);
				saveFile->writeStream(getResource(it->tag, it->index));
			}
			break;

		case MKTAG('V', 'W', 'C', 'F'):
		case MKTAG('D', 'R', 'C', 'F'):
			// There is only one config resource: the internal cast's in a
			// movie, the external cast's own in a cast file
			cast = findConfigCast(movie);
			if (cast->getConfigSize() == 0) {
				// Unsupported version (D10+): keep the original bytes
				debugC(7, kDebugSaving, "Saving resource %s as it is, without modification", tag2str(it->tag));
				saveFile->seek(it->offset, SEEK_SET);
				saveFile->writeUint32LE(it->tag);
				saveFile->writeUint32LE(it->size);
				saveFile->writeStream(getResource(it->tag, it->index));
			} else {
				cast->saveConfig(saveFile, it->offset, it->tag);
			}
			break;

		case MKTAG('V', 'W', 'S', 'C'):
			// The score is never edited at runtime (beginRecording is a
			// stub), so the original bytes are exact. Re-serializing it
			// changed Safecracker's MAIN enough to break it when reloaded.
			debugC(7, kDebugSaving, "Saving resource %s as it is, without modification", tag2str(it->tag));
			saveFile->seek(it->offset, SEEK_SET);
			saveFile->writeUint32LE(it->tag);
			saveFile->writeUint32LE(it->size);
			saveFile->writeStream(getResource(it->tag, it->index));
			break;

		case MKTAG('B', 'I', 'T', 'D'):
		case MKTAG('S', 'T', 'X', 'T'):
		case MKTAG('C', 'L', 'U', 'T'):
		case MKTAG('S', 'C', 'V', 'W'):
			{
				// Unchanged members keep their original data, as in Director,
				// which rewrites only dirty resources: e.g. the bitmap writer
				// doesn't compress, and stores 16-bit pixels interleaved
				CastMember *member = findChangedOwner(movie, it->tag, it->index);
				if (member) {
					writeChangedResource(member, it->tag, saveFile, it->offset);
					continue;
				}
			}
			// fall through

		default:
			if (it->tag == MKTAG('f', 'r', 'e', 'e') || it->tag == MKTAG('j', 'u', 'n', 'k')) {
				// Written without their data (see rebuildResources())
				saveFile->seek(it->offset, SEEK_SET);
				saveFile->writeUint32LE(it->tag);
				saveFile->writeUint32LE(0);
				break;
			}
			debugC(7, kDebugSaving, "Saving resource %s as it is, without modification", tag2str(it->tag));
			saveFile->seek(it->offset, SEEK_SET);
			saveFile->writeUint32LE(it->tag);
			saveFile->writeUint32LE(it->size);
			saveFile->writeStream(getResource(it->tag, it->index));
			break;
		}
	}

	if (image.size() != (int64)imageSize)
		warning("RIFXArchive::writeToFile: '%s' came out %d bytes instead of %d", saveFileName.c_str(), (int)image.size(), imageSize);

	// Prefer saving only the changes over the game file, so the game
	// file's own data is used for everything else. A full copy, stamped
	// with the original it stands in for, is the fallback.
	Common::MemoryWriteStreamDynamic out(DisposeAfterUse::YES);
	if (!writeOverlay(&out, image.getData(), imageSize, builtResources)) {
		out.write(image.getData(), imageSize);
		SavedArchive::writeStamp(&out, SavedArchive::stampFor(saveKey));
	}

	for (auto it : builtResources) {
		delete it;
	}

	SavedArchive *saved = SavedArchive::get();
	if (mode == kSaveWhileOpen) {
		saved->addOpenFile(saveFileName, getPathName(), out.getData(), out.size());
		debugC(3, kDebugSaving, "RIFXArchive::writeToFile: Kept the changes to %s while it is open (%d bytes)", saveFileName.c_str(), (int)out.size());
		invalidateAfterSave(filename);
		return true;
	}
	if (mode == kSaveForSession) {
		saved->addSessionFile(saveFileName, out.getData(), out.size());
		debugC(3, kDebugSaving, "RIFXArchive::writeToFile: Kept the movie as %s for this session (%d bytes)", saveFileName.c_str(), (int)out.size());
		invalidateAfterSave(filename);
		return true;
	}

	// Don't open the save file as compressed which doesn't support seeking
	Common::OutSaveFile *outFile = g_engine->getSaveFileManager()->openForSaving(saveFileName, false);
	if (!outFile) {
		warning("RIFXArchive::writeToFile: Failed to open file %s for saving", saveFileName.c_str());
		return false;
	}
	outFile->write(out.getData(), out.size());
	outFile->finalize();
	bool ok = !outFile->err();
	delete outFile;
	if (!ok) {
		warning("RIFXArchive::writeToFile: Error saving the file %s", saveFileName.c_str());
		return false;
	}
	debugC(3, kDebugSaving, "RIFXArchive::writeToFile: Saved the movie as file %s (%d bytes)", saveFileName.c_str(), (int)out.size());

	// Add to search index
	saved->addWrittenFile(saveFileName);
	invalidateAfterSave(filename);

	return true;
}

// The cached archives for the saved file are out of date now. Have them
// read back the next time the file is opened, or reopening the movie in
// the same session would show its contents from before the save (Opera
// Fatal's save slots). An empty filename means the movie was saved over
// its own file.
void RIFXArchive::invalidateAfterSave(const Common::String &filename) {
	g_director->markArchiveStale(getPathName());
	if (!filename.empty())
		g_director->markArchiveStale(Common::Path(convertPath(filename), g_director->_dirSeparator));
}

// Offsets in the layout that writeToFile() and writeMemoryMap() produce
enum {
	kRifxHeaderSize = 12,		// 'RIFX', size, movie type; the imap follows
	kChunkHeaderSize = 8,		// tag, size
	kImapMmapOffsetPos = 12,	// mmap offset, from the start of the imap chunk
	kMmapHeaderSize = 24,		// from the end of the mmap chunk header
	kMmapEntrySize = 20,		// tag, size, offset, flags, unk1, next free
	kMmapEntrySizePos = 4,
	kMmapEntryOffsetPos = 8
};

namespace {

// A saved movie as the game file plus a tail of changed resources (see
// RIFXArchive::writeOverlay())
struct OverlayBuild {
	const byte *base = nullptr;	// the game file
	uint32 baseSize = 0;
	uint32 hdrLen = 0;			// RIFX header and imap
	Common::MemoryWriteStreamDynamic tail{DisposeAfterUse::YES};
	Common::Array<uint32> newOffsets;	// per built resource
	uint rifxIndex = 0;
	uint mmapIndex = 0;
	bool haveMmap = false;
	uint kept = 0;
	uint moved = 0;
};

} // End of anonymous namespace

// Resources whose data is unchanged stay where they are in the game file;
// the rest go to the tail. The chunk headers aren't compared: the readers
// go by the memory map, and a big-endian game file has its headers
// swapped compared to the little-endian image. origResources hold the
// offsets in the game file, in the same order as builtResources.
static bool diffResources(OverlayBuild &ob, const byte *image, uint32 imageSize, const Common::Array<Resource *> &builtResources, const Common::Array<Resource *> &origResources) {
	if (ob.baseSize & 1)
		ob.tail.writeByte(0);

	ob.newOffsets.resize(builtResources.size());
	for (uint i = 0; i < builtResources.size(); i++) {
		const Resource *res = builtResources[i];
		switch (res->tag) {
		case MKTAG('R', 'I', 'F', 'X'):
		case MKTAG('X', 'F', 'I', 'R'):
			ob.rifxIndex = i;
			ob.newOffsets[i] = 0;
			continue;
		case MKTAG('i', 'm', 'a', 'p'):
			ob.newOffsets[i] = kRifxHeaderSize;
			continue;
		case MKTAG('m', 'm', 'a', 'p'):
			ob.mmapIndex = i;
			ob.haveMmap = true;
			continue;
		default:
			break;
		}

		if ((uint64)res->offset + kChunkHeaderSize + res->size > imageSize) {
			warning("RIFXArchive::writeOverlay: '%s' %d lies outside the movie", tag2str(res->tag), i);
			return false;
		}
		const byte *data = image + res->offset;

		const Resource *orig = i < origResources.size() ? origResources[i] : nullptr;
		if (orig && orig->offset >= (int32)ob.hdrLen && (uint64)orig->offset + kChunkHeaderSize + res->size <= ob.baseSize &&
				!memcmp(ob.base + orig->offset + kChunkHeaderSize, data + kChunkHeaderSize, res->size)) {
			ob.newOffsets[i] = orig->offset;
			ob.kept++;
			continue;
		}

		ob.newOffsets[i] = ob.baseSize + ob.tail.pos();
		ob.tail.write(data, kChunkHeaderSize + res->size);
		if (ob.tail.pos() & 1)
			ob.tail.writeByte(0);
		ob.moved++;
	}

	if (!ob.haveMmap) {
		warning("RIFXArchive::writeOverlay: no mmap");
		return false;
	}
	return true;
}

// Appends the memory map to the tail, with the new offsets and the new
// movie size in the RIFX entry. Returns the mmap's offset in the movie.
static uint32 appendMmap(OverlayBuild &ob, const byte *image, const Common::Array<Resource *> &builtResources) {
	const Resource *mmap = builtResources[ob.mmapIndex];
	uint32 mmapPos = ob.tail.pos();
	uint32 mmapOffset = ob.baseSize + mmapPos;
	ob.newOffsets[ob.mmapIndex] = mmapOffset;
	ob.tail.write(image + mmap->offset, kChunkHeaderSize + mmap->size);

	uint32 movieSize = ob.baseSize + (uint32)ob.tail.size();
	byte *entries = ob.tail.getData() + mmapPos + kChunkHeaderSize + kMmapHeaderSize;
	for (uint i = 0; i < builtResources.size(); i++)
		WRITE_LE_UINT32(entries + i * kMmapEntrySize + kMmapEntryOffsetPos, ob.newOffsets[i]);
	WRITE_LE_UINT32(entries + ob.rifxIndex * kMmapEntrySize + kMmapEntrySizePos, movieSize - kChunkHeaderSize);
	return mmapOffset;
}

// Writes the overlay: the new RIFX header and imap, then the tail
static void emitOverlay(Common::WriteStream *out, OverlayBuild &ob, const byte *image, const Common::String &baseKey, uint32 baseCrc, uint32 mmapOffset) {
	uint32 movieSize = ob.baseSize + (uint32)ob.tail.size();

	byte *header = (byte *)malloc(ob.hdrLen);
	memcpy(header, image, ob.hdrLen);
	WRITE_LE_UINT32(header + 4, movieSize - kChunkHeaderSize);
	WRITE_LE_UINT32(header + kRifxHeaderSize + kImapMmapOffsetPos, mmapOffset);

	SavedArchive::writeOverlayHeader(out, baseKey, ob.baseSize, baseCrc, movieSize, 2);
	out->writeUint32BE(0);
	out->writeUint32BE(ob.hdrLen);
	out->write(header, ob.hdrLen);
	out->writeUint32BE(ob.baseSize);
	out->writeUint32BE(ob.tail.size());
	out->write(ob.tail.getData(), ob.tail.size());
	free(header);

	SavedArchive::Stamp stamp;
	stamp.origSize = ob.baseSize;
	stamp.origCrc = baseCrc;
	SavedArchive::writeStamp(out, stamp);
}

bool RIFXArchive::writeOverlay(Common::WriteStream *out, const byte *image, uint32 imageSize, const Common::Array<Resource *> &builtResources) {
	// The movie must be a file of its own, read from the game files
	if (_startOffset != 0 || getPathName().empty())
		return false;
	if (_rifxType != MKTAG('M', 'V', '9', '3') && _rifxType != MKTAG('M', 'C', '9', '5'))
		return false;

	SavedArchive *saved = SavedArchive::get();
	if (!saved)
		return false;
	Common::String baseKey = saved->baseKeyFor(getPathName().toString('/'));

	OverlayBuild ob;
	byte *base = SavedArchive::readOriginal(baseKey, ob.baseSize);
	if (!base)
		return false;
	ob.base = base;
	ob.hdrLen = kRifxHeaderSize + kChunkHeaderSize + getImapSize();

	uint32 baseTag = ob.baseSize >= 4 ? READ_BE_UINT32(base) : 0;
	if (ob.baseSize < ob.hdrLen || (baseTag != MKTAG('R', 'I', 'F', 'X') && baseTag != MKTAG('X', 'F', 'I', 'R'))) {
		// e.g. a MacBinary file, whose data fork doesn't start at 0
		free(base);
		return false;
	}
	uint32 baseCrc = Common::CRC32().crcFast(base, ob.baseSize);

	// builtResources are copies of _resources, which hold the offsets in
	// the file this archive was read from
	bool ok = diffResources(ob, image, imageSize, builtResources, _resources);
	free(base);
	ob.base = nullptr;
	if (!ok)
		return false;

	uint32 mmapOffset = appendMmap(ob, image, builtResources);
	emitOverlay(out, ob, image, baseKey, baseCrc, mmapOffset);

	debugC(3, kDebugSaving, "RIFXArchive::writeOverlay: over '%s': %d resources kept, %d saved, %d of %d bytes",
		baseKey.c_str(), ob.kept, ob.moved, (int)(ob.tail.size() + ob.hdrLen), (int)(ob.baseSize + ob.tail.size()));
	return true;
}

bool RIFXArchive::writeMemoryMap(Common::SeekableWriteStream *writeStream, Common::Array<Resource *> resources) {
	Resource mmap;

	for (auto it : resources) {
		if (it->tag == MKTAG('m', 'm', 'a', 'p')) {
			mmap = *it;
		}
	}

	writeStream->writeUint32LE(MKTAG('i', 'm', 'a', 'p')); // The "imap" resource
	writeStream->writeUint32LE(_imapLength);		// length of "imap" resource
	writeStream->writeUint32LE(_mapversion);		// "imap" version
	writeStream->writeUint32LE(mmap.offset);		// offset of the "mmap" resource
	writeStream->writeUint32LE(_version);

	writeStream->seek(mmap.offset);
	writeStream->writeUint32LE(MKTAG('m', 'm', 'a', 'p'));
	writeStream->writeUint32LE(mmap.size);

	writeStream->writeUint16LE(_mmapHeaderSize);
	writeStream->writeUint16LE(_mmapEntrySize);

	uint32 newResCount = resources.size();
	writeStream->writeUint32LE(newResCount + _totalCount - _resCount); // _totalCount - _resCount is the number of empty entries
	writeStream->writeUint32LE(newResCount);
	writeStream->seek(8, SEEK_CUR);		// In the original file, these 8 bytes are all 0xFF, so this will produce a diff

	// ID of the first 'free' resource, we don't make use of it
	writeStream->writeUint32LE(0);

	for (auto &it : resources) {
		debugC(3, kDebugSaving, "RIFXArchive::writeMemoryMap: Memory map entry: '%s', size: %d, offset: %08x, flags: %x, unk1: %x, nextFreeResourceID: %d",
			tag2str(it->tag), it->size, it->offset, it->flags, it->unk1, it->nextFreeResourceID);

		// Write down the tag, the size and offset of the current resource
		writeStream->writeUint32LE(it->tag);
		writeStream->writeUint32LE(it->size);
		writeStream->writeUint32LE(it->offset);

		// Currently ignoring flags, unk1 and nextFreeResourceID
		writeStream->writeUint16LE(it->flags);
		writeStream->writeUint16LE(it->unk1);
		writeStream->writeUint32LE(it->nextFreeResourceID);
	}

	return true;
}

bool RIFXArchive::writeAfterBurnerMap(Common::SeekableWriteStream *writeStream) {
	warning("RIFXArchive::writeAfterBurnerMap: STUB: Incomplete function, needs further changes, AfterBurnerMap not written");
	return false;

#if 0
	writeStream->writeUint32LE(MKTAG('F', 'v', 'e', 'r'));

	writeStream->writeUint32LE(_fverLength);
	uint32 start = writeStream->pos();

	writeStream->writeUint32LE(_afterBurnerVersion);
	uint32 end = writeStream->pos();

	if (end - start != _fverLength) {
		warning("RIFXArchive::writeAfterburnerMap(): Expected Fver of length %d but read %d bytes", _fverLength, end - start);
		writeStream->seek(start + _fverLength);
	}

	writeStream->writeUint32LE(MKTAG('F', 'c', 'd', 'r'));
	writeStream->writeUint32LE(_fcdrLength);
	writeStream->seek(_fcdrLength, SEEK_CUR);

	writeStream->writeUint32LE(MKTAG('A', 'B', 'M', 'P'));

	return true;
#endif
}

bool RIFXArchive::writeKeyTable(Common::SeekableWriteStream *writeStream, uint32 offset) {
	writeStream->seek(offset);

	uint32 numEntries = (getKeyTableResourceSize() - 12) / 12;

	writeStream->writeUint32LE(MKTAG('K', 'E', 'Y', '*'));
	writeStream->writeUint32LE(getKeyTableResourceSize());

	writeStream->writeUint16LE(_keyTableEntrySize);
	writeStream->writeUint16LE(_keyTableEntrySize2);
	writeStream->writeUint32LE(numEntries);
	writeStream->writeUint32LE(numEntries);

	debugC(3, kDebugSaving, "RIFXArchive::writeKeyTable: writing key table:");

	for (auto &childTag : _keyData) {
		KeyMap keyMap = childTag._value;

		for (auto &parentIndex : keyMap) {
			KeyArray keyArray = parentIndex._value;

			for (auto childIndex : keyArray) {
				debugC(3, kDebugSaving, "_keyData contains tag: %s, parentIndex: %d, childIndex: %d", tag2str(childTag._key), parentIndex._key, childIndex);
				writeStream->writeUint32LE(childIndex);
				writeStream->writeUint32LE(parentIndex._key);
				writeStream->writeUint32LE(childTag._key);
			}
		}
	}

	return true;
}

bool RIFXArchive::writeCast(Common::SeekableWriteStream *writeStream, uint32 offset, uint32 libResourceId) {
	writeStream->seek(offset);
	uint32 casSize = getCASResourceSize(libResourceId);

	uint castTag = MKTAG('C', 'A', 'S', 't');
	writeStream->writeUint32LE(MKTAG('C', 'A', 'S', '*'));
	writeStream->writeUint32LE(casSize);

	Common::HashMap<uint16, uint16> castIndexes;

	// We can't just write all the 'CASt' indices randomly, we have to sort them by castId
	// Since the order they appear matters, they are given castIds accordingly
	uint32 maxCastId = 0;
	bool empty = true;
	for (auto &it : _types[castTag]) {
		if (it._value.libResourceId == libResourceId) {
			castIndexes[it._value.castId] = it._value.index;
			maxCastId = MAX(maxCastId, it._value.castId);
			empty = false;
		}
	}
	// A cast without members has an empty 'CAS*'
	if (empty)
		return true;

	debugC(5, kDebugSaving, "RIFXArchive::writeCast: Writing CAS* resource: size: %d, maxCastID: %d, libResourceID: %d", casSize, maxCastId, libResourceId);
	debugCN(5, kDebugSaving, "'CASt' indexes: [");
	for (uint32 i = 0; i <= maxCastId; i++) {
		uint32 castIndex = castIndexes.getValOrDefault(i, 0);
		// CAS* is a positional array: empty slots are written as 0
		writeStream->writeUint32BE(castIndex);
		if (castIndex)
			debugCN(5, kDebugSaving, (i == 0 ? "%d" : ", %d"), castIndex);
	}
	debugC(5, kDebugSaving, "]");

	return true;
}

Common::Array<Resource *> RIFXArchive::rebuildResources(Movie *movie) {
	// Currently handled Resource types:
	// imap		// BITD
	// mmap		// CLUT
	// RIFX		// STXT
	// KEY*		// SCVW (filmloop)
	// CAS*		// VWCF
	// CASt
	// SCVW
	// 'RTE0', 'RTE1', 'RTE2'

	// First we'll have to update the _types table to include all the newly added
	// cast members, and their 'CASt' resources
	Cast *cast = nullptr;
	ResourceMap &castResMap = _types[MKTAG('C', 'A', 'S', 't')];

	// A cast file's single library id, from its own KEY* table
	uint32 castFileLibResourceId = 1024;
	if (_types.contains(MKTAG('C', 'A', 'S', '*')) && !_types[MKTAG('C', 'A', 'S', '*')].empty())
		castFileLibResourceId = _types[MKTAG('C', 'A', 'S', '*')].begin()->_value.libResourceId;

	// Iterate over the casts stored in this file
	for (auto it : *(movie->getCasts())) {
		cast = it._value;
		if (!cast->isStoredIn(this))
			continue;

		// Iterate over all the loaded members of the cast to check for new cast members
		for (auto jt : *(cast->_loadedCast)) {
			// If the index is -1, that means the cast member is new or duplicated
			if (jt._value->_index == -1) {
				Resource *res = nullptr;
				uint16 targetCastId = jt._value->getID() - cast->_castArrayStart;

				// Checking if the castId already exists in the CASt resources
				for (auto castRes : castResMap) {
					if (castRes._value.castId == targetCastId) {
						res = &castRes._value;
					}
				}

				if (!res) {
					// If the castId is new, create a new resource
					// Assigning the next available index to the resource
					uint16 newResIndex = _resources.size();
					res = &castResMap[newResIndex];
					res->tag = MKTAG('C', 'A', 'S', 't');
					res->accessed = true;

					// An external cast's members belong to the cast file's own
					// library, whatever id the movie's MCsL gives it
					res->libResourceId = cast->_isExternal ? castFileLibResourceId : cast->_libResourceId;
					res->index = newResIndex;
					res->castId = jt._value->getID() - cast->_castArrayStart;
					_resources.push_back(res);

					// Clone the child resources that are re-serialized from
					// their owning member's state: the duplicate must not
					// share them with its source
					res->children.clear();
					for (auto child : jt._value->_children) {
						if (child.tag == MKTAG('S', 'T', 'X', 'T') || child.tag == MKTAG('B', 'I', 'T', 'D') ||
								child.tag == MKTAG('C', 'L', 'U', 'T') || child.tag == MKTAG('S', 'C', 'V', 'W')) {
							uint16 childIndex = _resources.size();
							Resource &newChild = _types[child.tag][childIndex];
							newChild = child;
							newChild.index = childIndex;
							_resources.push_back(&newChild);
							child.index = childIndex;
						}
						res->children.push_back(child);
					}

					for (const auto &child : res->children) {
						_keyData[child.tag][res->index].push_back(child.index);
						_keyTableUsedCount += 1;
						_keyTableEntryCount += 1;
					}

					debugC(5, kDebugSaving, "RIFXArchive::rebuildResources(): new 'CASt' resource added");
				} else {
					// The castId is not new, overwrite the key data of the previous cast
					for (const auto &child : res->children) {
						// Remove the data of the previous (removed) 'CASt'
						int8 count = _keyData[child.tag][res->index].size();
						_keyData[child.tag][res->index].clear();
						_keyTableUsedCount -= count;
						_keyTableEntryCount -= count;
					}

					res->children = jt._value->_children;

					for (const auto &child : res->children) {
						_keyData[child.tag][res->index].push_back(child.index);
						_keyTableUsedCount += 1;
						_keyTableEntryCount += 1;
					}
				}
			}
		}
	}

	// TODO: Then we'll need to see if there are any other newly added resources
	// Now when you use `duplicate`, say BitmapCastMember, the cast member is duplicated
	// but its children resources are not, meaning the duplicated BitmapCastMember is also loaded from the same 'BITD' resource
	// So it is not necessary to duplicate the 'BITD' resource
	// However, in case an entirely new cast member is added, say a filmloop is recorded, then that requires a new 'SCVW' resource
	// Same goes for if a new cast is added to the movie
	// Ignoring that for now

	// Next step is to recalculate the sizes and the offsets of all the resources

	// Since the first 3 resources are determined (RIFX, imap and mmap)
	// (the mmap doesn't need to be the third resource, but for simplicity, it's better there)
	// We'll start writing after that RIFX header, mmap and imap resources
	// The first 12 bytes are metaTag ('RIFX'), size of file, and RIFX type ('MV93', 'MV95', etc.)
	// The +8 bytes are to account for the header and size
	uint32 currentSize = 12 + (getImapSize() + 8) + (getMmapSize() + 8);

	// need to make a new resources array, because we need the old offsets as well as new ones
	Common::Array<Resource *> builtResources;

	for (auto it: _resources) {
		builtResources.push_back(new Resource(it));
	}

	uint32 resSize = 0;
	for (auto &it : builtResources) {
		switch (it->tag) {
		case MKTAG('R', 'I', 'F', 'X'):
		case MKTAG('X', 'F', 'I', 'R'):
			// only one resource only
			// Size will be determined after all other sizes have been calculated
			it->offset = 0;
			break;

		case MKTAG('i', 'm', 'a', 'p'):
			// one resource only
			it->size = getImapSize();
			it->offset = 12;		// First 12 bytes are reserved for metaTag ('RIFX'), size of file, and RIFX type ('MV93', 'MV95', etc.)
			break;

		case MKTAG('m', 'm', 'a', 'p'):
			// one resource only
			it->size = getMmapSize();
			it->offset = 12 + (getImapSize() + 8);	// The +8 is to account for header and size
			break;

		case MKTAG('C', 'A', 'S', 't'):
			{
				// A 'CASt' that no 'CAS*' lists has no cast; keep its original bytes
				cast = it->libResourceId ? findCast(movie, it->libResourceId) : nullptr;
				// The castIds of cast members start from _castArrayStart
				CastMember *target = cast ? cast->getCastMember(it->castId + cast->_castArrayStart) : nullptr;

				if (target && !cast->keepOriginalCastBytes(target)) {
					resSize = target->getCastResourceSize();
					it->size = resSize;		// getCastResourceSize returns size without header and size
				} else {
					// Members without a version-capable writer keep the original bytes
					resSize = it->size;
				}
				it->offset = currentSize;
				currentSize += resSize + 8;
			}
			break;

		case MKTAG('C', 'A', 'S', '*'):
			// Currently handling only movies with one 'CAS*' resource, i.e. only one cast
			resSize = getCASResourceSize(it->libResourceId);	// getCASResourceSize() returns size without header and size
			it->size = resSize;
			it->offset = currentSize;
			currentSize += resSize + 8;
			break;

		case MKTAG('K', 'E', 'Y', '*'):
			resSize = getKeyTableResourceSize();
			it->size = resSize;
			it->offset = currentSize;
			currentSize += resSize + 8;
			break;

		case MKTAG('V', 'W', 'C', 'F'):
		case MKTAG('D', 'R', 'C', 'F'):
			{
				// Only one config resource per file
				// No need to update the key mapping
				cast = findConfigCast(movie);
				resSize = cast->getConfigSize();

				it->offset = currentSize;

				if (resSize == 0) {
					// Unsupported Director version: keep the original size
					resSize = it->size;
				} else {
					it->size = resSize;
				}

				currentSize += resSize + 8;			// getConfigSize() doesn't include header and size
			}
			break;

		case MKTAG('S', 'T', 'X', 'T'):
		case MKTAG('C', 'L', 'U', 'T'):
		case MKTAG('B', 'I', 'T', 'D'):
		case MKTAG('S', 'C', 'V', 'W'):
			{
				// Kept verbatim unless changed; see the matching case in writeToFile()
				CastMember *member = findChangedOwner(movie, it->tag, it->index);
				if (member)
					it->size = changedResourceSize(member, it->tag);
				resSize = it->size;

				it->offset = currentSize;
				currentSize += resSize + 8;
			}
			break;

		case MKTAG('V', 'W', 'S', 'C'):
			// The original bytes are kept (see writeToFile())
			resSize = it->size;
			it->offset = currentSize;
			currentSize += resSize + 8;		// The size doesn't include the header and the size entry
			break;

		case MKTAG('f', 'r', 'e', 'e'):
		case MKTAG('j', 'u', 'n', 'k'):
			// These resources do not hold any data
			it->size = 0;

			// We could just ignore these and not write them at all
			it->offset = currentSize;
			currentSize += 8;
			break;

		default:
			it->offset = currentSize;
			currentSize += it->size + 8;		// This size doesn't include the header and size entry
			break;

		}
		debugC(3, kDebugSaving, "Rebuild RIFX resource index %d: '%s', %d bytes @ 0x%08x (%d), flags: %x unk1: %x nextFreeResourceId: %d",
			it->index, tag2str(it->tag), it->size, it->offset, it->offset, it->flags, it->unk1, it->nextFreeResourceID);
	}

	// Now that all sizes have been updated, we can safely calculate the overall archive size
	for (auto &it : builtResources) {
		if (it->tag == MKTAG('R', 'I', 'F', 'X') || it->tag == MKTAG('X', 'F', 'I', 'R')) {
			it->size = getArchiveSize(builtResources) + 4;
		}
	}

	return builtResources;
}

uint32 RIFXArchive::getImapSize() {
	// The length of imap doesn't change
	// This is the length without header and size
	return _imapLength;
}

uint32 RIFXArchive::getMmapSize() {
	// The headers: 24 bytes and and 20 bytes per resources
	return 24 + 20 * _resources.size();
}

uint32 RIFXArchive::getArchiveSize(Common::Array<Resource *> resources) {
	// This will be called after updating the size of all the resources
	uint32 size = 0;

	for (auto it : resources) {
		if (it->tag != MKTAG('R', 'I', 'F', 'X') && it->tag != MKTAG('X', 'F', 'I', 'R')) {
			size += it->size + 8;		// The 8 is to account for the header and size
		}
	}
	return size;
}

uint32 RIFXArchive::getCASResourceSize(uint32 libResourceID) {
	uint castTag = MKTAG('C', 'A', 'S', 't');
	uint32 maxCastId = 0;
	bool empty = true;

	// maxCastId is the basically the number of cast members present in the cast
	// This is the number of entries present in the 'CAS*' resource
	for (auto &it : _types[castTag]) {
		if (it._value.libResourceId == libResourceID) {
			maxCastId = MAX(maxCastId, it._value.castId);
			empty = false;
		}
	}

	// A cast without members (e.g. Safecracker's empty internal casts) has
	// an empty 'CAS*', not a single null entry
	return empty ? 0 : (maxCastId + 1) * 4;
}

uint32 RIFXArchive::getKeyTableResourceSize() {
	// 12 bytes of header + 12 * number of entries
	uint32 size = 0;
	for (auto &childTag : _keyData) {
		KeyMap keyMap = childTag._value;

		for (auto &parentIndex : keyMap) {
			KeyArray keyArray = parentIndex._value;

			size += keyArray.size() * 12u;
		}
	}
	return size + 12u;
}

void dumpFile(Common::String fileName, uint32 id, uint32 tag, byte *dumpData, uint32 dumpSize) {
	debug("dumpFile():: dumping file %s, resource: %s (id: %d)", fileName.c_str(), tag2str(tag), id);
	Common::DumpFile out;
	Common::String fname = Common::String::format("./dumps/%d-%s-%s", id, tag2str(tag), fileName.c_str());

	// Write the movie out, stored in dumpData
	if (out.open(Common::Path(fname), true)) {
		out.write(dumpData, dumpSize);
		out.flush();
		out.close();
	} else {
		warning("RIFXArchive::writeStream: Error saving the file %s", fname.c_str());
	}
}

uint32 RIFXArchive::findParentIndex(uint32 tag, uint16 index) {
	// We have to find the parent
	// Look into the keyData, for all parents of resource tag
	// If the parent contains this resource's index, that's our parent
	for (auto &it : _keyData[tag]) {
		for (auto &kt : it._value) {
			if (kt == index) {
				return it._key;
			}
		}
	}

	warning("RIFXArchive::findParentIndex: The parent for resource: %s, index: %d, was not found", tag2str(tag), index);
	return kNoParent;
}

// Resolves the cast member owning a child resource (BITD, STXT, CLUT,
// SCVW); nullptr when the parent or member is missing
CastMember *RIFXArchive::findResourceOwner(Movie *movie, uint32 tag, uint16 index) {
	uint32 parentIndex = findParentIndex(tag, index);
	if (parentIndex == kNoParent)
		return nullptr;

	ResourceMap &castResMap = _types[MKTAG('C', 'A', 'S', 't')];
	if (!castResMap.contains(parentIndex))
		return nullptr;

	const Resource &parent = castResMap[parentIndex];
	Cast *cast = parent.libResourceId ? findCast(movie, parent.libResourceId) : nullptr;
	return cast ? cast->getCastMember(parent.castId + cast->_castArrayStart) : nullptr;
}

// The type of member whose writer produces each child resource. The
// owner is not always that type, e.g. a D4 script cast member owns its
// source text in an 'STXT'
static bool ownerWritesTag(const CastMember *member, uint32 tag) {
	switch (tag) {
	case MKTAG('B', 'I', 'T', 'D'):
		return member->_type == kCastBitmap;
	case MKTAG('S', 'T', 'X', 'T'):
		return member->_type == kCastText || member->_type == kCastButton;
	case MKTAG('C', 'L', 'U', 'T'):
		return member->_type == kCastPalette;
	case MKTAG('S', 'C', 'V', 'W'):
		return member->_type == kCastFilmLoop;
	default:
		return false;
	}
}

// The owner of a child resource when it has changed and its writer can
// re-serialize the resource; nullptr when the original bytes are kept
CastMember *RIFXArchive::findChangedOwner(Movie *movie, uint32 tag, uint16 index) {
	CastMember *member = findResourceOwner(movie, tag, index);
	if (!member || !member->needsSaving() || !ownerWritesTag(member, tag))
		return nullptr;
	return member;
}

// The movie's cast whose members this file stores under libResourceId. A
// cast file holds a single library, which may have a different id from
// the one in the movie's MCsL (and may share it with the internal cast)
Cast *RIFXArchive::findCast(Movie *movie, uint32 libResourceId) {
	for (auto &it : *movie->getCasts()) {
		Cast *cast = it._value;
		if (!cast->isStoredIn(this))
			continue;
		if (cast->_isExternal || cast->_libResourceId == libResourceId)
			return cast;
	}
	return nullptr;
}

Cast *RIFXArchive::findConfigCast(Movie *movie) {
	for (auto &it : *movie->getCasts()) {
		if (it._value->_isExternal && it._value->isStoredIn(this))
			return it._value;
	}
	return movie->getCast();
}

} // End of namespace Director
