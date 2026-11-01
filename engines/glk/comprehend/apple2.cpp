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

#include "glk/comprehend/apple2.h"
#include "glk/comprehend/graphics_magician.h"
#include "glk/comprehend/graphics_magician_dhgr.h"
#include "glk/comprehend/disk_image.h"

#include "common/algorithm.h"
#include "common/config-manager.h"
#include "common/endian.h"
#include "common/formats/disk_image.h"
#include "common/fs.h"

// Apple II disk-image import for the Comprehend port.

namespace Glk {
namespace Comprehend {

enum {
	kNumTracks = 35,
	kSectorsPerTrack = 16,
	kSectorSize = 256,
	kDskImageSize = kNumTracks * kSectorsPerTrack * kSectorSize,

	// ProDOS volume layout
	kBlockSize = 512,
	kNumBlocks = 280,
	kFirstDirBlock = 2,
	kDirEntriesPerBlock = 13,
	kDirEntryLen = 39,
	kMaxBlocksPerIndex = 256,
	kStorageSeedling = 1,
	kStorageSapling = 2,
	kStorageTree = 3,
	kFileTypeBIN = 6,
	kMaxPlausibleFileSize = 150000
};

bool hasAppleDiskImageExtension(const Common::String &name) {
	return name.hasSuffixIgnoreCase(".woz") || name.hasSuffixIgnoreCase(".dsk") ||
		name.hasSuffixIgnoreCase(".do") || name.hasSuffixIgnoreCase(".nib");
}

/*
 * The ProDOS scan below is based on parts of the CiderPress source by Andy
 * McFadden (https://github.com/fadden/ciderpress).
 */

/**
 * Reads a ProDOS block from a disk image in DOS 3.3 sector order, which is what
 * Common::DiskImage produces whatever the format of the image file.
 */
static bool readBlock(const byte *dsk, uint block, byte *out) {
	// The DOS 3.3 sectors holding the two halves of each of a track's 8 blocks
	static const byte BLOCK_TO_DOS_SECTOR[kSectorsPerTrack] = {
		0, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 15
	};

	if (block >= kNumBlocks)
		return false;

	const uint track = block / (kSectorsPerTrack / 2);
	const uint half = (block % (kSectorsPerTrack / 2)) * 2;
	const byte *trackData = dsk + track * kSectorsPerTrack * kSectorSize;
	memcpy(out, trackData + BLOCK_TO_DOS_SECTOR[half] * kSectorSize, kSectorSize);
	memcpy(out + kSectorSize, trackData + BLOCK_TO_DOS_SECTOR[half + 1] * kSectorSize, kSectorSize);
	return true;
}

/** Copies up to maxCount block numbers out of an index block. */
static bool readIndexBlock(const byte *dsk, uint block, uint16 *list, uint maxCount) {
	byte buf[kBlockSize];
	if (!readBlock(dsk, block, buf))
		return false;

	for (uint i = 0; i < maxCount && i < kMaxBlocksPerIndex; ++i)
		list[i] = buf[i] | ((uint16)buf[i + 256] << 8);
	return true;
}

/** Lists the blocks of a file. Sparse entries stay zero. */
static bool readBlockList(const byte *dsk, uint storageType, uint keyBlock, uint32 eof,
		Common::Array<uint16> &list) {
	const uint count = MAX<uint32>((eof + kBlockSize - 1) / kBlockSize, 1);
	list.clear();
	list.resize(count);
	for (uint i = 0; i < count; ++i)
		list[i] = 0;

	if (storageType == kStorageSeedling) {
		list[0] = keyBlock;
		return true;
	}
	if (storageType == kStorageSapling)
		return readIndexBlock(dsk, keyBlock, list.data(), count);

	byte master[kBlockSize];
	if (!readBlock(dsk, keyBlock, master))
		return false;

	for (uint done = 0, idx = 0; done < count && idx < kMaxBlocksPerIndex;
			done += kMaxBlocksPerIndex, ++idx) {
		const uint indexBlock = master[idx] | ((uint16)master[idx + 256] << 8);
		if (indexBlock && !readIndexBlock(dsk, indexBlock, &list[done], count - done))
			return false;
	}
	return true;
}

/**
 * Does this look like the entry of a Comprehend game file: a ProDOS BIN file
 * with a printable name, of a plausible size, that starts on the disk?
 */
static bool looksLikeGameFileEntry(const byte *entry) {
	const uint storageType = entry[0] >> 4;
	const uint nameLen = entry[0] & 0x0f;

	if (storageType < kStorageSeedling || storageType > kStorageTree || !nameLen)
		return false;
	if (entry[0x10] != kFileTypeBIN)
		return false;

	for (uint i = 0; i < nameLen; ++i) {
		if (entry[1 + i] < 33 || entry[1 + i] >= 127)
			return false;
	}

	const uint32 eof = READ_LE_UINT24(entry + 0x15);
	const uint key = READ_LE_UINT16(entry + 0x11);
	return eof != 0 && eof <= kMaxPlausibleFileSize && key >= kFirstDirBlock && key < kNumBlocks;
}

/**
 * Registers every game file of a disk as a new disk in the DiskImageFS, and
 * returns the number of files.
 *
 * The Penguin/Polarware loader keeps the game's directory unlinked from the
 * ProDOS volume root, so walking the directory tree would miss it. Instead,
 * every block is scanned for what looks like directory entries.
 */
static int registerGameFiles(const byte *dsk) {
	Common::Array<Common::String> seen;
	byte dirBlock[kBlockSize], fileBlock[kBlockSize];
	int added = 0;

	for (uint block = kFirstDirBlock; block < kNumBlocks; ++block) {
		readBlock(dsk, block, dirBlock);

		for (uint i = 0; i < kDirEntriesPerBlock; ++i) {
			const byte *entry = dirBlock + 4 + i * kDirEntryLen;
			if (!looksLikeGameFileEntry(entry))
				continue;

			const Common::String name((const char *)entry + 1, entry[0] & 0x0f);
			if (Common::find(seen.begin(), seen.end(), name) != seen.end())
				continue;

			const uint32 eof = READ_LE_UINT24(entry + 0x15);
			Common::Array<uint16> blocks;
			if (!readBlockList(dsk, entry[0] >> 4, READ_LE_UINT16(entry + 0x11), eof, blocks))
				continue;

			Common::Array<byte> data;
			data.resize(eof);
			for (uint32 done = 0; done < eof; done += kBlockSize) {
				if (readBlock(dsk, blocks[done / kBlockSize], fileBlock))
					memcpy(&data[done], fileBlock, MIN<uint32>(eof - done, kBlockSize));
				else
					memset(&data[done], 0, MIN<uint32>(eof - done, kBlockSize));
			}

			// Each disk side keeps its own file set; multi-disk games (Crimson
			// Crown, Talisman) carry a different dataset per disk under the same
			// names.
			if (!added)
				DiskImageFS::beginDisk();
			DiskImageFS::add(name, data.data(), data.size());
			seen.push_back(name);
			++added;
		}
	}
	return added;
}

/** Decodes an opened disk image and registers the game files it holds. */
static int loadOneImage(Common::DiskImage &image) {
	Common::SeekableReadStream *stream = image.getDiskStream();
	if (!stream || stream->size() != kDskImageSize)
		return 0;  // e.g. a 13-sector disk

	Common::Array<byte> dsk;
	dsk.resize(kDskImageSize);
	stream->seek(0);
	if (stream->read(dsk.data(), kDskImageSize) != kDskImageSize)
		return 0;

	return registerGameFiles(dsk.data());
}

int loadAppleDiskImage(const Common::String &filename) {
	Common::DiskImage primary;
	if (!primary.open(Common::Path(filename)))
		return 0;
	int total = loadOneImage(primary);

	// Pull in the companion disk sides sitting in the game's directory.
	Common::FSNode dir(ConfMan.getPath("path"));
	Common::FSList entries;
	if (dir.getChildren(entries, Common::FSNode::kListFilesOnly)) {
		for (Common::FSList::const_iterator it = entries.begin(); it != entries.end(); ++it) {
			if (it->getName().equalsIgnoreCase(filename) ||
					!hasAppleDiskImageExtension(it->getName()))
				continue;
			Common::DiskImage sibling;
			if (sibling.open(*it))
				total += loadOneImage(sibling);
		}
	}

	// The Talisman boot disk's "T2" file (Graphics Magician) carries the
	// drawing tables (pattern data, fill-colour subindices, brush bitmaps)
	// used by graphics_magician.cpp. Install them now that every side has been
	// loaded; the renderer is otherwise a no-op.
	const Common::Array<byte> *t2 = DiskImageFS::get("t2");
	if (t2)
		gmInstallDrawingTables(t2->data(), t2->size());

	// The "T5" file is the boot disk's double-hi-res ("<D>" mode) interpreter;
	// its drawing tables sit at fixed offsets inside the headerless $4000 image.
	// Install them so the DHGR renderer (graphics_magician_dhgr.cpp) can run when
	// the player toggles double hi-res. Absent on games that ship no <D> mode,
	// in which case the toggle stays unavailable.
	const Common::Array<byte> *t5 = DiskImageFS::get("t5");
	if (t5)
		gmDhgrInstallDrawingTables(t5->data(), t5->size());

	return total;
}

} // namespace Comprehend
} // namespace Glk
