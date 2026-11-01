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

#ifndef GLK_COMPREHEND_COMPREHEND_H
#define GLK_COMPREHEND_COMPREHEND_H

#include "common/scummsys.h"
#include "glk/glk_api.h"
#include "glk/window_graphics.h"
#include "glk/window_text_buffer.h"
#include "glk/window_text_grid.h"
#include "glk/comprehend/disk_image.h"
#include "glk/comprehend/game.h"
#include "glk/comprehend/game_traits.h"
#include "glk/comprehend/gm_backend.h"



namespace Glk {
namespace Comprehend {

class DrawSurface;
class Pics;

enum {
	GLK_BUFFER_ROCK = 1,
	GLK_STATUS_ROCK = 1010,
	GLK_GRAPHICS_ROCK = 1020
};

#define EXTRA_STRING_TABLE(x) (0x8200 | (x))

struct GameStrings {
	uint16 game_restart;
};

class Comprehend : public GlkAPI {
private:
	int _saveSlot;
	bool _graphicsEnabled;
	// The picture window is closed because the host has graphics switched
	// off, not because the game has no pictures; see showGraphics().
	bool _closedBySetting = false;
	bool _disableSaves;
	const GameTraits *_traits = nullptr;   // set with the id; null for an unknown id
	// Last room description shown in the status grid, kept so it can be re-wrapped
	// to the new width when the window is resized.
	Common::String _lastRoomDesc;

	// Count of consecutive newlines just written to the bottom buffer window. Runs are
	// collapsed so the engine (which is liberal with '\n') never emits more than two in a
	// row, i.e. at most one blank line. Starts at 2 so any leading newlines before the
	// first real output are suppressed entirely.
	int _trailingNewlines = 2;

	// Open transcript file stream (echo of the bottom window), or null. Driven
	// by the #transcript metacommand.
	strid_t _transcript = nullptr;

	// When set, print() output is sent to the transcript only, bypassing the
	// buffer window. Used for room descriptions, which are shown in the status
	// grid rather than the scroll-back buffer but still belong in a log.
	bool _redirectToTranscript = false;
	// Separate trailing-newline counter for the transcript-redirect path, so it
	// doesn't clobber the buffer window's collapse state.
	int _transcriptTrailingNewlines = 2;

	// Undo history: serialized game-state snapshots, one per turn (oldest
	// first). The back() entry mirrors the current state; #undo drops it and
	// restores the entry below. Capped at kMaxUndo to bound memory.
	static const size_t kMaxUndo = 100;
	Common::Array<Common::Array<byte> > _undoStack;
	bool serializeGameState(Common::Array<byte> &out);
	void deserializeGameState(const Common::Array<byte> &in);

public:
	winid_t _topWindow;     // graphics window (or null if disabled)
	winid_t _statusWindow;  // text grid showing the room description (text mode only)
	winid_t _bottomWindow;  // main text buffer
	DrawSurface *_drawSurface;
	ComprehendGame *_game;
	Pics *_pics;
	uint _drawFlags;

	// Pixel scale used when blitting the render buffer to the Glk graphics
	// window: one 280-wide column, and one picture row, is _pixelSize window
	// pixels. The Apple II renders 560 columns over the same physical width, so
	// there _pixelSize is the vertical height and the horizontal column step is
	// half of it; see pixelStepX().
	int _pixelSize;

	// The higher-colour renderer is selected: Apple II double hi-res ("<D>" mode,
	// 560-wide draw surface) on a disk image, IBM PCjr / Tandy 16 colours on a
	// DOS v1 game. Only ever set when that renderer's drawing tables loaded; see
	// gfxBackend() for the renderer it resolves to.
	bool _moreColours = false;

	// The player's preferences (the comprehend_slow_draw and
	// comprehend_more_colours game options): reveal pictures progressively, and
	// draw with the higher-colour renderer where the game ships one.
	bool _slowDraw = true;
	int _preferredGraphics = 0;

	// Set when the options were changed from the main menu, which opens in the
	// middle of glk_select(). The change is applied on the next timer event.
	bool _optionsChanged = false;
	void readOptions();
	void applyChangedOptions();

