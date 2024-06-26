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
 *
 * This file is dual-licensed.
 * In addition to the GPLv3 license mentioned above, this code is also
 * licensed under LGPL 2.1. See LICENSES/COPYING.LGPL file for the
 * full text of the license.
 *
 */

#ifndef GOB_SOUND_INFOGRAMES_H
#define GOB_SOUND_INFOGRAMES_H

#include "audio/mixer.h"
#include "audio/mods/infogrames.h"

namespace Gob {

class Infogrames {
public:
	Infogrames(Audio::Mixer &mixer);
	~Infogrames();

	bool loadInstruments(const char *fileName);
	bool loadSong(const char *fileName);

	void play();
	void stop();

private:
	Audio::Mixer *_mixer;

	Audio::Infogrames::Instruments *_instruments;
	Audio::Infogrames *_song;
	Audio::SoundHandle _handle;

	void clearInstruments();
	void clearSong();

	bool loadInst(const char *fileName);
};

} // End of namespace Gob

#endif // GOB_SOUND_INFOGRAMES_H
