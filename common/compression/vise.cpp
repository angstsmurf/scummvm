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

#include "common/compression/vise.h"

#include "common/debug.h"
#include "common/hashmap.h"
#include "common/macresman.h"
#include "common/memstream.h"
#include "common/compression/deflate.h"

// Installer VISE archive loader.
//
// Supported versions:
//
// Macintosh:
//     Installer VISE 3.5 Lite
//     Installer VISE 3.6 Lite
//
// Windows:
//     VISE 3.x (vise32ex.dll)

namespace Common {

class MacVISEArchive : public Common::Archive {
private:
	struct FileDesc {
		FileDesc();

		uint32 type;
		uint32 creator;
		uint32 compressedDataSize;
		uint32 uncompressedDataSize;
		uint32 compressedResSize;
		uint32 uncompressedResSize;
		uint32 positionInArchive;

		uint16 containingDirectory;

		Common::String name;
		Common::Path fullPath;
	};

	struct DirectoryDesc {
		uint16 containingDirectory;

		Common::String name;
		Common::Path fullPath;
	};

	class ArchiveMember : public Common::ArchiveMember {
	public:
		ArchiveMember(Common::SeekableReadStream *archiveStream, const FileDesc *fileDesc);

		Common::SeekableReadStream *createReadStream() const override;
		Common::SeekableReadStream *createReadStreamForAltStream(Common::AltStreamType altStreamType) const override;
		Common::String getName() const override;
		Common::Path getPathInArchive() const override;
		Common::String getFileName() const override;
		bool isInMacArchive() const override;

	private:
		Common::SeekableReadStream *createReadStreamForDataStream(bool isResFork) const;

		Common::SeekableReadStream *_archiveStream;
		const FileDesc *_fileDesc;
	};

public:
	explicit MacVISEArchive(Common::SeekableReadStream *archiveStream);

	bool loadCatalog();

	const FileDesc *getFileDesc(const Common::Path &path) const;

	bool hasFile(const Common::Path &path) const override;
	int listMembers(Common::ArchiveMemberList &list) const override;
	const Common::ArchiveMemberPtr getMember(const Common::Path &path) const override;
	Common::SeekableReadStream *createReadStreamForMember(const Common::Path &path) const override;
	Common::SeekableReadStream *createReadStreamForMemberAltStream(const Common::Path &path, Common::AltStreamType altStreamType) const override;
	char getPathSeparator() const override;

private:
	bool getFileDescIndex(const Common::Path &path, uint &outIndex) const;

