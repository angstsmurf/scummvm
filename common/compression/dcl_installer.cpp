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

#include "common/compression/dcl_installer.h"
#include "common/compression/dcl.h"

#include "common/debug.h"
#include "common/formats/winexe_ne.h"
#include "common/hashmap.h"
#include "common/memstream.h"
#include "common/substream.h"
#include "common/ustr.h"

namespace Common {

/*
 * Layout of the installer executable:
 *
 * - The NE executable of the installer itself.
 * - A NUL-terminated string such as "[20/20]", then the LE32 file offset
 *   of the first entry.
 * - A linked list of entries. Each one has a 0x114-byte header:
 *     0x000  LE32  file offset of the next entry, 0 for the last one
 *     0x004  LE32  flags (0x20 for files of the script, 0x80 for parts of
 *                  the installer itself)
 *     0x008  LE32  DOS time
 *     0x00c  LE32  DOS date
 *     0x010  LE32  size of the payload following this header
 *     0x014  char  NUL-terminated file name, in a 0x100-byte buffer
 *   A compressed payload starts with another 0xe2-byte header, which holds
 *   the size of the DCL stream following it at offset 0xc2. The install
 *   script (*.cfg) is stored uncompressed.
 */

static const uint32 kEntryHeaderSize = 0x114;
static const uint32 kCompressedHeaderSize = 0xe2;

class DCLInstallerArchive : public Archive {
public:
	DCLInstallerArchive(SeekableReadStream *stream);
	~DCLInstallerArchive() override;

	bool load();

	bool hasFile(const Path &path) const override;
	int listMembers(ArchiveMemberList &list) const override;
	const ArchiveMemberPtr getMember(const Path &path) const override;
	SeekableReadStream *createReadStreamForMember(const Path &path) const override;

private:
	struct FileDesc {
		uint32 offset;
		uint32 size;
		bool compressed;
	};

	struct Entry {
		String name;
		FileDesc desc;
	};

	bool findFirstEntry(uint32 &offset);
	void addScriptPaths(const Array<Entry> &entries, HashMap<String, String, IgnoreCase_Hash, IgnoreCase_EqualTo> &destPaths);

