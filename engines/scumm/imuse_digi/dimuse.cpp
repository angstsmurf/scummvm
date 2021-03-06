/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/system.h"
#include "common/timer.h"

#include "scumm/actor.h"
#include "scumm/scumm_v7.h"
#include "scumm/sound.h"
#include "scumm/imuse_digi/dimuse.h"
#include "scumm/imuse_digi/dimuse_bndmgr.h"
#include "scumm/imuse_digi/dimuse_codecs.h"
#include "scumm/imuse_digi/dimuse_track.h"

#include "audio/audiostream.h"
#include "audio/mixer.h"
#include "audio/decoders/raw.h"

namespace Scumm {

void IMuseDigital::timer_handler(void *refCon) {
	IMuseDigital *imuseDigital = (IMuseDigital *)refCon;
	imuseDigital->callback();
}

IMuseDigital::IMuseDigital(ScummEngine_v7 *scumm, Audio::Mixer *mixer, int fps)
	: _vm(scumm), _mixer(mixer) {
	assert(_vm);
	assert(mixer);

	_pause = false;
	_sound = new ImuseDigiSndMgr(_vm);
	assert(_sound);
	_callbackFps = fps;
	resetState();
	for (int l = 0; l < MAX_DIGITAL_TRACKS + MAX_DIGITAL_FADETRACKS; l++) {
		_track[l] = new Track;
		assert(_track[l]);
		_track[l]->reset();
		_track[l]->trackId = l;
	}
	_vm->getTimerManager()->installTimerProc(timer_handler, 1000000 / _callbackFps, this, "IMuseDigital");

	_audioNames = NULL;
	_numAudioNames = 0;
}

IMuseDigital::~IMuseDigital() {
	_vm->getTimerManager()->removeTimerProc(timer_handler);
	stopAllSounds();
	for (int l = 0; l < MAX_DIGITAL_TRACKS + MAX_DIGITAL_FADETRACKS; l++) {
		delete _track[l];
	}
	delete _sound;
	free(_audioNames);
}

static int32 makeMixerFlags(Track *track) {
	const int32 flags = track->mixerFlags;
	int32 mixerFlags = 0;
	if (flags & kFlagUnsigned)
		mixerFlags |= Audio::FLAG_UNSIGNED;
	if (flags & kFlag16Bits)
		mixerFlags |= Audio::FLAG_16BITS;

#ifdef SCUMM_LITTLE_ENDIAN
	if (track->sndDataExtComp)
		mixerFlags |= Audio::FLAG_LITTLE_ENDIAN;
#endif
	if (flags & kFlagStereo)
		mixerFlags |= Audio::FLAG_STEREO;
	return mixerFlags;
}

void IMuseDigital::resetState() {
	_curMusicState = 0;
	_curMusicSeq = 0;
	_curMusicCue = 0;
	memset(_attributes, 0, sizeof(_attributes));
	_nextSeqToPlay = 0;
	_stopingSequence = 0;
	_radioChatterSFX = 0;
	_triggerUsed = false;
}

static void syncWithSerializer(Common::Serializer &s, Track &t) {
	s.syncAsSByte(t.pan, VER(31));
	s.syncAsSint32LE(t.vol, VER(31));
	s.syncAsSint32LE(t.volFadeDest, VER(31));
	s.syncAsSint32LE(t.volFadeStep, VER(31));
	s.syncAsSint32LE(t.volFadeDelay, VER(31));
	s.syncAsByte(t.volFadeUsed, VER(31));
	s.syncAsSint32LE(t.soundId, VER(31));
	s.syncArray(t.soundName, 15, Common::Serializer::SByte, VER(31));
	s.syncAsByte(t.used, VER(31));
	s.syncAsByte(t.toBeRemoved, VER(31));
	s.syncAsByte(t.souStreamUsed, VER(31));
	s.skip(1, VER(31), VER(76)); // mixerStreamRunning
	s.syncAsSint32LE(t.soundPriority, VER(31));
	s.syncAsSint32LE(t.regionOffset, VER(31));
	s.skip(4, VER(31), VER(31)); // trackOffset
	s.syncAsSint32LE(t.dataOffset, VER(31));
	s.syncAsSint32LE(t.curRegion, VER(31));
	s.syncAsSint32LE(t.curHookId, VER(31));
	s.syncAsSint32LE(t.volGroupId, VER(31));
	s.syncAsSint32LE(t.soundType, VER(31));
	s.syncAsSint32LE(t.feedSize, VER(31));
	s.syncAsSint32LE(t.dataMod12Bit, VER(31));
	s.syncAsSint32LE(t.mixerFlags, VER(31));
	s.skip(4, VER(31), VER(42)); // mixerVol
	s.skip(4, VER(31), VER(42)); // mixerPan
	s.syncAsByte(t.sndDataExtComp, VER(45));
}

void IMuseDigital::saveLoadEarly(Common::Serializer &s) {
	Common::StackLock lock(_mutex, "IMuseDigital::saveLoadEarly()");

	s.skip(4, VER(31), VER(42)); // _volVoice
	s.skip(4, VER(31), VER(42)); // _volSfx
	s.skip(4, VER(31), VER(42)); // _volMusic
	s.syncAsSint32LE(_curMusicState, VER(31));
	s.syncAsSint32LE(_curMusicSeq, VER(31));
	s.syncAsSint32LE(_curMusicCue, VER(31));
	s.syncAsSint32LE(_nextSeqToPlay, VER(31));
	s.syncAsByte(_radioChatterSFX, VER(76));
	s.syncArray(_attributes, 188, Common::Serializer::Sint32LE, VER(31));

	for (int l = 0; l < MAX_DIGITAL_TRACKS + MAX_DIGITAL_FADETRACKS; l++) {
		Track *track = _track[l];
		if (s.isLoading()) {
			track->reset();
		}
		syncWithSerializer(s, *track);
		if (s.isLoading()) {
			_track[l]->trackId = l;
			if (!track->used)
				continue;
			if ((track->toBeRemoved) || (track->souStreamUsed) || (track->curRegion == -1)) {
				track->used = false;
				continue;
			}

			// TODO: The code below has a lot in common with that in IMuseDigital::startSound.
			// Try to refactor them to reduce the code duplication.

			track->soundDesc = _sound->openSound(track->soundId, track->soundName, track->soundType, track->volGroupId, -1);
			if (!track->soundDesc)
				track->soundDesc = _sound->openSound(track->soundId, track->soundName, track->soundType, track->volGroupId, 1);
			if (!track->soundDesc)
				track->soundDesc = _sound->openSound(track->soundId, track->soundName, track->soundType, track->volGroupId, 2);

			if (!track->soundDesc) {
				warning("IMuseDigital::saveOrLoad: Can't open sound so will not be resumed");
				track->used = false;
				continue;
			}

			track->sndDataExtComp = _sound->isSndDataExtComp(track->soundDesc);
			track->dataOffset = _sound->getRegionOffset(track->soundDesc, track->curRegion);
			int bits = _sound->getBits(track->soundDesc);
			int channels = _sound->getChannels(track->soundDesc);
			int freq = _sound->getFreq(track->soundDesc);
			track->feedSize = freq * channels;
			track->mixerFlags = 0;
			if (channels == 2)
				track->mixerFlags = kFlagStereo;

			if ((bits == 12) || (bits == 16)) {
				track->mixerFlags |= kFlag16Bits;
				track->feedSize *= 2;
			} else if (bits == 8) {
				track->mixerFlags |= kFlagUnsigned;
			} else
				error("IMuseDigital::saveOrLoad(): Can't handle %d bit samples", bits);

			track->stream = Audio::makeQueuingAudioStream(freq, (track->mixerFlags & kFlagStereo) != 0);

			_mixer->playStream(track->getType(), &track->mixChanHandle, track->stream, -1, track->getVol(), track->getPan());
			_mixer->pauseHandle(track->mixChanHandle, true);
		}
	}
}

void IMuseDigital::callback() {
	Common::StackLock lock(_mutex, "IMuseDigital::callback()");

	for (int l = 0; l < MAX_DIGITAL_TRACKS + MAX_DIGITAL_FADETRACKS; l++) {
		Track *track = _track[l];
		if (track->used) {
			// Ignore tracks which are about to finish. Also, if it did finish in the meantime,
			// mark it as unused.
			if (!track->stream) {
				if (!_mixer->isSoundHandleActive(track->mixChanHandle))
					track->reset();
				continue;
			}

			if (_pause)
				return;

			if (track->volFadeUsed) {
				if (_vm->_game.id == GID_CMI) {
					if (track->vol == track->volFadeDest) // Sanity check
						track->volFadeUsed = false;

					if (track->volFadeStep < 0) { // Fade out
						if (track->vol > track->volFadeDest) {
							int tempVolume = transformVolumeEqualPowToLinear(track->vol, 1); // Equal power to linear...
							tempVolume += track->volFadeStep; // Remove step...
							track->vol = transformVolumeLinearToEqualPow(tempVolume, 1); // Linear to equal power...

							if (track->vol <= track->volFadeDest) {
								track->vol = track->volFadeDest;
								track->volFadeUsed = false;
								flushTrack(track);
								continue;
							}
							if (track->vol == 0) {
								// Fade out complete -> remove this track
								flushTrack(track);
								continue;
							}
						}
					} else if (track->volFadeStep > 0) { // Fade in
						if (track->vol < track->volFadeDest) {
							int tempVolume = transformVolumeEqualPowToLinear(track->vol, 1); // Equal power to linear...
							tempVolume += track->volFadeStep; // Add step...
							track->vol = transformVolumeLinearToEqualPow(tempVolume, 1); // Linear to equal power...
							if (track->vol >= track->volFadeDest) {
								track->vol = track->volFadeDest;
								track->volFadeUsed = false;
							}
						}
					}
				} else {
					if (track->volFadeStep < 0) {
						if (track->vol > track->volFadeDest) {
							track->vol += track->volFadeStep; 
							if (track->vol < track->volFadeDest) {
								track->vol = track->volFadeDest;
								track->volFadeUsed = false;
							}
							if (track->vol == 0) {
								// Fade out complete -> remove this track
								flushTrack(track);
								continue;
							}
						}
					} else if (track->volFadeStep > 0) {
						if (track->vol < track->volFadeDest) {
							track->vol += track->volFadeStep;
							if (track->vol > track->volFadeDest) {
								track->vol = track->volFadeDest;
								track->volFadeUsed = false;
							}
						}
					}
				}
 				debug(5, "Fade: sound(%d), Vol(%d) in track(%d)", track->soundId, track->vol / 1000, track->trackId);
			}

			if (!track->souStreamUsed) {
				assert(track->stream);
				byte *tmpSndBufferPtr = NULL;
				int32 curFeedSize = 0;

				if (track->curRegion == -1) {
					switchToNextRegion(track);
					if (!track->stream)	// Seems we reached the end of the stream
						continue;
				}

				int bits = _sound->getBits(track->soundDesc);
				int channels = _sound->getChannels(track->soundDesc);

				int32 feedSize = track->feedSize / _callbackFps;

				if (track->stream->endOfData()) {
					feedSize *= 2;
				}

				if ((bits == 12) || (bits == 16)) {
					if (channels == 1)
						feedSize &= ~1;
					if (channels == 2)
						feedSize &= ~3;
				} else if (bits == 8) {
					if (channels == 2)
						feedSize &= ~1;
				} else {
					warning("IMuseDigita::callback: Unexpected sample width, %d bits", bits);
					continue;
				}

				if (feedSize == 0)
					continue;

				do {
					if (bits == 12) {
						byte *tmpPtr = NULL;

						feedSize += track->dataMod12Bit;
						int tmpFeedSize12Bits = (feedSize * 3) / 4;
						int tmpLength12Bits = (tmpFeedSize12Bits / 3) * 4;
						track->dataMod12Bit = feedSize - tmpLength12Bits;

						int32 tmpOffset = (track->regionOffset * 3) / 4;
						int tmpFeedSize = _sound->getDataFromRegion(track->soundDesc, track->curRegion, &tmpPtr, tmpOffset, tmpFeedSize12Bits);
						curFeedSize = BundleCodecs::decode12BitsSample(tmpPtr, &tmpSndBufferPtr, tmpFeedSize);

						free(tmpPtr);
					} else if (bits == 16) {
						curFeedSize = _sound->getDataFromRegion(track->soundDesc, track->curRegion, &tmpSndBufferPtr, track->regionOffset, feedSize);
						if (channels == 1) {
							curFeedSize &= ~1;
						}
						if (channels == 2) {
							curFeedSize &= ~3;
						}
					} else if (bits == 8) {
						curFeedSize = _sound->getDataFromRegion(track->soundDesc, track->curRegion, &tmpSndBufferPtr, track->regionOffset, feedSize);
						if (_radioChatterSFX && track->soundId == 10000) {
							if (curFeedSize > feedSize)
								curFeedSize = feedSize;
							byte *buf = (byte *)malloc(curFeedSize);
							int index = 0;
							int count = curFeedSize - 4;
							byte *ptr_1 = tmpSndBufferPtr;
							byte *ptr_2 = tmpSndBufferPtr + 4;
							int value = ptr_1[0] - 0x80;
							value += ptr_1[1] - 0x80;
							value += ptr_1[2] - 0x80;
							value += ptr_1[3] - 0x80;
							do {
								int t = *ptr_1++;
								int v = t - (value / 4);
								value = *ptr_2++ - 0x80 + (value - t + 0x80);
								buf[index++] = v * 2 + 0x80;
							} while (--count);
							buf[curFeedSize - 1] = 0x80;
							buf[curFeedSize - 2] = 0x80;
							buf[curFeedSize - 3] = 0x80;
							buf[curFeedSize - 4] = 0x80;
							free(tmpSndBufferPtr);
							tmpSndBufferPtr = buf;
						}
						if (channels == 2) {
							curFeedSize &= ~1;
						}
					}

					if (curFeedSize > feedSize)
						curFeedSize = feedSize;

					if (_mixer->isReady()) {
						track->stream->queueBuffer(tmpSndBufferPtr, curFeedSize, DisposeAfterUse::YES, makeMixerFlags(track));
						track->regionOffset += curFeedSize;
					} else
						free(tmpSndBufferPtr);

					if (_sound->isEndOfRegion(track->soundDesc, track->curRegion)) {
						switchToNextRegion(track);
						if (!track->stream)	// Seems we reached the end of the stream
							break;
					}
					feedSize -= curFeedSize;
					assert(feedSize >= 0);
				} while (feedSize != 0);
			}
			if (_mixer->isReady()) {
				_mixer->setChannelVolume(track->mixChanHandle, track->getVol());
				_mixer->setChannelBalance(track->mixChanHandle, track->getPan());
			}
		}
	}
}

void IMuseDigital::switchToNextRegion(Track *track) {
	assert(track);

	if (track->trackId >= MAX_DIGITAL_TRACKS) {
		flushTrack(track);
		debug(5, "SwToNeReg(trackId:%d) - fadetrack can't go next region, exiting SwToNeReg", track->trackId);
		return;
	}

	int num_regions = _sound->getNumRegions(track->soundDesc);

	if (++track->curRegion == num_regions) {
		flushTrack(track);
		debug(5, "SwToNeReg(trackId:%d) - end of region, exiting SwToNeReg", track->trackId);
		return;
	}

	ImuseDigiSndMgr::SoundDesc *soundDesc = track->soundDesc;
	if (_triggerUsed && track->soundDesc->numMarkers) {
		if (_sound->checkForTriggerByRegionAndMarker(soundDesc, track->curRegion, _triggerParams.marker)) {
			debug(5, "SwToNeReg(trackId:%d) - trigger %s reached", track->trackId, _triggerParams.marker);
			debug(5, "SwToNeReg(trackId:%d) - exit current region %d", track->trackId, track->curRegion);
			debug(5, "SwToNeReg(trackId:%d) - call cloneToFadeOutTrack(delay:%d)", track->trackId, _triggerParams.fadeOutDelay);
			Track *fadeTrack = cloneToFadeOutTrack(track, _triggerParams.fadeOutDelay);
			if (fadeTrack) {
				fadeTrack->dataOffset = _sound->getRegionOffset(fadeTrack->soundDesc, fadeTrack->curRegion);
				fadeTrack->regionOffset = 0;
				debug(5, "SwToNeReg(trackId:%d)-sound(%d) select region %d, curHookId: %d", fadeTrack->trackId, fadeTrack->soundId, fadeTrack->curRegion, fadeTrack->curHookId);
				fadeTrack->curHookId = 0;
			}
			flushTrack(track);
			startMusic(_triggerParams.filename, _triggerParams.soundId, _triggerParams.hookId, _triggerParams.volume);
			_triggerUsed = false;
			return;
		}
	}

	int jumpId = _sound->getJumpIdByRegionAndHookId(soundDesc, track->curRegion, track->curHookId);
	if (jumpId != -1) {
		int region = _sound->getRegionIdByJumpId(soundDesc, jumpId);
		assert(region != -1);
		int sampleHookId = _sound->getJumpHookId(soundDesc, jumpId);
		assert(sampleHookId != -1);
		debug(5, "SwToNeReg(trackId:%d) - JUMP found - sound:%d, track hookId:%d, data hookId:%d", track->trackId, track->soundId, track->curHookId, sampleHookId);
		if (track->curHookId == sampleHookId) {
			int fadeDelay = (60 * _sound->getJumpFade(soundDesc, jumpId)) / 1000;
			debug(5, "SwToNeReg(trackId:%d) - sound(%d) match hookId", track->trackId, track->soundId);
			if (fadeDelay) {
				// COMI specific: jump to new region without true crossfades; this is not true to the original interpreter,
				// but crossfading between regions in the correct way requires an implementation which is more complex.
				// Leaving as is, for now.
				if (_vm->_game.id != GID_CMI) {
					debug(5, "SwToNeReg(trackId:%d) - call cloneToFadeOutTrack(delay:%d)", track->trackId, fadeDelay);
					Track *fadeTrack = cloneToFadeOutTrack(track, fadeDelay);
					if (fadeTrack) {
						fadeTrack->dataOffset = _sound->getRegionOffset(fadeTrack->soundDesc, fadeTrack->curRegion);
						fadeTrack->regionOffset = 0;
						debug(5, "SwToNeReg(trackId:%d) - sound(%d) faded track, select region %d, curHookId: %d", fadeTrack->trackId, fadeTrack->soundId, fadeTrack->curRegion, fadeTrack->curHookId);
						fadeTrack->curHookId = 0;
					}
				}	
			}
			track->curRegion = region;
			debug(5, "SwToNeReg(trackId:%d) - sound(%d) jump to region %d, curHookId: %d", track->trackId, track->soundId, track->curRegion, track->curHookId);
			track->curHookId = 0;
			
		} else {
			debug(5, "SwToNeReg(trackId:%d) - Normal switch region, sound(%d), hookId(%d)", track->trackId, track->soundId, track->curHookId);
		}
	} else {
		debug(5, "SwToNeReg(trackId:%d) - Normal switch region, sound(%d), hookId(%d)", track->trackId, track->soundId, track->curHookId);
	}
	debug(5, "SwToNeReg(trackId:%d) - sound(%d), select region %d", track->trackId, track->soundId, track->curRegion);
	track->dataOffset = _sound->getRegionOffset(soundDesc, track->curRegion);
	track->regionOffset = 0;
	debug(5, "SwToNeReg(trackId:%d) - end of func", track->trackId);
}

} // End of namespace Scumm
