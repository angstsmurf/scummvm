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

#include "glk/comprehend/gm_backend.h"
#include "glk/comprehend/graphics_magician.h"
#include "glk/comprehend/graphics_magician_cga.h"
#include "glk/comprehend/graphics_magician_dhgr.h"
#include "glk/comprehend/graphics_magician_pcjr.h"

// The per-renderer entry point tables. See gm_backend.h.

namespace Glk {
namespace Comprehend {

static const GmCMPanelOps kHiresCM = {
	gmCMPanelValid, gmBeginCMPanelRebuild, gmEndCMPanelRebuild,
	gmOverlayCMPanel, gmDrawCMHourglass,
	gmCMHourglassFallBegin, gmCMHourglassFallStep, gmCMHourglassFallAbort
};

static const GmCMPanelOps kDhgrCM = {
	gmDhgrCMPanelValid, gmDhgrBeginCMPanelRebuild, gmDhgrEndCMPanelRebuild,
	gmDhgrOverlayCMPanel, gmDhgrDrawCMHourglass,
	gmDhgrCMHourglassFallBegin, gmDhgrCMHourglassFallStep, gmDhgrCMHourglassFallAbort
};

static const GmBackend kHires = {
	GFX_APPLE_HIRES,
	gmResetScreen, gmDrawImage, gmBlitToSurface,
	gmSetSlowDraw, gmSlowDrawActive, gmAdvanceSlowDraw, gmFinishSlowDraw,
	gmSlowConsumeDirty, gmBlitSlowToSurface, &kHiresCM
};

static const GmBackend kDhgr = {
	GFX_APPLE_DHGR,
	gmDhgrResetScreen, gmDhgrDrawImage, gmDhgrBlitToSurface,
	gmDhgrSetSlowDraw, gmDhgrSlowDrawActive, gmDhgrAdvanceSlowDraw, gmDhgrFinishSlowDraw,
	gmDhgrSlowConsumeDirty, gmDhgrBlitSlowToSurface, &kDhgrCM
};

static const GmBackend kCga = {
	GFX_DOS_CGA,
	gmcgaResetScreen, gmcgaDrawImage, gmcgaBlitToSurface,
	gmcgaSetSlowDraw, gmcgaSlowDrawActive, gmcgaAdvanceSlowDraw, gmcgaFinishSlowDraw,
	gmcgaSlowConsumeDirty, gmcgaBlitSlowToSurface, nullptr
};

static const GmBackend kPcjr = {
	GFX_DOS_PCJR,
	gmpcjrResetScreen, gmpcjrDrawImage, gmpcjrBlitToSurface,
	gmpcjrSetSlowDraw, gmpcjrSlowDrawActive, gmpcjrAdvanceSlowDraw, gmpcjrFinishSlowDraw,
	gmpcjrSlowConsumeDirty, gmpcjrBlitSlowToSurface, nullptr
};

const GmBackend *gmBackend(GfxBackend id) {
	switch (id) {
	case GFX_APPLE_HIRES: return &kHires;
	case GFX_APPLE_DHGR:  return &kDhgr;
	case GFX_DOS_CGA:     return &kCga;
	case GFX_DOS_PCJR:    return &kPcjr;
	}
	return &kHires;
}

void gmSetSlowDrawAll(bool on) {
	kHires.setSlowDraw(on);
	kCga.setSlowDraw(on);
	kDhgr.setSlowDraw(on);
	kPcjr.setSlowDraw(on);
}

const GmBackend *gmSlowDrawBackend() {
	if (kDhgr.slowDrawActive()) return &kDhgr;
	if (kCga.slowDrawActive())  return &kCga;
	if (kPcjr.slowDrawActive()) return &kPcjr;
	return &kHires;
}

void gmAppleResetState();
void gmDhgrResetState();
void gmcgaResetState();
void gmpcjrResetState();
void gmAppleFreeState();
void gmDhgrFreeState();
void gmcgaFreeState();
void gmpcjrFreeState();

void gmResetState() {
	gmAppleResetState();
	gmDhgrResetState();
	gmcgaResetState();
	gmpcjrResetState();
}

void gmFreeState() {
	gmAppleFreeState();
	gmDhgrFreeState();
	gmcgaFreeState();
	gmpcjrFreeState();
}

} // namespace Comprehend
} // namespace Glk