	Common::SeekableReadStream *_archiveStream;
	Common::Array<FileDesc> _fileDescs;
	Common::Array<DirectoryDesc> _directoryDescs;
};

MacVISEArchive::ArchiveMember::ArchiveMember(Common::SeekableReadStream *archiveStream, const FileDesc *fileDesc)
	: _archiveStream(archiveStream), _fileDesc(fileDesc) {
}

Common::SeekableReadStream *MacVISEArchive::ArchiveMember::createReadStream() const {
	return createReadStreamForDataStream(false);
}

Common::SeekableReadStream *MacVISEArchive::ArchiveMember::createReadStreamForAltStream(Common::AltStreamType altStreamType) const {
	if (altStreamType == AltStreamType::MacFinderInfo) {
		Common::MacFinderInfoData *finfoData = static_cast<Common::MacFinderInfoData *>(malloc(sizeof(Common::MacFinderInfoData)));

		if (!finfoData)
			return nullptr;

		Common::MacFinderInfo finfo;
		finfo.type = _fileDesc->type;
		finfo.creator = _fileDesc->creator;

		*finfoData = finfo.toData();

		return new Common::MemoryReadStream(reinterpret_cast<const byte *>(finfoData), sizeof(Common::MacFinderInfoData), DisposeAfterUse::YES);
	}

	if (altStreamType == AltStreamType::MacResourceFork)
		return createReadStreamForDataStream(true);

	return nullptr;
}

Common::SeekableReadStream *MacVISEArchive::ArchiveMember::createReadStreamForDataStream(bool isResFork) const {
	static const uint8 vl3DeobfuscationTable[] = {
		0x6a, 0xb7, 0x36, 0xec, 0x15, 0xd9, 0xc8, 0x73, 0xe8, 0x38, 0x9a, 0xdf, 0x21, 0x25, 0xd0, 0xcc,
		0xfd, 0xdc, 0x16, 0xd7, 0xe3, 0x43, 0x05, 0xc5, 0x8f, 0x48, 0xda, 0xf2, 0x3f, 0x10, 0x23, 0x6c,
		0x77, 0x7c, 0xf9, 0xa0, 0xa3, 0xe9, 0xed, 0x46, 0x8b, 0xd8, 0xac, 0x54, 0xce, 0x2d, 0x19, 0x5e,
		0x6d, 0x7d, 0x87, 0x5d, 0xfa, 0x5b, 0x9b, 0xe0, 0xc7, 0xee, 0x9f, 0x52, 0xa9, 0xb9, 0x0a, 0xd1,
		0xfe, 0x78, 0x76, 0x4a, 0x3d, 0x44, 0x5a, 0x96, 0x90, 0x1f, 0x26, 0x9d, 0x58, 0x1b, 0x8e, 0x57,
		0x59, 0xc3, 0x0b, 0x6b, 0xfc, 0x1d, 0xe6, 0xa2, 0x7f, 0x92, 0x4f, 0x40, 0xb4, 0x06, 0x72, 0x4d,
		0xf4, 0x34, 0xaa, 0xd2, 0x49, 0xad, 0xef, 0x22, 0x1a, 0xb5, 0xba, 0xbf, 0x29, 0x68, 0x89, 0x93,
		0x3e, 0x32, 0x04, 0xf5, 0xde, 0xe1, 0x6f, 0xfb, 0x67, 0xe4, 0x7e, 0x08, 0xaf, 0xf0, 0xab, 0x41,
		0x82, 0xea, 0x50, 0x0f, 0x2a, 0xc6, 0x35, 0xb3, 0xa8, 0xca, 0xe5, 0x4c, 0x45, 0x8a, 0x97, 0xae,
		0xd6, 0x66, 0x27, 0x53, 0xc9, 0x1c, 0x3c, 0x03, 0x99, 0xc1, 0x09, 0x2e, 0x69, 0x37, 0x8d, 0x2f,
		0x60, 0xc2, 0xa6, 0x18, 0x4e, 0x7a, 0xb8, 0xcf, 0xa7, 0x3a, 0x17, 0xd5, 0x9e, 0xf1, 0x84, 0x51,
		0x0d, 0xa4, 0x64, 0xc4, 0x1e, 0xb1, 0x30, 0x98, 0xbb, 0x79, 0x01, 0xf6, 0x62, 0x0e, 0xb2, 0x63,
		0x91, 0xcb, 0xff, 0x80, 0x71, 0xe7, 0xd4, 0x00, 0xdb, 0x75, 0x2c, 0xbd, 0x39, 0x33, 0x94, 0xbc,
		0x8c, 0x3b, 0xb6, 0x20, 0x85, 0x24, 0x88, 0x2b, 0x70, 0x83, 0x6e, 0x7b, 0x9c, 0xbe, 0x14, 0x47,
		0x65, 0x4b, 0x56, 0x81, 0xf8, 0x12, 0x11, 0x28, 0xeb, 0x55, 0x74, 0xa1, 0x31, 0xf7, 0xb0, 0x13,
		0x86, 0xdd, 0x5f, 0x42, 0xd3, 0x02, 0x61, 0x95, 0x0c, 0x5c, 0xa5, 0xcd, 0xc0, 0x07, 0xe2, 0xf3,
	};

	uint32 uncompressedSize = isResFork ? _fileDesc->uncompressedResSize : _fileDesc->uncompressedDataSize;
	uint32 compressedSize = isResFork ? _fileDesc->compressedResSize : _fileDesc->compressedDataSize;
	uint32 filePosition = _fileDesc->positionInArchive;

	if (uncompressedSize == 0 && !isResFork) {
		// Always return a stream for the data fork, even if it's empty
		return new Common::MemoryReadStream(nullptr, 0, DisposeAfterUse::NO);
	}

	if (isResFork)
		filePosition += _fileDesc->compressedDataSize;

	if (uncompressedSize == 0)
		return nullptr;

	Common::Array<byte> compressedData;
	compressedData.resize(compressedSize);

	_archiveStream->seek(filePosition, SEEK_SET);
	if (_archiveStream->read(&compressedData[0], compressedSize) != compressedSize)
		return nullptr;

	// Undo byte swapping
	for (uint i = 1; i < compressedSize; i += 2) {
		byte temp = compressedData[i];
		compressedData[i] = compressedData[i - 1];
		compressedData[i - 1] = temp;
	}

	// Undo obfuscation
	for (byte &b : compressedData)
		b = vl3DeobfuscationTable[b];

	byte *decompressedData = static_cast<byte *>(malloc(uncompressedSize));
	if (!decompressedData)
		return nullptr;


	// WARNING/TODO: Based on reverse engineering of the "Dcmp" resource from the installer, which contains the decompression code,
	// the bitstream format is usually just deflate, however there is one difference: Stored blocks are flushed to a 2-byte boundary
	// instead of 1-byte boundary, because the decompressor reads 2 bytes at a time.  This doesn't usually matter because stored
	// blocks are very rare in practice on compressible data, and small files with only 1 compressible block are already 2-byte
	// aligned on the first block.
	//
	// If this turns out to be significant, then this will need to be updated to pass information to the deflate decompressor to
	// handle the non-standard behavior.
	if (!Common::inflateZlibHeaderless(decompressedData, uncompressedSize, &compressedData[0], compressedSize)) {
		free(decompressedData);
		return nullptr;
	}

	return new Common::MemoryReadStream(decompressedData, uncompressedSize, DisposeAfterUse::YES);
}

Common::String MacVISEArchive::ArchiveMember::getName() const {
	return getPathInArchive().getLastComponent().toString(':');
}

Common::Path MacVISEArchive::ArchiveMember::getPathInArchive() const {
	return _fileDesc->fullPath;
}

Common::String MacVISEArchive::ArchiveMember::getFileName() const {
	return _fileDesc->name;
}

bool MacVISEArchive::ArchiveMember::isInMacArchive() const {
	return true;
}

MacVISEArchive::FileDesc::FileDesc() : type(0), creator(0), compressedDataSize(0), uncompressedDataSize(0), compressedResSize(0), uncompressedResSize(0), positionInArchive(0) {
}

MacVISEArchive::MacVISEArchive(Common::SeekableReadStream *archiveStream) : _archiveStream(archiveStream) {
}

bool MacVISEArchive::loadCatalog() {
	uint8 vl3Header[44];
	if (_archiveStream->read(vl3Header, 44) != 44 || memcmp(vl3Header, "SVCT", 4)) {
		debug(1, "Failed to read VISE header");
		return false;
	}

	uint32 catalogPosition = READ_BE_UINT32(vl3Header + 36);

	uint32 archiveVersion = READ_BE_UINT32(vl3Header + 16);

	if (archiveVersion == 0x80010202)
		debug(3, "Detected VISE archive as 3.5 Lite");
	else if (archiveVersion == 0x80010300)
		debug(3, "Detected VISE archive as 3.6 Lite");
	else {
		debug(1, "Unrecognized VISE archive version");
		return false;
	}

	if (!_archiveStream->seek(catalogPosition))
		error("Failed to seek to VISE catalog");

	uint8 vl3Catalog[20];
	if (_archiveStream->read(vl3Catalog, 20) != 20 || memcmp(vl3Catalog, "CVCT", 4)) {
		debug(1, "Failed to read VISE catalog");
		return false;
	}

	uint16 numEntries = READ_BE_UINT16(vl3Catalog + 16);

	for (uint16 i = 0; i < numEntries; i++) {
		uint8 entryMagic[4];
		if (_archiveStream->read(entryMagic, 4) != 4 || memcmp(entryMagic + 1, "VCT", 3)) {
			debug(1, "Failed to read VISE catalog item");
			return false;
		}

		if (entryMagic[0] == 'D') {
			uint8 directoryData[78];
			if (_archiveStream->read(directoryData, 78) != 78) {
				debug(1, "Failed to read VISE directory");
				return false;
			}

			// 3.6 Lite archives have an additional 6 bytes before the name in directory entries
			if (archiveVersion == 0x80010300)
				_archiveStream->seek(6, SEEK_CUR);

			DirectoryDesc desc;
			desc.containingDirectory = READ_BE_UINT16(directoryData + 68);

			uint8 nameLength = directoryData[76];

			if (nameLength > 0) {
				char fileNameChars[256];
				if (_archiveStream->read(fileNameChars, nameLength) != nameLength) {
					debug(1, "Failed to read VISE directory name");
					return false;
				}
				desc.name = Common::String(fileNameChars, nameLength);
			}

			_directoryDescs.push_back(desc);
		} else if (entryMagic[0] == 'F') {
			uint8 fileData[120];
			if (_archiveStream->read(fileData, 120) != 120) {
				debug(1, "Failed to read VISE file");
				return false;
			}

			FileDesc desc;
			desc.type = READ_BE_UINT32(fileData + 40);
			desc.creator = READ_BE_UINT32(fileData + 44);
			desc.compressedDataSize = READ_BE_UINT32(fileData + 64);
			desc.uncompressedDataSize = READ_BE_UINT32(fileData + 68);
			desc.compressedResSize = READ_BE_UINT32(fileData + 72);
			desc.uncompressedResSize = READ_BE_UINT32(fileData + 76);
			desc.containingDirectory = READ_BE_UINT16(fileData + 92);
			desc.positionInArchive = READ_BE_UINT32(fileData + 96);

			uint8 nameLength = fileData[118];

			if (nameLength > 0) {
				char fileNameChars[256];
				if (_archiveStream->read(fileNameChars, nameLength) != nameLength) {
					debug(1, "Failed to read VISE file name");
					return false;
				}
				desc.name = Common::String(fileNameChars, nameLength);
			}

			_fileDescs.push_back(desc);
		} else {
			debug(1, "Unknown VISE catalog entry item type");
			return false;
		}
	}

	// Generate full paths
	for (DirectoryDesc &dirDesc : _directoryDescs) {
		if (dirDesc.containingDirectory == 0)
			dirDesc.fullPath = dirDesc.name;
		else {
			if (dirDesc.containingDirectory > _directoryDescs.size())
				error("VISE 3 containing directory index was invalid");

			dirDesc.fullPath = _directoryDescs[dirDesc.containingDirectory - 1].fullPath.appendComponent(dirDesc.name);
		}
	}

	for (FileDesc &fileDesc : _fileDescs) {
		if (fileDesc.containingDirectory == 0)
			fileDesc.fullPath = fileDesc.name;
		else {
			if (fileDesc.containingDirectory > _directoryDescs.size())
				error("VISE 3 containing directory index was invalid");

			fileDesc.fullPath = _directoryDescs[fileDesc.containingDirectory - 1].fullPath.appendComponent(fileDesc.name);
		}
	}

	return true;
}

const MacVISEArchive::FileDesc *MacVISEArchive::getFileDesc(const Common::Path &path) const {
	for (const FileDesc &desc : _fileDescs) {
		if (desc.fullPath == path)
			return &desc;
	}

	return nullptr;
}

bool MacVISEArchive::hasFile(const Common::Path &path) const {
	uint index = 0;
	return getFileDescIndex(path, index);
}

int MacVISEArchive::listMembers(Common::ArchiveMemberList &list) const {
	int numMembers = 0;
	for (uint fileIndex = 0; fileIndex < _fileDescs.size(); fileIndex++) {
		const FileDesc &desc = _fileDescs[fileIndex];

		list.push_back(Common::ArchiveMemberPtr(new ArchiveMember(_archiveStream, &desc)));
		numMembers++;
	}
	return numMembers;
}

const Common::ArchiveMemberPtr MacVISEArchive::getMember(const Common::Path &path) const {
	uint descIndex = 0;
	if (!getFileDescIndex(path, descIndex))
		return nullptr;

	return Common::ArchiveMemberPtr(new ArchiveMember(_archiveStream, &_fileDescs[descIndex]));
}

Common::SeekableReadStream *MacVISEArchive::createReadStreamForMember(const Common::Path &path) const {
	Common::ArchiveMemberPtr archiveMember = getMember(path);
	if (!archiveMember)
		return nullptr;

	return archiveMember->createReadStream();
}

Common::SeekableReadStream *MacVISEArchive::createReadStreamForMemberAltStream(const Common::Path &path, Common::AltStreamType altStreamType) const {
	Common::ArchiveMemberPtr archiveMember = getMember(path);
	if (!archiveMember)
		return nullptr;

	return archiveMember->createReadStreamForAltStream(altStreamType);
}

char MacVISEArchive::getPathSeparator() const {
	return ':';
}

bool MacVISEArchive::getFileDescIndex(const Common::Path &path, uint &outIndex) const {
	for (uint descIndex = 0; descIndex < _fileDescs.size(); descIndex++) {
		const FileDesc &desc = _fileDescs[descIndex];

		if (desc.fullPath == path) {
			outIndex = descIndex;
			return true;
		}
	}

	return false;
}

Common::Archive *createMacVISEArchive(Common::SeekableReadStream *stream) {
	MacVISEArchive *archive = new MacVISEArchive(stream);
	if (!archive->loadCatalog()) {
		delete archive;
		return nullptr;
	}

	return archive;
}

// Windows Installer VISE
//
// The installer is a PE executable with the archive appended as an overlay.
// The file ends with "ESIV" followed by the offset of the overlay, which also
// starts with "ESIV". The overlay holds the installer support files, the
// installer script and the compressed file data.
//
// The script is a list of items, each starting with a 32-bit type. Only
// folders (type 1) and files (type 2) are parsed here. The other item types
// are skipped by scanning for the next folder or file item that parses
// cleanly. The item layouts come from vise32ex.dll (VISE 3.x).
//
// File data is raw deflate over byte-swapped 16-bit words.

class WinVISEArchive : public Common::Archive {
public:
	explicit WinVISEArchive(Common::SeekableReadStream *stream);
	~WinVISEArchive() override;

