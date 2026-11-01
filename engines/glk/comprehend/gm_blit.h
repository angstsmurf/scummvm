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

#ifndef GLK_COMPREHEND_GM_BLIT_H
#define GLK_COMPREHEND_GM_BLIT_H
#include "glk/comprehend/gm_artifact.h"

// The last step of the Apple II renderers' RGBA conversion: sampling a scanline's 560 artifact
// colours down to the surface width with the Apple palette.

namespace Glk {
namespace Comprehend {

// Write one output scanline of `w` pixels (0xRRGGBBAA) from the 560 artifact
// colour indices of a row.  With `oneToOneWhenWide`, a surface w >= 560 reads
// column x directly (clamped to 559); otherwise column x*560/w.
inline void gmSampleRow560(const uint8 colors560[560], uint32 *out, int w,
						   bool oneToOneWhenWide) {
	const bool direct = oneToOneWhenWide && w >= 560;
	for (int x = 0; x < w; x++) {
		int sx = direct ? x : (x * 560) / w;
		if (sx > 559) sx = 559;
		uint32 rgb = gm_apple2_palette[colors560[sx] & 0x0f];
		out[x] = (rgb << 8) | 0xff;
	}
}

} // namespace Comprehend
} // namespace Glk

#endif
