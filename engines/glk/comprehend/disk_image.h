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

#ifndef GLK_COMPREHEND_DISK_IMAGE_H
#define GLK_COMPREHEND_DISK_IMAGE_H

#include "common/array.h"
#include "common/file.h"
#include "common/md5.h"
#include "common/memstream.h"
#include "common/serializer.h"
#include "common/str.h"
#include "common/stream.h"
#include "common/util.h"

namespace Glk {
namespace Comprehend {

/**
 * The files extracted from Apple II disk images (apple2.cpp), keyed by
 * case-insensitive name and visible to Common::File through SearchMan. Empty
 * for the DOS releases, whose files are read from the game directory.
 *
 * Each disk side is its own file set. Most games are one dataset spread over
 * several sides, so lookups fall back across every disk. Crimson Crown and
 * Talisman instead carry a different dataset per disk under the same names and
 * switch with selectDiskByGameMagic(); the active disk is consulted first.
 */
namespace DiskImageFS {

void beginDisk();   ///< Start a new disk; later add()s go to it
void add(const Common::String &name, const byte *data, size_t len);

/** Make the disk whose "g0" begins with magic active. False if there is none. */
bool selectDiskByGameMagic(uint16 magic);

bool has(const Common::String &name);
const Common::Array<byte> *get(const Common::String &name);
bool active();      ///< True once any file has been added
void clear();       ///< Forget every disk and leave SearchMan

} // namespace DiskImageFS

} // namespace Comprehend
} // namespace Glk

#endif