	bool loadCatalog();

	bool hasFile(const Common::Path &path) const override;
	int listMembers(Common::ArchiveMemberList &list) const override;
	const Common::ArchiveMemberPtr getMember(const Common::Path &path) const override;
	Common::SeekableReadStream *createReadStreamForMember(const Common::Path &path) const override;

private:
	enum ItemType {
		kItemFolder = 1,
		kItemFile = 2
	};

	struct Item {
		Item() : type(0), id(0), parent(0), compressed(0), installMethod(0), size(0), compressedSize(0), offset(0) {}

		uint32 type;
		uint32 id;
		uint32 parent;
		Common::String name;

		// Files only
		uint32 compressed;
		byte installMethod;
		uint32 size;
		uint32 compressedSize;
		uint32 offset;
	};

	struct FileDesc {
		uint32 size;
		uint32 compressedSize;
		uint32 offset;
	};

	class ScriptReader {
	public:
		ScriptReader(const byte *data, uint32 size, uint32 pos) : _data(data), _size(size), _pos(pos), _err(false) {}

		bool err() const { return _err; }
		uint32 pos() const { return _pos; }

		void skip(uint32 count) {
			if (_err || count > _size - _pos)
				_err = true;
			else
				_pos += count;
		}

		byte readByte() {
			if (_err || _pos >= _size) {
				_err = true;
				return 0;
			}
			return _data[_pos++];
		}

