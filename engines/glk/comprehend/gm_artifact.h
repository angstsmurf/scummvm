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

#ifndef gm_artifact_h
#define gm_artifact_h

#include "common/scummsys.h"

// Apple II hi-res to colour kernel shared by the Apple II renderers: the NTSC artifact-colour
// algorithm (from the MAME Apple II driver), the 16-entry palette and the 7-bit doubling.

namespace Glk {
namespace Comprehend {


// 16-entry Apple II hi-res palette (0x00RRGGBB), indexed by the 4-bit artifact
// colour produced by gm_render_line_colors().
extern const uint32 gm_apple2_palette[16];

// Double a 7-bit hi-res byte into its 14-bit doubled form (each set low bit
// becomes two output bits). Returns 0 for inputs > 127.
uint16 gm_double7bits(uint8 value);

// Compute the 40 doubled 14-bit "words" for one hi-res scanline
// (vram_row[0..39]), applying the high-bit (palette/phase) carry between
// adjacent bytes. words[] must hold 40 entries.
void gm_compute_row_words(const uint8 *vram_row, uint16 *words);

// Expand one scanline's 40 words into 560 4-bit NTSC-artifact colour indices.
// colors560[] must hold 560 entries.
void gm_render_line_colors(const uint16 *words, uint8 *colors560);

// Double hi-res (DHGR) supplies 560 real pixels per row, so there is no 7->14 doubling: each of
// the 40 columns carries 14 bits, the aux byte's low 7 pixels (leftmost) then the main byte's low
// 7. Build those words from a scanline's aux_row[0..39], main_row[0..39]. words[] must hold 40.
void gm_compute_dhgr_row_words(const uint8 *aux_row, const uint8 *main_row,
    uint16 *words);

// Expand one DHGR scanline's 40 words into 560 4-bit NTSC-artifact colour
// indices. Identical pixel-run colour kernel as gm_render_line_colors(), but at
// the 80-column chroma phase (MAME's is_80_column rotation offset). colors560[]
// must hold 560 entries.
void gm_render_dhgr_line_colors(const uint16 *words, uint8 *colors560);



} // namespace Comprehend
} // namespace Glk

#endif /* gm_artifact_h */
