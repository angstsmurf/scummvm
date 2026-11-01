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

#include "glk/comprehend/disk_image.h"
#include "common/archive.h"
#include "common/hash-str.h"
#include "common/hashmap.h"

namespace Glk {
namespace Comprehend {
namespace DiskImageFS {

typedef Common::HashMap<Common::String, Common::Array<byte>,
	Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> Disk;

static const char *const ARCHIVE_NAME = "comprehend_disks";

class DiskArchive : public Common::Archive {
public:
	Common::Array<Disk *> _disks;
	int _active = -1;

	~DiskArchive() override {
		for (uint i = 0; i < _disks.size(); ++i)
			delete _disks[i];
	}

	const Common::Array<byte> *find(const Common::String &name) const {
		if (_active >= 0) {
			Disk::const_iterator it = _disks[_active]->find(name);
			if (it != _disks[_active]->end())
				return &it->_value;
		}
		for (uint i = 0; i < _disks.size(); ++i) {
			Disk::const_iterator it = _disks[i]->find(name);
			if (it != _disks[i]->end())
				return &it->_value;
		}
		return nullptr;
	}

	bool hasFile(const Common::Path &path) const override {
		return find(path.baseName()) != nullptr;
	}

	int listMembers(Common::ArchiveMemberList &list) const override {
		int count = 0;
		for (uint i = 0; i < _disks.size(); ++i) {
			for (Disk::const_iterator it = _disks[i]->begin(); it != _disks[i]->end(); ++it) {
				list.push_back(Common::ArchiveMemberPtr(
					new Common::GenericArchiveMember(Common::Path(it->_key), *this)));
				++count;
			}
		}
		return count;
	}

	const Common::ArchiveMemberPtr getMember(const Common::Path &path) const override {
		return Common::ArchiveMemberPtr(new Common::GenericArchiveMember(path, *this));
	}

	Common::SeekableReadStream *createReadStreamForMember(const Common::Path &path) const override {
		const Common::Array<byte> *data = find(path.baseName());
		if (!data)
			return nullptr;
		return new Common::MemoryReadStream(data->data(), data->size());
	}
};

// Owned by SearchMan once mounted
static DiskArchive *s_archive = nullptr;

void beginDisk() {
	if (!s_archive) {
		s_archive = new DiskArchive();
		SearchMan.add(ARCHIVE_NAME, s_archive, 99, true);
	}
	s_archive->_disks.push_back(new Disk());
	if (s_archive->_active < 0)
		s_archive->_active = 0;
}

void add(const Common::String &name, const byte *data, size_t len) {
	if (!s_archive)
		beginDisk();
	Common::Array<byte> &file = s_archive->_disks.back()->getOrCreateVal(name);
	file.resize(len);
	if (len)
		memcpy(file.data(), data, len);
}

bool selectDiskByGameMagic(uint16 magic) {
	if (!s_archive)
		return false;
	for (uint i = 0; i < s_archive->_disks.size(); ++i) {
		Disk::const_iterator it = s_archive->_disks[i]->find("g0");
		if (it != s_archive->_disks[i]->end() && it->_value.size() >= 2 &&
				READ_LE_UINT16(it->_value.data()) == magic) {
			s_archive->_active = (int)i;
			return true;
		}
	}
	return false;
}

bool has(const Common::String &name) {
	return get(name) != nullptr;
}

const Common::Array<byte> *get(const Common::String &name) {
	return s_archive ? s_archive->find(name) : nullptr;
}

bool active() {
	return s_archive && !s_archive->_disks.empty();
}

void clear() {
	if (s_archive) {
		SearchMan.remove(ARCHIVE_NAME);
		s_archive = nullptr;
	}
}

} // namespace DiskImageFS
} // namespace Comprehend
} // namespace Glk