		uint16 readUint16() {
			if (_err || _size - _pos < 2) {
				_err = true;
				return 0;
			}
			uint16 value = READ_LE_UINT16(_data + _pos);
			_pos += 2;
			return value;
		}

		uint32 readUint32() {
			if (_err || _size - _pos < 4) {
				_err = true;
				return 0;
			}
			uint32 value = READ_LE_UINT32(_data + _pos);
			_pos += 4;
			return value;
		}

		// 16-bit length followed by the characters
		void skipString(uint maxLength) {
			uint16 length = readUint16();
			if (length > maxLength)
				_err = true;
			else
				skip(length);
		}

		// Item name, followed by its translations
		Common::String readName() {
			uint16 length = readUint16();
			if (_err || length == 0 || length > 259 || length > _size - _pos) {
				_err = true;
				return Common::String();
			}

			Common::String name(reinterpret_cast<const char *>(_data + _pos), length);
			_pos += length;

			for (uint i = 0; i < length; i++) {
				byte c = name[i];
				if (c < 0x20 || strchr("\\/:*?\"<>|", c)) {
					_err = true;
					return Common::String();
				}
			}

			uint16 numTranslations = readUint16();
			if (numTranslations > 64)
				_err = true;

			for (uint i = 0; i < numTranslations && !_err; i++) {
				readUint16(); // Language
				skipString(259);
			}

			return name;
		}

