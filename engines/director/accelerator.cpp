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

/*
 * MacroMind Accelerator movies (file type VWC0, creator MMDA), played by
 * the playAccel command. The format was worked out from the Accelerator
 * driver (VWAD 1 in the MacroMind Player) and the sample movies on the
 * Director 3.1 disks.
 *
 * Header, 42 bytes, big-endian:
 *   0x00 word  version (0x0b, 0x0f)
 *   0x02 word  flags: 0x01 last frame loops back to the first,
 *              0x40 frames are PackBits compressed (version >= 0x0f)
 *   0x08 word  row width in words (32 = 512 pixels at 1 bit)
 *   0x0a word  rows
 *   0x0c word  pixel depth
 *   0x12 word  number of frames, the loop frame included
 *   0x16 long  size of the frame data
 *   0x1a word  timing: 0 = each frame's duration, otherwise the tempo
 *   0x1c word  bits 0-14: tempo in frames per second, negative = seconds
 *              per frame; bit 15 is a flag
 *
 * Each frame follows:
 *   long  size
 *   word  duration in ticks
 *   word, word (unused here)
 *   data  [size] (compressed: long unpacked size, then PackBits data)
 *   long  size again, so the file can be walked backwards
 *
 * The data is a list of updates to the previous screen. Each starts with an
 * address word w (negative ends the frame):
 *   w & 1 == 0  the byte at w >> 1 in the current row
 *   w & 3 == 1  next row, byte w >> 2
 *   w & 3 == 3  row (w >> 2) / 4, byte given by the next word
 * then a count word c: c >> 1 words, copied from the data if c & 1,
 * otherwise a run of the one data word that follows.
 */

#include "common/file.h"
#include "common/system.h"

#include "graphics/managed_surface.h"
#include "graphics/macgui/macwindowmanager.h"

#include "director/director.h"
#include "director/window.h"
#include "director/util.h"

