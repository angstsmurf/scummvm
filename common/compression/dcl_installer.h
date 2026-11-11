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

#ifndef COMMON_DCL_INSTALLER_H
#define COMMON_DCL_INSTALLER_H

/**
 * @file
 * Reader for a 16-bit Windows installer that stores its files with PKWARE DCL, used in engines:
 * - director
 */

#include "common/archive.h"

namespace Common {

/**
 * Loads the files appended to a 16-bit Windows (NE) installer executable
 * whose window class is "InstallWClass". It is driven by a text script
 * ("FILE: "Xtras\Fileio.x32", ...") stored in the same archive, and stores
 * the other files compressed with PKWARE DCL. This is the format of one
 * installer (the one on the European Safecracker CDs), not a general DCL
 * archive format.
 *
 * Members are named by the destination paths in the script, relative to
 * the install folder. Files the script doesn't name are listed by their
 * stored names.
 *
 * Takes ownership of the stream, which is deleted if loading fails.
 */
Common::Archive *createDCLInstallerArchive(Common::SeekableReadStream *stream);

} // End of namespace Common

#endif