	private:
		const byte *_data;
		uint32 _size;
		uint32 _pos;
		bool _err;
	};

	bool readItem(ScriptReader &reader, Item &item) const;
	static bool readItemHeader(ScriptReader &reader, Item &item);

	Common::SeekableReadStream *_stream;
	uint32 _overlayOffset;
	uint32 _overlaySize;

	typedef Common::HashMap<Common::Path, FileDesc, Common::Path::IgnoreCase_Hash, Common::Path::IgnoreCase_EqualTo> FileMap;
	FileMap _files;
};

WinVISEArchive::WinVISEArchive(Common::SeekableReadStream *stream) : _stream(stream), _overlayOffset(0), _overlaySize(0) {
}

WinVISEArchive::~WinVISEArchive() {
	delete _stream;
}

bool WinVISEArchive::readItemHeader(ScriptReader &reader, Item &item) {
	item.id = reader.readUint32();
	item.parent = reader.readUint32();

	if (item.id == 0 || item.id >= 0x100000 || item.parent >= 0x100000 || item.parent == item.id)
		return false;

	reader.skip(4 + 8 + 6 + 1);

	uint16 numConditions = reader.readUint16();
	if (numConditions > 64)
		return false;
	reader.skip(numConditions * 5);

	reader.skip(1);

	for (uint list = 0; list < 2 && !reader.err(); list++) {
		uint16 count = reader.readUint16();
		if (count > 64)
			return false;

		for (uint i = 0; i < count && !reader.err(); i++) {
			reader.skipString(1024);
			reader.skip(4);
			reader.skipString(1024);
			reader.skipString(1024);
		}
	}

	// Two 256-bit masks and a flag
	reader.skip(32 + 32 + 1);

	return !reader.err();
}

bool WinVISEArchive::readItem(ScriptReader &reader, Item &item) const {
	item.type = reader.readUint32();
	if (item.type != kItemFolder && item.type != kItemFile)
		return false;

	if (!readItemHeader(reader, item))
		return false;

	item.name = reader.readName();
	reader.skipString(0x7fff); // Description
	if (reader.err())
		return false;

	if (item.type == kItemFolder) {
		reader.skip(4 + 1 + 4);
		reader.skipString(1024);
		reader.skip(1 + 1 + 1);
		reader.skipString(1024);
		reader.skip(4);
		return !reader.err();
	}

	item.size = reader.readUint32();
	item.compressedSize = reader.readUint32();
	item.compressed = reader.readUint32();
	item.offset = reader.readUint32();
	reader.skip(4); // Attributes

	if (item.compressed > 1 || (item.compressed && (item.offset > _overlaySize || item.compressedSize > _overlaySize - item.offset)))
		return false;

	reader.skip(3 * 8); // Creation, access and modification time
	reader.skip(2 + 2 + 1 + 4);
	reader.skipString(1024);
	reader.skip(4 + 1 + 1 + 4 + 4 + 4 + 4 + 4);

	item.installMethod = reader.readByte();
	reader.skipString(1024);
	reader.skip(1);
	if (item.installMethod == 4)
		reader.skip(16);
	reader.skip(1 + 4 + 4);

	return !reader.err();
}

bool WinVISEArchive::loadCatalog() {
	uint32 fileSize = _stream->size();
	if (fileSize < 8)
		return false;

	_stream->seek(fileSize - 8);
	if (_stream->readUint32BE() != MKTAG('E', 'S', 'I', 'V'))
		return false;

	_overlayOffset = _stream->readUint32LE();
	if (_overlayOffset >= fileSize - 8)
		return false;

	_overlaySize = fileSize - _overlayOffset;

	_stream->seek(_overlayOffset);
	if (_stream->readUint32BE() != MKTAG('E', 'S', 'I', 'V'))
		return false;

	// The script comes after the support files, which are small. File data
	// starts after the script, so stop scanning once the first file data
	// offset is reached.
	const uint32 kMaxScanSize = 4 * 1024 * 1024;
	uint32 scanSize = MIN<uint32>(_overlaySize, kMaxScanSize);

	byte *script = (byte *)malloc(scanSize);
	if (!script)
		return false;

	_stream->seek(_overlayOffset);
	if (_stream->read(script, scanSize) != scanSize) {
		free(script);
		return false;
	}

	Common::Array<Item> items;
	uint32 scanEnd = scanSize;

	for (uint32 pos = 4; pos + 4 <= scanEnd;) {
		uint32 type = READ_LE_UINT32(script + pos);
		if (type != kItemFolder && type != kItemFile) {
			pos++;
			continue;
		}

		ScriptReader reader(script, scanSize, pos);
		Item item;
		if (!readItem(reader, item)) {
			pos++;
			continue;
		}

		if (item.type == kItemFile && item.compressed)
			scanEnd = MIN(scanEnd, item.offset);

		items.push_back(item);
		pos = reader.pos();
	}

	free(script);

	Common::HashMap<uint32, uint> idToItem;
	for (uint i = 0; i < items.size(); i++) {
		if (!idToItem.contains(items[i].id))
			idToItem[items[i].id] = i;
	}

	for (const Item &item : items) {
		if (item.type != kItemFile)
			continue;

		// Install methods 2 and 4 refer to files outside the archive
		if (!item.compressed || item.installMethod == 2 || item.installMethod == 4) {
			debug(1, "WinVISEArchive: Skipping file '%s' (compressed %u, install method %u)", item.name.c_str(), item.compressed, item.installMethod);
			continue;
		}

		Common::Path path(item.name, Common::Path::kNoSeparator);
		uint32 parent = item.parent;
		for (uint depth = 0; parent != 0 && depth < 64; depth++) {
			Common::HashMap<uint32, uint>::const_iterator it = idToItem.find(parent);
			if (it == idToItem.end() || items[it->_value].type != kItemFolder)
				break;

			const Item &folder = items[it->_value];
			path = Common::Path(folder.name, Common::Path::kNoSeparator).join(path);
			parent = folder.parent;
		}

		if (_files.contains(path)) {
			debug(1, "WinVISEArchive: Duplicate file '%s'", path.toString().c_str());
			continue;
		}

		FileDesc desc;
		desc.size = item.size;
		desc.compressedSize = item.compressedSize;
		desc.offset = _overlayOffset + item.offset;
		_files[path] = desc;

		debug(2, "WinVISEArchive: '%s' size %u compressed %u at 0x%x", path.toString().c_str(), desc.size, desc.compressedSize, desc.offset);
	}

	return !_files.empty();
}

bool WinVISEArchive::hasFile(const Common::Path &path) const {
	return _files.contains(path);
}

int WinVISEArchive::listMembers(Common::ArchiveMemberList &list) const {
	for (const auto &file : _files)
		list.push_back(getMember(file._key));

	return _files.size();
}

const Common::ArchiveMemberPtr WinVISEArchive::getMember(const Common::Path &path) const {
	return Common::ArchiveMemberPtr(new Common::GenericArchiveMember(path, *this));
}

Common::SeekableReadStream *WinVISEArchive::createReadStreamForMember(const Common::Path &path) const {
	FileMap::const_iterator it = _files.find(path);
	if (it == _files.end())
		return nullptr;

	const FileDesc &desc = it->_value;

	// Pad to a whole number of 16-bit words
	uint32 paddedSize = desc.compressedSize + (desc.compressedSize & 1);
	byte *compressed = (byte *)malloc(paddedSize);
	byte *decompressed = (byte *)malloc(desc.size ? desc.size : 1);
	if (!compressed || !decompressed) {
		free(compressed);
		free(decompressed);
		return nullptr;
	}

	compressed[paddedSize - 1] = 0;
	_stream->seek(desc.offset);
	if (_stream->read(compressed, desc.compressedSize) != desc.compressedSize) {
		warning("WinVISEArchive: Failed to read '%s'", path.toString().c_str());
		free(compressed);
		free(decompressed);
		return nullptr;
	}

	for (uint32 i = 0; i < paddedSize; i += 2)
		SWAP(compressed[i], compressed[i + 1]);

	uint decompressedSize = desc.size;
	bool ok = Common::inflateZlibHeaderless(decompressed, &decompressedSize, compressed, paddedSize);
	free(compressed);

	if (!ok || decompressedSize != desc.size) {
		warning("WinVISEArchive: Failed to decompress '%s'", path.toString().c_str());
		free(decompressed);
		return nullptr;
	}

	return new Common::MemoryReadStream(decompressed, desc.size, DisposeAfterUse::YES);
}

Common::Archive *createWinVISEArchive(Common::SeekableReadStream *stream) {
	WinVISEArchive *archive = new WinVISEArchive(stream);
	if (!archive->loadCatalog()) {
		delete archive;
		return nullptr;
	}

	return archive;
}

} // End of namespace Common