namespace Director {

namespace {

struct AccelFrame {
	int duration;
	Common::Array<byte> data;
};

bool unpackBits(Common::SeekableReadStream &in, uint32 packedSize, uint32 size, Common::Array<byte> &out) {
	out.resize(size);
	uint32 pos = 0;
	int64 end = in.pos() + packedSize;

	while (pos < size && in.pos() < end) {
		byte c = in.readByte();
		if (c < 128) {
			uint32 n = MIN<uint32>(c + 1, size - pos);
			in.read(&out[pos], n);
			pos += n;
			if (n < (uint32)c + 1)
				in.skip(c + 1 - n);
		} else if (c > 128) {
			byte b = in.readByte();
			uint32 n = MIN<uint32>(257 - c, size - pos);
			memset(&out[pos], b, n);
			pos += n;
		}
	}
	in.seek(end);
	return pos == size;
}

void applyFrame(const Common::Array<byte> &data, Common::Array<byte> &screen, uint rowBytes) {
	uint i = 0;
	int32 base = 0, dst = 0;

	while (i + 2 <= data.size()) {
		int16 w = (int16)READ_BE_UINT16(&data[i]);
		i += 2;
		if (w < 0)
			return;

		if ((w & 1) == 0) {
			dst = base + (w >> 1);
		} else if ((w & 2) == 0) {
			base += rowBytes;
			dst = base + (w >> 2);
		} else {
			if (i + 2 > data.size())
				return;
			base = (w >> 4) * rowBytes;
			dst = base + (int16)READ_BE_UINT16(&data[i]);
			i += 2;
		}

		if (i + 2 > data.size())
			return;
		uint16 c = READ_BE_UINT16(&data[i]);
		i += 2;
		uint32 n = (c >> 1) * 2;

		if (c & 1) {
			n = MIN<uint32>(n, data.size() - i);
			for (uint32 k = 0; k < n; k++) {
				if (dst + k < screen.size())
					screen[dst + k] = data[i + k];
			}
			i += n;
		} else {
			if (i + 2 > data.size())
				return;
			for (uint32 k = 0; k < n; k++) {
				if (dst + k < screen.size())
					screen[dst + k] = data[i + (k & 1)];
			}
			i += 2;
		}
	}
}

} // End of anonymous namespace

void Window::playAccel(const Common::Path &path, const AccelOptions &opts) {
	Common::File file;
	if (!file.open(path)) {
		warning("Window::playAccel(): cannot open %s", path.toString().c_str());
		return;
	}

	uint16 version = file.readUint16BE();
	uint16 flags = file.readUint16BE();
	file.seek(8);
	uint rowBytes = file.readUint16BE() * 2;
	uint rows = file.readUint16BE();
	uint depth = file.readUint16BE();
	file.seek(0x12);
	uint numFrames = file.readUint16BE();
	file.seek(0x1a);
	uint16 timing = file.readUint16BE();
	int tempo = (int16)(file.readUint16BE() << 1) >> 1;

	if (depth != 1 || rowBytes == 0 || rows == 0) {
		warning("Window::playAccel(): unsupported movie %s (depth %d)", path.toString().c_str(), depth);
		return;
	}

	bool compressed = version >= 0x0f && (flags & 0x40);
	bool hasLoopFrame = flags & 0x01;

	Common::Array<AccelFrame> frames;
	file.seek(0x2a);
	while (frames.size() < numFrames && file.pos() + 14 <= file.size()) {
		AccelFrame frame;
		int32 size = file.readSint32BE();
		frame.duration = file.readSint16BE();
		file.skip(4);
		if (size < 0 || file.pos() + size > file.size())
			break;

		if (compressed) {
			uint32 unpackedSize = file.readUint32BE();
			if (!unpackBits(file, size, unpackedSize, frame.data))
				warning("Window::playAccel(): short frame %d in %s", frames.size(), path.toString().c_str());
		} else {
			frame.data.resize(size);
			file.read(frame.data.data(), size);
		}
		file.skip(4);	// size again
		frames.push_back(frame);
	}

	debugC(2, kDebugImages, "Window::playAccel(): %s version %d flags 0x%x %dx%d, %d frames, timing %d tempo %d",
		path.toString().c_str(), version, flags, rowBytes * 8, rows, frames.size(), timing, tempo);

	// Without looping, the loop frame (a copy of the first) is not shown
	uint lastFrame = frames.size();
	if (hasLoopFrame && lastFrame > 2)
		lastFrame--;
	if (lastFrame == 0)
		return;

	if (opts.tempo != 0)
		tempo = opts.tempo;
	bool useDurations = timing == 0 && opts.tempo == 0;
	if (tempo == 0)
		tempo = 60;

	Graphics::ManagedSurface *surface = _window->getSurface();
	Common::Rect area(rowBytes * 8, rows);
	Common::Point origin((surface->w - area.width()) / 2, (surface->h - area.height()) / 2);
	origin.x = MAX<int16>(origin.x, 0);
	origin.y = MAX<int16>(origin.y, 0);
	uint32 black = g_director->_wm->_colorBlack;
	uint32 white = g_director->_wm->_colorWhite;

	Common::Array<byte> screen;
	screen.resize(rowBytes * rows);

	Common::Array<Common::Event> held;
	bool stop = false;
	bool quit = false;
	uint plays = 0;
	uint frame = 0;
	uint32 nextTime = g_system->getMillis();
	uint32 tempoStart = nextTime;
	uint tempoFrames = 0;

	while (!stop) {
		applyFrame(frames[frame].data, screen, rowBytes);

		int w = MIN<int>(area.width(), surface->w - origin.x);
		int h = MIN<int>(area.height(), surface->h - origin.y);
		for (int y = 0; y < h; y++) {
			const byte *src = &screen[y * rowBytes];
			for (int x = 0; x < w; x++) {
				bool set = src[x >> 3] & (0x80 >> (x & 7));
				surface->setPixel(origin.x + x, origin.y + y, set ? black : white);
			}
		}
		_window->setDirty(true);
		g_director->draw();

		// The first pass shows 0..lastFrame-1. Later passes start with the
		// loop frame, which turns the last image back into the first, and
		// carry on from frame 1.
		if (frame + 1 == lastFrame) {
			plays++;
			if (!opts.loop && plays >= opts.repeat)
				break;
			if (lastFrame < frames.size()) {
				frame = lastFrame;
			} else {
				frame = 0;
				screen.clear();
				screen.resize(rowBytes * rows);
			}
		} else if (frame == lastFrame) {
			frame = 1;
		} else {
			frame++;
		}

		// The driver waits the duration of the frame about to be shown
		if (useDurations) {
			int ticks = MAX(frames[frame].duration & 0x7fff, 1);
			nextTime += ticks * 1000 / 60;
		} else if (tempo < 0) {
			nextTime += -tempo * 1000;
		} else {
			tempoFrames++;
			nextTime = tempoStart + tempoFrames * 1000 / tempo;
		}

		// Wait for the next frame, watching the mouse
		do {
			Common::Event event;
			while (g_director->pollModalEvent(event, quit)) {
				if (event.type == Common::EVENT_LBUTTONDOWN && (opts.click || opts.clickStop)) {
					stop = true;
					if (opts.click)
						held.push_back(event);
					continue;
				}
				if (event.type == Common::EVENT_MOUSEMOVE && opts.usePlayRect) {
					Common::Point pos = event.mouse;
					if (!opts.playRect.contains(pos))
						stop = true;
				}
				if (event.type == Common::EVENT_LBUTTONUP && opts.clickStop && stop) {
					// Swallow the release of the click that stopped the movie
					continue;
				}
				if (event.type == Common::EVENT_KEYDOWN || event.type == Common::EVENT_KEYUP ||
						event.type == Common::EVENT_LBUTTONUP || event.type == Common::EVENT_LBUTTONDOWN)
					held.push_back(event);
				else
					g_director->getMacWindowManager()->processEvent(event);
			}
			if (quit || stop)
				break;
			uint32 now = g_system->getMillis();
			if ((int32)(nextTime - now) <= 0)
				break;
			g_director->delayMillis(MIN<uint32>(nextTime - now, 10));
		} while (true);

		if (quit)
			break;
	}

	// clickStop eats the whole click: wait for the button to come up
	if (stop && opts.clickStop && !quit) {
		while (g_system->getEventManager()->getButtonState() & 1) {
			Common::Event event;
			while (g_director->pollModalEvent(event, quit)) {
				if (event.type == Common::EVENT_LBUTTONUP)
					break;
			}
			if (quit)
				break;
			g_director->delayMillis(10);
		}
	}

	for (auto &e : held)
		g_director->_injectedEvents.push_back(e);

	if (!opts.noUpdate && !quit) {
		render(true);
		g_director->draw();
	}
}

} // End of namespace Director