	// Last value of _preferredGraphics that applyPreferredGraphicsMode() acted on.
	// onArrange() re-reads the preference on every arrange and reacts only to a change,
	// so a plain window resize doesn't clobber a manual #dhgr/#pcjr override, while a
	// genuine change (or the corrected theme that arrives in a post-handshake arrange)
	// does switch the renderer. -1 = not yet applied.
	int _appliedComprehendGraphics = -1;

	// True when pictures have somewhere to go.
	bool hasPictureTarget() const {
		return _topWindow != nullptr;
	}

	// Last single picture handed to drawPicture(). repaintCurrentScene() uses it to
	// re-render after a graphics-mode switch when there is no room scene to replay,
	// notably the title screen, which beforeGame() draws via drawPicture() directly.
	// (uint)-1 means nothing has been drawn yet.
	uint _lastDrawnPicture = (uint)-1;

protected:
	void createConfiguration() override;
	void createDebugger() override;
	void initGraphicsMode() override;

public:
	Comprehend(OSystem *syst, const GlkGameDescription &gameDesc);
	~Comprehend() override;

	InterpreterType getInterpreterType() const override {
		return INTERPRETER_SCOTT;
	}

	void runGame() override;
	void applyGameSettings() override;

	// What the shared code needs to know about this game. Valid once
	// createGame() has accepted the id.
	const GameTraits &traits() const { return *_traits; }

	bool loadingSavegame() const { return _saveSlot >= 0; }
	bool loadLauncherSavegameIfNeeded();

	Common::Error readSaveData(Common::SeekableReadStream *rs) override;
	Common::Error writeGameData(Common::WriteStream *ws) override;

	bool canLoadGameStateCurrently(Common::U32String *msg = nullptr) override {
		return _game != nullptr;
	}
	bool canSaveGameStateCurrently(Common::U32String *msg = nullptr) override {
		return !_disableSaves && _game != nullptr;
	}
	void setDisableSaves(bool flag) { _disableSaves = flag; }

	// Output
	void print(const char *fmt, ...) GCC_PRINTF(2, 3);
	template<class... TParam>
	void print(const Common::U32String &fmt, TParam... param);

	void printRoomDesc(const Common::String &desc);
	void setCentered(bool on);
	void setPreformatted(bool on);
	// With stayOnEmptyLine, an empty line leaves the cursor where it was, after
	// the prompt, instead of moving to a new line.
	void readLine(char *buffer, size_t maxLen, bool stayOnEmptyLine = false);
	int readChar();

	// Graphics: render to _drawSurface then blit to _topWindow.
	void drawPicture(uint pictureNum);
	// Render a whole scene (a room picture plus any item overlays) as one
	// slow-draw reveal: the pictures are recorded into a single op list and
	// revealed in paint order. drawPicture() is the single-picture wrapper.
	// Drawing the pictures with separate drawPicture() calls would make each
	// one snap the previous picture's reveal (only the last would animate).
	void drawPictureList(const Common::Array<uint> &pics);
	void drawLocationPicture(int pictureNum, bool clearBg = true);
	void drawItemPicture(int pictureNum);
	void clearScreen(bool isBright);
	// Redraw just The Coveted Mirror's right-hand hourglass panel at the current sand level
	// (VM variable 0x11), without touching the room picture, and arm the freed-grain fall.
	// The room only repaints when it changes, so this gives the hourglass a per-turn update.
	// A no-op outside CM or before the panel is built. Call only on turns with no full room
	// repaint queued (that path draws the hourglass itself, snapping on the room change).
	void refreshCMHourglass();
	// Paint a whole scene that replaces the screen (a background-clearing
	// location plus any item pictures layered on top). If the requested set of
	// pictures is identical to what is already composited, the reveal is skipped
	// and the scene is painted instantly; see _screenComposition.
	void paintBackground(const Common::Array<uint> &pics);
	// Layer pictures on top of the current screen without clearing it. Pictures
	// already present are repainted instantly rather than re-animated.
	void paintOverlay(const Common::Array<uint> &pics);
	bool toggleGraphics();
	void showGraphics();
	// Switch to text mode: close the picture window and open the room description
	// window. Also used when a game ships without any graphics files, so the
	// player gets a plain text game instead of an empty picture pane.
	void hideGraphics();

