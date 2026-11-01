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

// Comprehend engine glue: text output, the status window, transcripts and
// keyboard / script input.

namespace Glk {
namespace Comprehend {

void Comprehend::putBottom(const char *s) {
	// In redirect mode, send the text to the transcript only (or nowhere, if no
	// transcript is open); otherwise to the buffer window. Each destination has
	// its own trailing-newline counter so their collapse states stay separate.
	strid_t stream;
	int *trailing;
	if (_redirectToTranscript) {
		if (!_transcript) return;
		stream = _transcript;
		trailing = &_transcriptTrailingNewlines;
	} else {
		stream = glk_window_get_stream(_bottomWindow);
		trailing = &_trailingNewlines;
	}
	for (; *s; ++s) {
		if (*s == '\n') {
			if (*trailing >= 2)
				continue;  // already a blank line; collapse further newlines
			++*trailing;
		} else {
			*trailing = 0;
		}
		glk_put_char_stream(stream, (unsigned char)*s);
	}
}

void Comprehend::putBottomUni(const glui32 *s) {
	strid_t stream;
	int *trailing;
	if (_redirectToTranscript) {
		if (!_transcript) return;
		stream = _transcript;
		trailing = &_transcriptTrailingNewlines;
	} else {
		stream = glk_window_get_stream(_bottomWindow);
		trailing = &_trailingNewlines;
	}
	for (; *s; ++s) {
		if (*s == '\n') {
			if (*trailing >= 2)
				continue;  // already a blank line; collapse further newlines
			++*trailing;
		} else {
			*trailing = 0;
		}
		glk_put_char_stream_uni(stream, *s);
	}
}

void Comprehend::transcript(const char *arg) {
	// "#transcript off" stops; "#transcript [on]" starts; bare "#transcript" toggles.
	while (*arg == ' ') ++arg;
	if (scumm_stricmp(arg, "off") == 0)
		transcriptOff();
	else if (scumm_stricmp(arg, "on") == 0)
		transcriptOn();
	else if (_transcript)
		transcriptOff();
	else
		transcriptOn();
}

void Comprehend::transcriptOn() {
	if (_transcript) {
		print("A transcript is already active.\n");
		return;
	}

	frefid_t ref = glk_fileref_create_by_prompt(
		fileusage_TextMode | fileusage_Transcript, filemode_Write, 0);
	if (!ref)
		return;

	_transcript = glk_stream_open_file_uni(ref, filemode_Write, 0);
	glk_fileref_destroy(ref);

	if (!_transcript) {
		print("Failed to create transcript file.\n");
		return;
	}

	glk_put_string_stream(_transcript, "Start of transcript\n\n");
	_transcriptTrailingNewlines = 2;  // header ended with a blank line
	print("Transcript is now on.\n");
	glk_window_set_echo_stream(_bottomWindow, _transcript);

	// Record the room the player is currently standing in, so the transcript
	// doesn't begin mid-room with no description.
	if (_game)
		_game->transcribeCurrentRoom();
}

void Comprehend::transcriptOff() {
	if (!_transcript) {
		print("No transcript is currently running.\n");
		return;
	}

	glk_window_set_echo_stream(_bottomWindow, nullptr);
	glk_put_string_stream(_transcript, "\n\nEnd of transcript\n");
	glk_stream_close(_transcript, nullptr);
	_transcript = nullptr;
	print("Transcript is now off.\n");
}

void Comprehend::setCentered(bool on) {
	// Toggle the centred justification style on the buffer window (used for the
	// title credits). Routed through the same stream putBottom() writes to, so
	// subsequent console_println() output is centred until switched back.
	glk_set_style_stream(glk_window_get_stream(_bottomWindow),
						 on ? style_User1 : style_Normal);
}

void Comprehend::setPreformatted(bool on) {
	// Toggle a fixed-width (monospace) style on the buffer window. Used for the
	// few game strings that are hand-drawn ASCII boxes (e.g. Talisman's
	// "departing the Persian Empire" sign): only a monospace font with the
	// original spacing preserved keeps their dash rules and '!' borders aligned.
	glk_set_style_stream(glk_window_get_stream(_bottomWindow),
						 on ? style_Preformatted : style_Normal);
}

void Comprehend::print(const char *fmt, ...) {
	va_list argp;
	va_start(argp, fmt);
	Common::String msg = Common::String::vformat(fmt, argp);
	va_end(argp);
	putBottom(msg.c_str());
}

void Comprehend::print_u32_internal(const Common::U32String *fmt, ...) {
	Common::U32String out;
	va_list argp;
	va_start(argp, fmt);
	Common::U32String::vformat(fmt->begin(), fmt->end(), out, argp);
	va_end(argp);

	putBottomUni((const glui32 *)out.c_str());
}

void Comprehend::printRoomDesc(const Common::String &desc) {
	// Remember it so onArrange() can re-wrap it after a resize, and so text mode
	// can show it when the window opens.
	_lastRoomDesc = desc;

	if (!_statusWindow) return;

	glui32 width = 0;
	glk_window_get_size(_statusWindow, &width, nullptr);
	Common::String str = desc;
	if (width > 2)
		str.wordWrap(width - 2);
	str += '\n';

	// Size the window to the wrapped text plus one blank line at the bottom, used as a
	// delimiter, rather than leaving it at a fixed height with empty rows below the description.
	int lines = 0;
	for (uint i = 0; i < str.size(); ++i)
		if (str[i] == '\n') ++lines;
	winid_t parent = glk_window_get_parent(_statusWindow);
	if (parent)
		glk_window_set_arrangement(parent, winmethod_Above | winmethod_Fixed,
								   (glui32)(lines + 1), _statusWindow);

	glk_window_clear(_statusWindow);

	while (!str.empty()) {
		size_t idx = str.findFirstOf('\n');
		if (idx == Common::String::npos) idx = str.size() - 1;
		Common::String line = Common::String::format(" %s",
			Common::String(str.c_str(), str.c_str() + idx + 1).c_str());
		glk_put_string_stream(glk_window_get_stream(_statusWindow),
							  line.c_str());
		str = Common::String(str.c_str() + idx + 1);
	}

	// Fill the bottom row with underscores as a delimiter.
	glk_window_move_cursor(_statusWindow, 0, (glui32)lines);
	strid_t stream = glk_window_get_stream(_statusWindow);
	for (glui32 col = 0; col < width; ++col)
		glk_put_char_stream(stream, '_');
}

void Comprehend::readLine(char *buffer, size_t maxLen, bool stayOnEmptyLine) {
	maybeStartHourglassFall();
	event_t ev;
	glk_request_line_event(_bottomWindow, buffer, (glui32)(maxLen - 1), 0);
	// Glk can't echo only some lines, so turn its echo off and write the line
	// below instead. A request resets the echo flag, so this has to follow it.
	if (stayOnEmptyLine)
		glk_set_echo_line_event(_bottomWindow, 0);
	for (;;) {
		glk_select(&ev);
		if (ev.type == evtype_LineInput) break;
		if (ev.type == evtype_Quit) {
			glk_cancel_line_event(_bottomWindow, &ev);
			buffer[0] = 0;
			return;
		}
		if (ev.type == evtype_Timer) {
			if (_optionsChanged) applyChangedOptions();
			tickAnimations();
		} else if (ev.type == evtype_Arrange || ev.type == evtype_Redraw)
			onArrange();
	}
	buffer[ev.val1] = 0;
	if (stayOnEmptyLine) {
		glk_set_echo_line_event(_bottomWindow, 1);
		if (ev.val1 == 0)
			return;  // the cursor is still after the prompt
		// Glk has already sent the line to the transcript.
		strid_t stream = glk_window_get_stream(_bottomWindow);
		glk_window_set_echo_stream(_bottomWindow, nullptr);
		glk_set_style_stream(stream, style_Input);
		glk_put_buffer_stream(stream, buffer, ev.val1);
		glk_set_style_stream(stream, style_Normal);
		glk_put_char_stream(stream, '\n');
		glk_window_set_echo_stream(_bottomWindow, _transcript);
	}
	// Glk echoes the typed line plus a single newline to the buffer; reflect
	// that one trailing newline so the count stays accurate across the input.
	_trailingNewlines = 1;
}

int Comprehend::readChar() {
	maybeStartHourglassFall();
	glk_request_char_event(_bottomWindow);
	const bool savesWereDisabled = _disableSaves;
	setDisableSaves(true);
	event_t ev;
	ev.type = evtype_None;
	while (ev.type != evtype_CharInput) {
		glk_select(&ev);
		if (ev.type == evtype_Quit) {
			glk_cancel_char_event(_bottomWindow);
			setDisableSaves(savesWereDisabled);
			return -1;
		}
		if (ev.type == evtype_Timer) {
			if (_optionsChanged) applyChangedOptions();
			tickAnimations();
		} else if (ev.type == evtype_Arrange || ev.type == evtype_Redraw)
			onArrange();
	}
	setDisableSaves(savesWereDisabled);
	return ev.val1;
}

} // namespace Comprehend
} // namespace Glk
