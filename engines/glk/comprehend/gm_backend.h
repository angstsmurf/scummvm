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

#ifndef GLK_COMPREHEND_GM_BACKEND_H
#define GLK_COMPREHEND_GM_BACKEND_H

#include "common/scummsys.h"

// One table of entry points per Graphics Magician renderer (Apple II hi-res and double hi-res,
// DOS CGA and PCjr). The host code picks a GmBackend once and calls through it.

namespace Glk {
namespace Comprehend {

// Which renderer draws the pictures.
enum GfxBackend {
	GFX_APPLE_HIRES,
	GFX_APPLE_DHGR,
	GFX_DOS_CGA,
	GFX_DOS_PCJR
};

// The Coveted Mirror's persistent logo/hourglass panel. Apple II only.
struct GmCMPanelOps {
	bool (*panelValid)();
	void (*beginPanelRebuild)();
	void (*endPanelRebuild)();
	void (*overlayPanel)();
	void (*drawHourglass)(int sand);
	void (*fallBegin)();
	bool (*fallStep)(int *y0, int *y1);
	void (*fallAbort)();
};

struct GmBackend {
	GfxBackend id;
	void (*resetScreen)(bool white);
	void (*drawImage)(const uint8 *data, size_t size);
	void (*blitToSurface)(uint32 *out, int w, int h);
	void (*setSlowDraw)(bool on);
	bool (*slowDrawActive)();
	bool (*advanceSlowDraw)(int budget);
	void (*finishSlowDraw)();
	bool (*slowConsumeDirty)(int *y0, int *y1);
	void (*blitSlowToSurface)(uint32 *out, int w, int h);
	const GmCMPanelOps *cm; // nullptr on the DOS renderers

	// Blit the page the player should currently see: the partially revealed
	// one while a slow-draw reveal is queued, the finished one otherwise.
	void blitVisible(uint32 *out, int w, int h) const {
		if (slowDrawActive())
			blitSlowToSurface(out, w, h);
		else
			blitToSurface(out, w, h);
	}
};

// The table for a renderer.
const GmBackend *gmBackend(GfxBackend id);

// Turn draw-order recording on or off in every renderer. Only the one that
// actually draws records anything.
void gmSetSlowDrawAll(bool on);

// The renderer with a reveal queued, or the Apple II hi-res one when none has.
const GmBackend *gmSlowDrawBackend();

// Forget what the renderers kept from a previous game in this ScummVM session.
void gmResetState();

// Free what the renderers allocated. Called when the engine shuts down.
void gmFreeState();

} // namespace Comprehend
} // namespace Glk

#endif