	SeekableReadStream *_stream;
	HashMap<Path, FileDesc, Path::IgnoreCase_Hash, Path::IgnoreCase_EqualTo> _files;
};

DCLInstallerArchive::DCLInstallerArchive(SeekableReadStream *stream) : _stream(stream) {
}

DCLInstallerArchive::~DCLInstallerArchive() {
	delete _stream;
}

// The archive follows the last segment or resource of the NE executable
bool DCLInstallerArchive::findFirstEntry(uint32 &offset) {
	NEResources exe;
	if (!exe.loadFromEXE(_stream, DisposeAfterUse::NO))
		return false;
	uint32 end = exe.getImageEnd();

	if (end == NEResources::kInvalidAddress || !_stream->seek(end))
		return false;

	// Skip the version string
	uint i = 0;
	while (_stream->readByte() != 0) {
		if (++i > 32 || _stream->eos())
			return false;
	}

	offset = _stream->readUint32LE();
	return !_stream->eos() && !_stream->err();
}

// The script names the destination of each file, such as
// FILE: "SC_DATA\Main_eng.dxr", overwrite=yes, From="D:\Safecracker\SC_DATA\Main_eng.dxr"
void DCLInstallerArchive::addScriptPaths(const Array<Entry> &entries, HashMap<String, String, IgnoreCase_Hash, IgnoreCase_EqualTo> &destPaths) {
	for (const Entry &entry : entries) {
		if (entry.desc.compressed || !entry.name.hasSuffixIgnoreCase(".cfg"))
			continue;

		SeekableSubReadStream script(_stream, entry.desc.offset, entry.desc.offset + entry.desc.size);
		while (!script.eos() && !script.err()) {
			String line = script.readLine();
			line.trim();
			if (!line.hasPrefixIgnoreCase("FILE:"))
				continue;

			size_t start = line.findFirstOf('"');
			if (start == String::npos)
				continue;
			size_t end = line.findFirstOf('"', start + 1);
			if (end == String::npos)
				continue;
			String destPath = line.substr(start + 1, end - start - 1);

			// Paths in system folders ("$WinSysDir$\Qtvrw32.qtc") are kept at the root
			if (destPath.hasPrefix("$")) {
				size_t separator = destPath.findFirstOf('\\');
				destPath = (separator == String::npos) ? String() : destPath.substr(separator + 1);
			}

			size_t separator = destPath.findLastOf('\\');
			String baseName = (separator == String::npos) ? destPath : destPath.substr(separator + 1);
			if (!baseName.empty())
				destPaths[baseName] = destPath;
		}
	}
}

bool DCLInstallerArchive::load() {
	uint32 offset;
	if (!findFirstEntry(offset)) {
		debug(1, "DCL installer: Not an NE executable with an archive");
		return false;
	}

	Array<Entry> entries;
	uint32 streamSize = _stream->size();
	while (offset != 0) {
		if (offset > streamSize - kEntryHeaderSize || !_stream->seek(offset)) {
			debug(1, "DCL installer: Entry offset %u is out of range", offset);
			return false;
		}

		byte header[kEntryHeaderSize];
		if (_stream->read(header, kEntryHeaderSize) != kEntryHeaderSize)
			return false;

		uint32 next = READ_LE_UINT32(header);
		uint32 payloadSize = READ_LE_UINT32(header + 0x10);
		uint32 payloadOffset = offset + kEntryHeaderSize;
		if (payloadSize > streamSize - payloadOffset || (next != 0 && next != payloadOffset + payloadSize)) {
			debug(1, "DCL installer: Entry at %u is invalid", offset);
			return false;
		}

		Entry entry;
		entry.name = String((const char *)header + 0x14, strnlen((const char *)header + 0x14, kEntryHeaderSize - 0x14));
		entry.desc.offset = payloadOffset;
		entry.desc.size = payloadSize;
		entry.desc.compressed = false;

		if (payloadSize >= kCompressedHeaderSize) {
			byte compressedHeader[kCompressedHeaderSize];
			if (_stream->read(compressedHeader, kCompressedHeaderSize) != kCompressedHeaderSize)
				return false;
			if (READ_LE_UINT32(compressedHeader + 0xc2) == payloadSize - kCompressedHeaderSize) {
				entry.desc.offset += kCompressedHeaderSize;
				entry.desc.size -= kCompressedHeaderSize;
				entry.desc.compressed = true;
			}
		}

		entries.push_back(entry);
		offset = next;
	}

	if (entries.empty())
		return false;

	HashMap<String, String, IgnoreCase_Hash, IgnoreCase_EqualTo> destPaths;
	addScriptPaths(entries, destPaths);

	for (const Entry &entry : entries) {
		String path = entry.name;
		if (destPaths.contains(entry.name))
			path = destPaths[entry.name];
		path = U32String(path, kWindows1252).encode();
		_files[Path(path, '\\')] = entry.desc;
		debug(2, "DCL installer: %s at %u, %u bytes%s", path.c_str(), entry.desc.offset, entry.desc.size, entry.desc.compressed ? ", compressed" : "");
	}

	return true;
}

bool DCLInstallerArchive::hasFile(const Path &path) const {
	return _files.contains(path);
}

int DCLInstallerArchive::listMembers(ArchiveMemberList &list) const {
	for (const auto &file : _files)
		list.push_back(ArchiveMemberPtr(new GenericArchiveMember(file._key, *this)));
	return _files.size();
}

const ArchiveMemberPtr DCLInstallerArchive::getMember(const Path &path) const {
	if (!hasFile(path))
		return ArchiveMemberPtr();
	return ArchiveMemberPtr(new GenericArchiveMember(path, *this));
}

SeekableReadStream *DCLInstallerArchive::createReadStreamForMember(const Path &path) const {
	if (!_files.contains(path))
		return nullptr;

	const FileDesc &desc = _files[path];
	SeekableSubReadStream data(_stream, desc.offset, desc.offset + desc.size);
	if (!desc.compressed)
		return data.readStream(desc.size);

	// The unpacked size isn't stored, so the DCL stream's end marker ends it
	SeekableReadStream *file = decompressDCL(&data);
	if (!file)
		warning("DCL installer: Failed to decompress %s", path.toString('\\').c_str());
	return file;
}

Archive *createDCLInstallerArchive(SeekableReadStream *stream) {
	DCLInstallerArchive *archive = new DCLInstallerArchive(stream);
	if (!archive->load()) {
		delete archive;
		return nullptr;
	}
	return archive;
}

} // End of namespace Common
