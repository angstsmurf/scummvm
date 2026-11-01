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

#include "glk/comprehend/game_traits.h"
#include "glk/comprehend/game_cc.h"
#include "glk/comprehend/game_cm.h"
#include "glk/comprehend/game_oo.h"
#include "glk/comprehend/game_tm.h"
#include "glk/comprehend/game_tr1.h"
#include "glk/comprehend/game_tr2.h"

// The per-game table. See game_traits.h.

namespace Glk {
namespace Comprehend {

template <typename T>
static ComprehendGame *make() { return new T(); }

static const GameTraits kGames[] = {
	{
		"transylvania", make<TransylvaniaGame1>,
		/* stringsOnlyInFiles */ false,
		/* padsStringBanks */ false, /* appleStringLead */ 2,
		/* appleStringBanksHeaderless */ false,
		/* gmLegacyFormat */ true, /* appleTitleOffset */ 0xFE,
		/* appleTitleOnBlack */ false, /* hasHourglassPanel */ false
	},
	{
		"transylvaniav2", make<TransylvaniaGame2>,
		false, false, 0, false,
		false, 0, false, false
	},
	{
		"crimsoncrown", make<CrimsonCrownGame>,
		false, false, 0, false,
		true, 0x100, false, false
	},
	{
		"ootopos", make<OOToposGame>,
		false, true, 1, false,
		false, 0x100, true, false
	},
	{
		// Talisman's T0 is a plain Graphics Magician image from offset 0.
		"talisman", make<TalismanGame>,
		true, false, 0, false,
		false, 0, false, false
	},
	{
		"covetedmirror", make<CovetedMirrorGame>,
		false, false, 0, true,
		false, 0x100, false, true
	}
};

const GameTraits *findGameTraits(const char *id) {
	for (const GameTraits &t : kGames)
		if (!strcmp(t.id, id))
			return &t;
	return nullptr;
}

} // namespace Comprehend
} // namespace Glk
