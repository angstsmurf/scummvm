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

#ifndef gm_hires_h
#define gm_hires_h

#include "common/scummsys.h"

// Apple II hi-res page geometry, shared by the Apple II renderers.

namespace Glk {
namespace Comprehend {


#define GM_HIRES_PAGE_SIZE 0x2000
#define GM_MAX_SCREEN_ADDR (GM_HIRES_PAGE_SIZE - 1)
#define GM_HIRES_ROWS      192
#define GM_HIRES_COLS      40 // bytes per scanline
#define GM_PIXELS_PER_BYTE 7  // bit 7 of each byte selects the palette

// Offset into the hi-res page of the first byte of scanline y
static inline int gm_row_address(int y)
{
    return (((y / 8) & 0x07) << 7) + (((y / 8) & 0x18) * 5) + ((y & 7) << 10);
}

// Inverse of gm_row_address: the scanline that page offset `offset` lies on.
// Only meaningful for offsets inside a scanline (offset & 0x7f < 120); the
// 8 "screen hole" bytes at the end of each 128-byte block belong to no row.
static inline int gm_row_from_offset(unsigned offset)
{
    return ((offset & 0x7f) / 40) * 64 + ((offset >> 7) & 7) * 8 + ((offset >> 10) & 7);
}

// Scanline and byte column (0..39) that page offset `offset` lies on, or -1 for
// the 8 "screen hole" bytes at the end of each 128-byte block.
static inline int gm_row_of_offset(unsigned offset)
{
    return (offset & 0x7f) < 120 ? gm_row_from_offset(offset) : -1;
}

static inline int gm_col_of_offset(unsigned offset)
{
    return (offset & 0x7f) < 120 ? (int)((offset & 0x7f) % 40) : -1;
}



} // namespace Comprehend
} // namespace Glk

#endif /* gm_hires_h */
