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

bool RIFXArchive::writeToFile(Common::String filename, Movie *movie, bool sessionOnly) {
	if (_rifxType == MKTAG('F', 'G', 'D', 'M') || _rifxType == MKTAG('F', 'G', 'D', 'C')) {
		warning("STUB: RIFXArchive::writeToFile(): saving Afterburner movies is not supported");
		return false;
	}

	// Refuse rather than silently lose runtime changes to members whose
	// writer can't re-serialize them for this version
	for (auto &it : *movie->getCasts()) {
		if (it._value->getArchive().get() == this && it._value->hasUnsavableChanges()) {
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
	{
		byte zeros[1024] = {};
		for (uint32 left = imageSize; left > 0;) {
			uint32 n = MIN<uint32>(left, sizeof(zeros));
			image.write(zeros, n);
			left -= n;
		}
		image.seek(0);
	}
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
				CastMember *member = findResourceOwner(movie, it->tag, it->index);
				// The owner is not always the type the tag implies, e.g. a D4
				// script cast member owns its source text in an 'STXT'
				// Unchanged members keep their original data, as in Director,
				// which rewrites only dirty resources: e.g. the bitmap writer
				// doesn't compress, and stores 16-bit pixels interleaved
				if (member && member->needsSaving()) {
					if (it->tag == MKTAG('B', 'I', 'T', 'D') && member->_type == kCastBitmap) {
						((BitmapCastMember *)member)->writeBITDResource(saveFile, it->offset);
						continue;
					}
					if (it->tag == MKTAG('S', 'T', 'X', 'T') && (member->_type == kCastText || member->_type == kCastButton)) {
						((TextCastMember *)member)->writeSTXTResource(saveFile, it->offset);
						continue;
					}
					if (it->tag == MKTAG('C', 'L', 'U', 'T') && member->_type == kCastPalette) {
						((PaletteCastMember *)member)->writePaletteData(saveFile, it->offset);
						continue;
					}
					if (it->tag == MKTAG('S', 'C', 'V', 'W') && member->_type == kCastFilmLoop) {
						((FilmLoopCastMember *)member)->writeSCVWResource(saveFile, it->offset);
						continue;
					}
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

	SavedArchive *saved = (SavedArchive *)SearchMan.getArchive(kSavedFilesArchive);
	if (sessionOnly) {
		saved->addSessionFile(saveFileName, out.getData(), out.size());
		debugC(3, kDebugSaving, "RIFXArchive::writeToFile: Kept the movie as %s for this session (%d bytes)", saveFileName.c_str(), (int)out.size());
		g_director->markArchiveStale(getPathName());
		if (!filename.empty())
			g_director->markArchiveStale(Common::Path(convertPath(filename), g_director->_dirSeparator));
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

	// The cached archives for the saved file are out of date now. Have
	// them read back from disk the next time the file is opened, or
	// reopening the movie in the same session would show its contents
	// from before the save (Opera Fatal's save slots).
	g_director->markArchiveStale(getPathName());
	if (!filename.empty())
		g_director->markArchiveStale(Common::Path(convertPath(filename), g_director->_dirSeparator));

	return true;
}

bool RIFXArchive::writeOverlay(Common::WriteStream *out, const byte *image, uint32 imageSize, const Common::Array<Resource *> &builtResources) {
	// The movie must be a file of its own, read from the game files
	if (_startOffset != 0 || getPathName().empty())
		return false;
	if (_rifxType != MKTAG('M', 'V', '9', '3') && _rifxType != MKTAG('M', 'C', '9', '5'))
		return false;

	SavedArchive *saved = (SavedArchive *)SearchMan.getArchive(kSavedFilesArchive);
	if (!saved)
		return false;
	Common::String baseKey = saved->baseKeyFor(getPathName().toString('/'));

	uint32 baseSize = 0;
	byte *base = SavedArchive::readOriginal(baseKey, baseSize);
	if (!base)
		return false;

	uint32 hdrLen = 12 + 8 + getImapSize();
	uint32 baseTag = baseSize >= 4 ? READ_BE_UINT32(base) : 0;
	if (baseSize < hdrLen || (baseTag != MKTAG('R', 'I', 'F', 'X') && baseTag != MKTAG('X', 'F', 'I', 'R'))) {
		// e.g. a MacBinary file, whose data fork doesn't start at 0
		free(base);
		return false;
	}
	uint32 baseCrc = Common::CRC32().crcFast(base, baseSize);

	// Resources whose data is unchanged stay where they are in the game
	// file; the rest go to a tail after it. The chunk headers aren't
	// compared: the readers go by the memory map, and a big-endian game
	// file has its headers swapped compared to the little-endian image.
	Common::MemoryWriteStreamDynamic tail(DisposeAfterUse::YES);
	if (baseSize & 1)
		tail.writeByte(0);

	Common::Array<uint32> newOffsets;
	newOffsets.resize(builtResources.size());
	uint32 mmapIndex = 0, rifxIndex = 0;
	bool haveMmap = false;
	uint32 kept = 0, moved = 0;

	for (uint i = 0; i < builtResources.size(); i++) {
		const Resource *res = builtResources[i];
		switch (res->tag) {
		case MKTAG('R', 'I', 'F', 'X'):
		case MKTAG('X', 'F', 'I', 'R'):
			rifxIndex = i;
			newOffsets[i] = 0;
			continue;
		case MKTAG('i', 'm', 'a', 'p'):
			newOffsets[i] = 12;
			continue;
		case MKTAG('m', 'm', 'a', 'p'):
			mmapIndex = i;
			haveMmap = true;
			continue;
		default:
			break;
		}

		if ((uint64)res->offset + 8 + res->size > imageSize) {
			warning("RIFXArchive::writeOverlay: '%s' %d lies outside the movie", tag2str(res->tag), i);
			free(base);
			return false;
		}
		const byte *data = image + res->offset;

		// builtResources are copies of _resources, which hold the offsets
		// in the file this archive was read from
		const Resource *orig = i < _resources.size() ? _resources[i] : nullptr;
		if (orig && orig->offset >= (int32)hdrLen && (uint64)orig->offset + 8 + res->size <= baseSize &&
				!memcmp(base + orig->offset + 8, data + 8, res->size)) {
			newOffsets[i] = orig->offset;
			kept++;
			continue;
		}

		newOffsets[i] = baseSize + tail.pos();
		tail.write(data, 8 + res->size);
		if (tail.pos() & 1)
			tail.writeByte(0);
		moved++;
	}
	free(base);

	if (!haveMmap) {
		warning("RIFXArchive::writeOverlay: no mmap");
		return false;
	}

	// The memory map goes last, with the new offsets
	const Resource *mmap = builtResources[mmapIndex];
	uint32 mmapPos = tail.pos();
	uint32 mmapOffset = baseSize + mmapPos;
	newOffsets[mmapIndex] = mmapOffset;
	tail.write(image + mmap->offset, 8 + mmap->size);

	uint32 movieSize = baseSize + tail.size();
	byte *tailData = tail.getData();
	// As laid out by writeMemoryMap(): a 24-byte header, 20-byte entries
	uint32 entries = mmapPos + 8 + 24;
	for (uint i = 0; i < builtResources.size(); i++) {
		uint32 entry = entries + i * 20;
		WRITE_LE_UINT32(tailData + entry + 8, newOffsets[i]);
	}
	// The RIFX entry's size is the movie's
	WRITE_LE_UINT32(tailData + entries + rifxIndex * 20 + 4, movieSize - 8);

	// The new RIFX header and imap
	byte *header = (byte *)malloc(hdrLen);
	memcpy(header, image, hdrLen);
	WRITE_LE_UINT32(header + 4, movieSize - 8);
	WRITE_LE_UINT32(header + 12 + 12, mmapOffset);

	SavedArchive::writeOverlayHeader(out, baseKey, baseSize, baseCrc, movieSize, 2);
	out->writeUint32BE(0);
	out->writeUint32BE(hdrLen);
	out->write(header, hdrLen);
	out->writeUint32BE(baseSize);
	out->writeUint32BE(tail.size());
	out->write(tailData, tail.size());
	free(header);

	SavedArchive::Stamp stamp;
	stamp.origSize = baseSize;
	stamp.origCrc = baseCrc;
	SavedArchive::writeStamp(out, stamp);

	debugC(3, kDebugSaving, "RIFXArchive::writeOverlay: over '%s': %d resources kept, %d saved, %d of %d bytes",
		baseKey.c_str(), kept, moved, (int)tail.size() + hdrLen, movieSize);
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
		if (cast->getArchive().get() != this)
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
			{
				CastMember *member = findResourceOwner(movie, it->tag, it->index);
				if (member && member->needsSaving() && (member->_type == kCastText || member->_type == kCastButton)) {
					resSize = ((TextCastMember *)member)->getSTXTResourceSize();
					it->size = resSize;
				} else {
					// Kept verbatim; see the matching case in writeToFile()
					resSize = it->size;
				}

				it->offset = currentSize;
				currentSize += resSize + 8;
			}
			break;

		case MKTAG('C', 'L', 'U', 'T'):
			{
				CastMember *member = findResourceOwner(movie, it->tag, it->index);
				if (member && member->needsSaving() && member->_type == kCastPalette) {
					resSize = ((PaletteCastMember *)member)->getPaletteDataSize();
					it->size = resSize;
				} else {
					resSize = it->size;
				}

				it->offset = currentSize;
				currentSize += resSize + 8;
			}
			break;

		case MKTAG('B', 'I', 'T', 'D'):
			{
				CastMember *member = findResourceOwner(movie, it->tag, it->index);
				if (member && member->needsSaving() && member->_type == kCastBitmap) {
					resSize = ((BitmapCastMember *)member)->getBITDResourceSize();
					it->size = resSize;
				} else {
					resSize = it->size;
				}

				it->offset = currentSize;
				currentSize += resSize + 8;
			}
			break;

		case MKTAG('S', 'C', 'V', 'W'):
			{
				CastMember *member = findResourceOwner(movie, it->tag, it->index);
				if (member && member->needsSaving() && member->_type == kCastFilmLoop) {
					resSize = ((FilmLoopCastMember *)member)->getSCVWResourceSize();
					it->size = resSize;
				} else {
					resSize = it->size;
				}

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

// The movie's cast whose members this file stores under libResourceId. A
// cast file holds a single library, which may have a different id from
// the one in the movie's MCsL (and may share it with the internal cast)
Cast *RIFXArchive::findCast(Movie *movie, uint32 libResourceId) {
	for (auto &it : *movie->getCasts()) {
		Cast *cast = it._value;
		if (cast->getArchive().get() != this)
			continue;
		if (cast->_isExternal || cast->_libResourceId == libResourceId)
			return cast;
	}
	return nullptr;
}

Cast *RIFXArchive::findConfigCast(Movie *movie) {
	for (auto &it : *movie->getCasts()) {
		if (it._value->_isExternal && it._value->getArchive().get() == this)
			return it._value;
	}
	return movie->getCast();
}

SavedArchive::SavedArchive(const Common::String &target) {
	_target = target;
	Common::StringArray saveFileList = g_engine->getSaveFileManager()->listSavefiles(target + "-*");

	debugC(3, kDebugLoading, "DirectorEngine:: loadSaveFiles: Loading save files");
	for (auto &saveFileName : saveFileList) {
		_addFile(saveFileName);
	}
}

bool SavedArchive::hasFile(const Common::Path &path) const {
	if (_sessionFiles.contains(path.toString()))
		return true;
	return _files.contains(path.toString()) && isTrusted(path.toString());
}

int SavedArchive::listMembers(Common::ArchiveMemberList &list) const {
	int count = 0;

	for (SessionMap::const_iterator i = _sessionFiles.begin(); i != _sessionFiles.end(); ++i) {
		list.push_back(Common::ArchiveMemberList::value_type(new Common::GenericArchiveMember(i->_key, *this)));
		++count;
	}
	for (FileMap::const_iterator i = _files.begin(); i != _files.end(); ++i) {
		if (_sessionFiles.contains(i->_key) || !isTrusted(i->_key))
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
	SessionMap::const_iterator session = _sessionFiles.find(key);
	if (session != _sessionFiles.end())
		return new Common::MemoryReadStream(session->_value.data(), session->_value.size());

	FileMap::const_iterator fDesc = _files.find(key);
	if (fDesc == _files.end() || !isTrusted(fDesc->_key))
		return nullptr;
	return g_engine->getSaveFileManager()->openForLoading(fDesc->_value);
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

void SavedArchive::addWrittenFile(const Common::String &fileName) {
	_addFile(fileName);
	Common::String key = keyFor(fileName);
	_trusted[key] = true;
	_sessionFiles.erase(key);
}

void SavedArchive::addSessionFile(const Common::String &fileName, const byte *data, uint32 size) {
	Common::Array<byte> &buf = _sessionFiles[keyFor(fileName)];
	buf.resize(size);
	if (size)
		memcpy(buf.data(), data, size);
}

// Bump this whenever the movie writer changes its output; raise
// kMinTrustedVersion to it when the change fixes saves that broke a game,
// so saves written before the fix are ignored and the game files used
// instead. 1: unchanged resources kept verbatim (e02180d7893).
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
	Common::Archive *saved = SearchMan.getArchive(kSavedFilesArchive);
	Common::ScopedPtr<Common::SeekableReadStream> orig(SearchMan.createReadStreamForMemberNext(Common::Path(key), saved));
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

bool SavedArchive::readOverlayHeader(Common::SeekableReadStream *stream, Common::String &baseKey, uint32 &baseSize, uint32 &baseCrc, uint32 &movieSize, uint32 &patchCount) {
	stream->seek(0);
	if (stream->readUint32BE() != kOverlayTag)
		return false;
	uint32 keyLen = stream->readUint32BE();
	if (keyLen > 4096 || stream->eos())
		return false;
	baseKey = stream->readString(0, keyLen);
	baseSize = stream->readUint32BE();
	baseCrc = stream->readUint32BE();
	movieSize = stream->readUint32BE();
	patchCount = stream->readUint32BE();
	return !stream->eos() && !stream->err() && !baseKey.empty();
}

// The movie an overlay save stands for: its game file with the patches
Common::SeekableReadStream *SavedArchive::buildOverlay(Common::SeekableReadStream *save) const {
	Common::String baseKey;
	uint32 baseSize, baseCrc, movieSize, patchCount;
	if (!readOverlayHeader(save, baseKey, baseSize, baseCrc, movieSize, patchCount))
		return nullptr;

	uint32 size;
	byte *base = readOriginal(baseKey, size);
	if (!base)
		return nullptr;
	if (size != baseSize || Common::CRC32().crcFast(base, size) != baseCrc || movieSize < baseSize) {
		free(base);
		return nullptr;
	}

	byte *data = (byte *)realloc(base, movieSize ? movieSize : 1);
	if (!data) {
		free(base);
		return nullptr;
	}
	memset(data + baseSize, 0, movieSize - baseSize);

	for (uint32 i = 0; i < patchCount; i++) {
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
	{
		Common::ScopedPtr<Common::SeekableReadStream> save(openSave(key));
		Common::String baseKey;
		uint32 baseSize, baseCrc, movieSize, patchCount;
		if (save && readOverlayHeader(save.get(), baseKey, baseSize, baseCrc, movieSize, patchCount))
			return baseKey;
	}
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
bool SavedArchive::isTrusted(const Common::String &key) const {
	TrustMap::const_iterator t = _trusted.find(key);
	if (t != _trusted.end())
		return t->_value;

	const Common::String &fileName = _files.getVal(key);
	Common::String reason;
	Stamp stamp;
	bool stamped = false;

	Common::ScopedPtr<Common::SeekableReadStream> save(g_engine->getSaveFileManager()->openForLoading(fileName));
	// An overlay is checked against the game file it was made from
	Common::String baseKey = key;
	bool overlay = false;
	if (save) {
		uint32 baseSize, baseCrc, movieSize, patchCount;
		overlay = readOverlayHeader(save.get(), baseKey, baseSize, baseCrc, movieSize, patchCount);
		if (!overlay)
			baseKey = key;
	}
	if (save && save->size() >= kStampSize) {
		save->seek(save->size() - 8);
		uint32 stampSize = save->readUint32BE();
		if (save->readUint32BE() == kStampTag && stampSize >= kStampSize && stampSize <= save->size()) {
			save->seek(save->size() - stampSize);
			stamp.version = save->readUint32BE();
			stamp.origSize = save->readUint32BE();
			stamp.origCrc = save->readUint32BE();
			stamped = true;
		}
	}

	if (!save) {
		reason = "it can't be read";
	} else if (overlay && !stamped) {
		reason = "it is damaged";
	} else if (!stamped) {
		// Saves keyed by path were only made by unstamped development
		// builds, some with the faulty writer. Older saves, keyed by
		// name, are kept: they hold players' progress, e.g. save slots.
		if (fileName.contains("%2F") || fileName.contains("%2f"))
			reason = "it was made by an older version of ScummVM";
	} else if (stamp.version < kMinTrustedVersion) {
		reason = "it was made by an older version of ScummVM";
	} else if (overlay || stamp.origSize != kNoOriginal) {
		Stamp now = stampFor(baseKey);
		if (overlay && now.origSize == kNoOriginal)
			reason = Common::String::format("the game file it was made from, '%s', is missing", baseKey.c_str());
		else if (now.origSize != stamp.origSize || now.origCrc != stamp.origCrc)
			reason = "the game file it stands in for has changed";
	}

	if (!reason.empty())
		warning("SavedArchive: ignoring saved movie '%s' for '%s': %s. Delete it to stop this warning", fileName.c_str(), key.c_str(), reason.c_str());
	_trusted[key] = reason.empty();
	return reason.empty();
}

bool SavedArchive::_addFile(const Common::String &fileName) {
	Common::String origFileName = keyFor(fileName);
	if (!_files.contains(origFileName)) {
		debugC(3, kDebugLoading, "Found save file: %s -> %s", fileName.c_str(), origFileName.c_str());

		_files[origFileName] = fileName;
		return true;
	}
	return false;
}

} // End of namespace Director