	// Apply the preferred-graphics-mode setting (_preferredGraphics):
	// 0 = more colours (enable PCjr or DHGR if drawing tables are available),
	// 1 = less colours (keep CGA / standard hi-res). Called after loadGame() so
	// the drawing tables are already installed, and again from onArrange() so a
	// mid-game change of the preference (or the real theme arriving in a later
	// arrange) takes effect. Idempotent: re-applies only when the value changed
	// since last time. Returns true if it switched the active renderer.
	bool applyPreferredGraphicsMode();

	// Re-render the scene currently composited (the last room + its items)
	// through the active renderer. Used after an on-the-fly graphics-mode switch,
	// when the persistent surface still holds the picture drawn by the previous
	// renderer, so a plain re-blit would show the old mode.
	void repaintCurrentScene();

	// The renderer that draws this game's pictures right now.
	GfxBackend gfxBackend() const;
	// Whether the game shipped a higher-colour renderer to switch to (double
	// hi-res on an Apple II disk with a <D> interpreter, PCjr on a DOS v1 game
	// with JR_GRAPH.OVR).
	bool moreColoursAvailable() const;
	// Switch between the default renderer (standard hi-res / CGA) and the
	// higher-colour one (double hi-res / PCjr). Clears the window; the caller
	// repaints the picture.
	void setMoreColours(bool on);
	bool moreColours() const { return _moreColours; }
	// The Apple II renderer (and its panel verbs) that The Coveted Mirror's
	// hourglass lives on.
	const GmBackend *cmBackend() const;
	const GmCMPanelOps *cmPanelOps() const { return cmBackend()->cm; }
	// The Apple II renderers (standard and double hi-res) fill a 560-wide surface,
	// one column per NTSC dot, so colour edges land on the right dots. The DOS
	// renderers fill 280 columns.
	bool appleRender() const {
		GfxBackend b = gfxBackend();
		return b == GFX_APPLE_HIRES || b == GFX_APPLE_DHGR;
	}
	int renderWidth() const;
	// Horizontal width (in window pixels) of one render column: full _pixelSize
	// for the 280-wide DOS surface, half for the 560-wide Apple II one so the
	// picture keeps the same physical size/aspect.
	int pixelStepX() const {
		return appleRender() ? (_pixelSize > 1 ? _pixelSize / 2 : 1) : _pixelSize;
	}
	// Logical text/graphics-mode switch that keeps the picture window open
	// (used by Talisman's mode-switch opcodes), unlike toggleGraphics() which
	// genuinely opens/closes the window for the player's manual toggle.
	void setGraphicsMode(bool graphicsMode);
	bool isGraphicsEnabled() const { return _graphicsEnabled; }

	ComprehendGame *getGame() const { return _game; }
	bool isInputLineActive() const {
		return _bottomWindow && (_bottomWindow->_lineRequest || _bottomWindow->_lineRequestUni);
	}

	// Blits the current state of _drawSurface to the Glk graphics window,
	// pixel-by-pixel via glk_window_fill_rect. Called by drawPicture().
	void blitSurfaceToWindow();
	// Blits only Apple rows [y0,y1] of _drawSurface (used by the slow-draw
	// reveal to repaint just the band it changed this tick).
	void blitSurfaceRowsToWindow(int y0, int y1);

	// Window-resize / repaint handling (evtype_Arrange / evtype_Redraw), called
	// from the input-wait loops: rescale the picture to the new window, repaint
	// it, and re-wrap the status description.
	void onArrange();
	// Pick the largest integer pixel scale of the 280x160 render that fits the
	// window (width, and up to ~60% of the screen height), resizing the graphics
	// window to suit. Returns true if the scale changed.
	bool recomputeGraphicsScale();

private:
	// True while a slow-draw reveal is running in the background (timer events
	// are active and the input loops are advancing it tick by tick).
	bool _slowDrawActive;

	// When set, drawPicture() paints instantly even if slow-draw is on. The
	// paintBackground()/paintOverlay() scene helpers set this while repainting a scene
	// identical to the one already on screen (e.g. Talisman's OPCODE_DRAW_ROOM repainting
	// the current room after a speech), since re-animating the same image looks like a stutter.
	bool _suppressSlowDraw;

