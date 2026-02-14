# InfoBoard Console: Rolling Log Notes

This document captures what was tested and observed while validating rolling console output on the e-paper display,
along with the likely root causes and mitigation notes for future debugging.

## What was tested
- Sending numbered lines in console mode (`1`..`27`) shows ~25 visible lines, filling the screen in portrait mode.
- A long single-line message with many words produced no screen update.
- The same long message with explicit line breaks rendered correctly.
- Sending quoted text or manual JSON sometimes rendered raw JSON on screen.

## Root causes and behavior
- Serial input is newline-delimited. Each message must fit within `MAX_PAYLOAD_SIZE`, otherwise it is discarded and
  nothing is rendered. The limit is enforced in `src/activities/infoboard/InfoBoardActivity.h`.
- Console rendering uses a fixed line budget derived from font line height and margins (UI_12 font in portrait yields
  roughly 25 lines). Long lines wrap and consume multiple slots, so fewer logical lines fit.
- JSON parsing fails if the line is truncated or malformed (for example, manual JSON with unescaped quotes). When JSON
  parsing fails, the raw message is rendered, which looks like JSON on-screen.
- Double-encoding JSON (sending a JSON string through `--console`) results in JSON being shown as text.

## Current dev changes (in progress)
- Increased serial line buffer to 10 KB and added console buffer trimming.
- Console mode now renders the last visible lines (not the first).
- Added JSON flags: append, final, clear (batching without redraw until final).
- Updated `scripts/send_infoboard.py` to expose `--append`, `--final`, `--clear`.
- Updated `scripts/rolling_console_demo.py` default max bytes to 10 KB.

## Recommended usage
- Prefer `scripts/send_infoboard.py` for JSON payloads so strings are escaped correctly.
- To batch multiple lines without redrawing each time, send JSON with `append=true` and `final=false`, then finish with
  a `final=true` message. Use `clear=true` to reset the console buffer.
- Keep each serialized message under the configured line buffer limit to avoid dropped updates.
- For rolling log tests, trim payloads and/or send in chunks to avoid UART overflow:

```sh
python3 scripts/rolling_console_demo.py <port> --max-bytes 10240 --chunk-size 64 --chunk-delay 0.01
```

## Where to look in code
- Buffer limits and serial handling: `src/activities/infoboard/InfoBoardActivity.h`
- Console rendering and JSON parsing: `src/activities/infoboard/InfoBoardActivity.cpp`
- CLI helpers: `scripts/send_infoboard.py`, `scripts/rolling_console_demo.py`
