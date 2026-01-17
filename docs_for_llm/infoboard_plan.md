# InfoBoard Plan (Serial-Driven, Dedicated Mode)

## Goals
- Dedicated “InfoBoard” activity that updates the e‑ink display on push from a connected host.
- Use **USB serial** as the transport (reliable, no Wi‑Fi/AP). Device can stay plugged in.
- Safe changes: no impact on bootloader/flashing, no blocking reads, no changes to USB‑CDC flags.
- Extensible protocol: start with simple text lines; allow future “draw list” commands for layout control.

## Constraints & Assumptions
- ESP32‑C3 has ~380KB usable RAM. Avoid large buffers or heavy parsing.
- E‑ink refresh is slow and can ghost if updated too frequently.
- Dedicated mode only: updates happen only when the InfoBoard activity is active.
- Serial is already used for logs; reading from Serial is safe if non‑blocking and activity‑scoped.

## Phase 0: Protocol Definition (minimal + extensible)
- **MVP** (line-based): each `\n`‑terminated line becomes the full‑screen message.
- **Extended** (JSON draw list): 1 JSON object per line.
  - Example:
    ```json
    {"v":1,"refresh":"FAST","cmds":[
      {"op":"clear","color":1},
      {"op":"rect","x":20,"y":20,"w":440,"h":120,"color":1},
      {"op":"fill","x":22,"y":22,"w":436,"h":116,"color":0},
      {"op":"text","font":"UI_12","x":30,"y":40,"text":"Download finished","color":1,"style":"BOLD"}
    ]}
    ```
  - Supported ops (initial): `clear`, `rect`, `fill`, `line`, `text`.
  - Fonts: map string IDs → existing font IDs (`UI_10`, `UI_12`, `SMALL`).
  - Refresh: `FAST` default; optionally allow `HALF`/`FULL`.
- **Limits**: payload size 4–8KB; command count <= 100; reject/ignore oversized payloads.

## Phase 1: New InfoBoard Activity
- Create `src/activities/infoboard/InfoBoardActivity.{h,cpp}`.
- Behavior:
  - On enter: optional `Serial.begin(115200)` if not active.
  - In `loop()`: non‑blocking read from `Serial.available()` into a small buffer.
  - On newline: parse payload.
    - If it starts with `{`, treat as JSON draw list.
    - Otherwise treat as plain text (centered).
  - Re-render only when new valid payload arrives (rate‑limit if needed).
- Rendering:
  - **MVP**: centered text similar to `FullScreenMessageActivity`.
  - **JSON draw list**: map ops to `GfxRenderer` calls.
- Prevent auto‑sleep while in InfoBoard mode via `preventAutoSleep()`.

## Phase 2: Home Menu Entry
- Add “InfoBoard” to the Home menu list and selection logic.
- Wire a new `onGoToInfoBoard()` in `src/main.cpp`.

## Phase 3: Basic Robustness
- Add a small error banner or fallback if JSON parse fails.
- Hard cap buffer size (drop if exceeds limit).
- Optional: store last successful payload in memory for redraws.

## Phase 4: Optional Enhancements
- Partial updates using `displayWindow()` for low‑ghosting text areas.
- Periodic full refresh after N updates (e.g., every 20–50 updates) to clear ghosting.
- SD‑card image references for static assets (not streamed over serial).

## Safety Notes
- No changes to bootloader / eFuses / USB‑CDC flags.
- Keep serial reads non‑blocking to avoid watchdog issues.
- Activity‑scoped reads only; other activities unaffected.

## Testing Checklist
- Enter InfoBoard mode, send a plain text line, confirm it renders.
- Send a small JSON draw list, confirm shapes/text render.
- Ensure device still flashes normally.
- Check no watchdog resets when idle.


## Extra notes

refer to BUILD_FIRMWARE.md for instructions on building the firmware.