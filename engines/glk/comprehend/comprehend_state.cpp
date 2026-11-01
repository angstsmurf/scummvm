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

#include "glk/comprehend/comprehend.h"
#include "glk/comprehend/graphics_magician.h"
#include "glk/comprehend/pics.h"

// Comprehend engine glue: saved games and undo.

namespace Glk {
namespace Comprehend {

Common::Error Comprehend::readSaveData(Common::SeekableReadStream *rs) {
	Common::Array<byte> payload;
	payload.resize((uint)rs->size());
	if (!payload.empty() && rs->read(payload.data(), payload.size()) != payload.size())
		return Common::Error(Common::kReadingFailed);

	Common::Array<byte> snapshot;
	if (!serializeGameState(snapshot))
		return Common::Error(Common::kReadingFailed);

	// A save can come from another part of the game, whose database has other
	// room and item counts, so load that part before checking the size.
	if (!_game->prepareRestore(payload))
		return Common::Error(Common::kReadingFailed);

	// The state layout is fixed-size for a given part, so the live state's
	// serialized length is exactly how long a genuine save's payload must be.
	// A wrong-sized file is rejected here, before it can reach synchronizeSave()
	// (which asserts on a mismatched room/item count).
	Common::Array<byte> expected;
	if (!serializeGameState(expected) || payload.size() != expected.size()) {
		deserializeGameState(snapshot);
		return Common::Error(Common::kReadingFailed);
	}

	deserializeGameState(payload);

	// A right-sized but inconsistent payload must not strand the player in a
	// non-existent room: roll back if so.
	if (!_game->saveStateLooksValid()) {
		deserializeGameState(snapshot);
		return Common::Error(Common::kReadingFailed);
	}

	clearUndo();  // can't undo across a restore

	// Repaint the room description, item list and picture for the restored state.
	_game->_updateFlags = UPDATE_ALL;

	// A restore from the GMM arrives while the game is waiting at the prompt.
	if (isInputLineActive()) {
		print("\n");
		_game->update();
		print("> ");
	}

	return Common::kNoError;
}

Common::Error Comprehend::writeGameData(Common::WriteStream *ws) {
	Common::Serializer s(nullptr, ws);
	_game->synchronizeSave(s);
	return Common::kNoError;
}

bool Comprehend::serializeGameState(Common::Array<byte> &out) {
	if (!_game) return false;
	Common::MemoryWriteStreamDynamic mem(DisposeAfterUse::YES);
	Common::Serializer s(nullptr, &mem);
	_game->synchronizeSave(s);
	size_t sz = (size_t)mem.size();
	out.resize((uint)sz);
	if (sz)
		memcpy(out.data(), mem.getData(), sz);
	return true;
}

void Comprehend::deserializeGameState(const Common::Array<byte> &in) {
	if (!_game) return;
	// Let the game swap in a different static database before the payload is
	// applied (Talisman loads its part-2 desert tables when restoring a desert
	// save). Must run before synchronizeSave reads the rooms/items into them.
	_game->prepareRestore(in);
	Common::MemoryReadStream mem(in.data(), in.size());
	Common::Serializer s(&mem, nullptr);
	_game->synchronizeSave(s);
	gmCMHourglassReset();  // undo jumps the sand back; snap, don't animate
	_hourglassFallPending = false;
}

void Comprehend::pushUndo() {
	Common::Array<byte> snapshot;
	if (!serializeGameState(snapshot))
		return;
	// Don't record an identical, back-to-back snapshot (e.g. when a turn is
	// re-run via REDO_TURN), so each entry is a distinct turn boundary.
	if (!_undoStack.empty() && _undoStack.back() == snapshot)
		return;
	_undoStack.push_back(snapshot);
	if (_undoStack.size() > kMaxUndo)
		_undoStack.remove_at(0);
}

bool Comprehend::undo() {
	// back() mirrors the current state; at least one earlier state is needed to
	// revert to.
	if (_undoStack.size() < 2)
		return false;
	_undoStack.pop_back();
	deserializeGameState(_undoStack.back());
	return true;
}

bool Comprehend::undoTurn(uint turns) {
	if (_undoStack.empty())
		return false;
	// back() is the start of the fatal turn (one turn back). Each additional
	// turn drops one more boundary off the top, but always keep one snapshot to
	// restore. After restoring, back() mirrors the restored state, matching the
	// invariant the rest of the undo machinery relies on.
	while (turns > 1 && _undoStack.size() > 1) {
		_undoStack.pop_back();
		--turns;
	}
	deserializeGameState(_undoStack.back());
	return true;
}

} // namespace Comprehend
} // namespace Glk
