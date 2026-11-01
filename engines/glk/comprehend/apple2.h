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

#ifndef COMPREHEND_APPLE2_H
#define COMPREHEND_APPLE2_H

#include "glk/comprehend/disk_image.h"

// Apple II disk-image import. The Apple II releases store their data files (g0, RA, OA,
// MA, T0...) inside a ProDOS volume on a disk image, with the game-file directory unlinked
// from the volume root. This module extracts every file (plus companion disk sides found
// alongside it) into the in-memory DiskImageFS, so the engine opens them by name as on DOS.

namespace Glk {
namespace Comprehend {

// True if the name has an Apple II disk image extension (.dsk/.do/.nib/.woz).
bool hasAppleDiskImageExtension(const Common::String &name);

// Load the named image and any sibling disk images in the game directory into
// the DiskImageFS, and install the drawing tables they carry. The opened disk
// is the active one. Returns the number of files registered.
int loadAppleDiskImage(const Common::String &filename);

} // namespace Comprehend
} // namespace Glk

#endif