	// The pictures currently composited on screen, in paint order: a background
	// (location / dark / bright) followed by any item pictures layered on top.
	// Used by paintBackground()/paintOverlay() to detect repaints that change
	// nothing. Maintained across every draw path (per-turn refresh, explicit
	// draw opcodes, scripted cutscenes) so they share one notion of the screen.
	Common::Array<uint> _screenComposition;

	// True while The Coveted Mirror's per-turn grain-fall animation is running.
	// Shares the same Glk timer as the slow-draw reveal; the input loops advance
	// whichever is active. _hgTickAccum subdivides the fast slow-draw tick so the
	// grain steps at a visible pace.
	bool _hourglassFalling = false;
	int _hgTickAccum = 0;
	// Set when a grain fall is armed during a room reveal (a move): the reveal owns the
	// timer and the panel band, so the fall is held until the reveal finishes (tickSlowDraw)
	// and then started, making the grain drop visible on move turns too.
	bool _hourglassFallPending = false;

	// When tickAnimations() last caught up, in g_system->getMillis() time.
	uint32 _animationClock = 0;

	// Run the active animations for the timer ticks that have passed since the
	// last call. Called for each Glk timer event.
	void tickAnimations();
	// Advance the background slow-draw reveal by the given number of timer ticks
	// and blit the changed rows. Cancels the timer and clears _slowDrawActive when
	// done.
	void tickSlowDraw(uint32 ticks);
	// Advance the grain-fall animation (paced down from the timer tick) and blit
	// its dirty band. Clears _hourglassFalling when the fall completes.
	void tickHourglass(uint32 ticks);
	// Start the grain-fall animation if gmDrawCMHourglass() flagged a single-grain
	// drop this turn, with the picture window open and slow-draw enabled. If a
	// room reveal is running (a move), defer it via _hourglassFallPending until
	// the reveal finishes rather than dropping it, so the grain falls every turn.
	void maybeStartHourglassFall();
	// Actually kick off the grain fall (shared by the immediate and deferred
	// paths). Assumes the preconditions in maybeStartHourglassFall() are met.
	void beginHourglassFall();
	// Request the Glk timer iff either background animation needs it.
	void updateTimerRequest();
	// Complete the slow-draw reveal immediately (called before starting a new
	// picture or when the window is resized). Updates _drawSurface to the final
	// fully-drawn page; the caller is responsible for blitting to the window.
	void finishSlowDraw();

	void initialize();
	void deinitialize();
	void createGame();
	void print_u32_internal(const Common::U32String *fmt, ...);

	// Write to the bottom window stream, collapsing consecutive newlines into
	// a single one (tracking state across calls via _lastBottomChar).
	void putBottom(const char *s);
	void putBottomUni(const glui32 *s);

public:
	// #transcript metacommand: echo the bottom-window output to a text file.
	// transcript() toggles (or follows an explicit "on"/"off" argument).
	void transcript(const char *arg);
	void transcriptOn();
	void transcriptOff();

	// Route subsequent print() output to the transcript only (true) or back to
	// the buffer window (false). See _redirectToTranscript.
	void redirectOutputToTranscript(bool on) { _redirectToTranscript = on; }

	// #undo support. pushUndo() records the current state as a turn boundary
	// (deduplicated against the last snapshot); undo() reverts to the previous
	// turn's state, returning false (with no change) if there is nothing to
	// undo.
	void pushUndo();
	bool undo();
	// Step back `turns` turn boundaries, restoring that snapshot, for undoing a death:
	// by then the current state has advanced past the top snapshot (the start of the fatal
	// turn), so undoing one turn restores back() itself rather than popping to the previous
	// turn. Talisman's deaths are delayed countdowns, so its death menu undoes two turns.
	// Stops short if the history is shorter than requested; returns false (no change) if empty.
	bool undoTurn(uint turns = 1);
	void clearUndo() { _undoStack.clear(); }
};

template<class... TParam>
void Comprehend::print(const Common::U32String &fmt, TParam... param) {
	print_u32_internal(&fmt, Common::forward<TParam>(param)...);
}

extern Comprehend *g_comprehend;

} // namespace Comprehend
} // namespace Glk

#endif
