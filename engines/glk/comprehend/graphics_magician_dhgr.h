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

#ifndef GLK_COMPREHEND_GRAPHICS_MAGICIAN_DHGR_H
#define GLK_COMPREHEND_GRAPHICS_MAGICIAN_DHGR_H

#include "common/scummsys.h"

// Apple II double hi-res ("<D>" mode) renderer for the V2 games (Talisman, OO-Topos, Transylvania
// v2, The Coveted Mirror). Companion to graphics_magician.h; see graphics_magician_dhgr.cpp.

namespace Glk {
namespace Comprehend {

// Install the double-hi-res drawing tables (colour subindex, sub-table, pattern rows, brush
// bitmaps) from the boot disk's "T5" file, the headerless <D> interpreter, which loads at $4000;
// the tables sit at fixed offsets inside it. Returns false if the buffer is too short. Per-image
// colour overrides arrive at draw time as op7 commands.
bool gmDhgrInstallDrawingTables(const uint8 *t5, size_t size);
bool gmDhgrHaveDrawingTables();

// Clear the persistent double-hi-res pages (white = room background, black =
// transparent base for overlays). Call before a fresh room; item pictures
// compose onto the existing pages and must not reset them.
void gmDhgrResetScreen(bool white);

// Run one image's Graphics Magician vector stream onto the persistent pages.
void gmDhgrDrawImage(const uint8 *data, size_t size);

// Convert the pages to RGBA (0xRRGGBBAA) into out[w*h], rendering rows 0..h-1.
// The native 560 DHGR columns are sampled to the surface width w (1:1 at w>=560).
void gmDhgrBlitToSurface(uint32 *out, int w, int h);

// Slow draw, as in graphics_magician.h: when enabled, gmDhgrDrawImage records the order its bytes
// are plotted (and op13 DELAY markers), and the host reveals them on a timer while
// gmDhgrSlowDrawActive() is true.

// Enable/disable op recording for the next image(s).
void gmDhgrSetSlowDraw(bool on);
// True while recorded bytes (or a DELAY pause) are still waiting to be revealed.
bool gmDhgrSlowDrawActive();
// Reveal up to `budget` more bytes (rounded up across adjacent runs); returns
// true while more remain.
bool gmDhgrAdvanceSlowDraw(int budget);
// Reveal everything left at once (resize / cancel).
void gmDhgrFinishSlowDraw();
// Report the inclusive row band [*y0,*y1] changed since the last call (so the
// host repaints just that band), and reset it. Returns false if unchanged.
bool gmDhgrSlowConsumeDirty(int *y0, int *y1);
// Blit the partially-revealed pages, same format as gmDhgrBlitToSurface().
void gmDhgrBlitSlowToSurface(uint32 *out, int w, int h);

// The Coveted Mirror's right-hand panel: counterparts of gmCaptureCMPanel / gmOverlayCMPanel /
// gmDrawCMHourglass on the aux+main pages. The panel is captured by byte column [col0,col1], with
// the same column indices as standard hi-res.
void gmDhgrCaptureCMPanel(int col0, int col1);
void gmDhgrOverlayCMPanel();
bool gmDhgrCMPanelValid();
void gmDhgrDrawCMHourglass(int sand);

// Double-hi-res counterpart of gmBeginCMPanelRebuild()/gmEndCMPanelRebuild():
// rebuild the panel snapshot (logo + frame + collected mirror pieces) off to the
// side without disturbing the live aux/main pages or recording any of it.
void gmDhgrBeginCMPanelRebuild();
void gmDhgrEndCMPanelRebuild();

// Double-hi-res counterpart of the standard per-turn grain fall
// (gmCMHourglassFallBegin/Step/Abort in graphics_magician.h): animates the freed
// grain down the neck on the aux+main pages. Arming is shared (gmCMHourglassArm /
// gmCMHourglassConsumeFallArmed), so the host drives whichever renderer is active.
void gmDhgrCMHourglassFallBegin();
bool gmDhgrCMHourglassFallActive();
bool gmDhgrCMHourglassFallStep(int *y0, int *y1);
void gmDhgrCMHourglassFallAbort();

// Diagnostic access to the raw main/aux pages.
const uint8 *gmDhgrMainPtr();
const uint8 *gmDhgrAuxPtr();

} // namespace Comprehend
} // namespace Glk

#endif
