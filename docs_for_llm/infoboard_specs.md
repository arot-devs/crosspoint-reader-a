# InfoBoard Serial Protocol (v1)

InfoBoard is a dedicated activity that renders text pushed over USB serial. Messages are line-delimited: each `\n`
terminates one payload. The protocol supports both plain text and JSON messages.

## Transport
- USB serial, 115200 baud.
- Each **line** is one message. Lines longer than the firmware buffer are dropped (no update).
- Carriage returns (`\r`) are ignored on input.

## Limits
- **Max payload**: 10 KB per line (`MAX_PAYLOAD_SIZE = 10240` bytes), including JSON and escaping.
- **Console buffer**: last 10 KB of console text retained.
- **Visible lines**: about **25 lines** in portrait (UI_12 font), depending on line wrapping.
- **Line width**: proportional font; max width is ~440 px. ASCII text fits roughly **60–70 chars per line** depending
  on characters.

If a payload starts with `{` and JSON parsing fails, the line is **dropped** to avoid rendering raw JSON.

## Message Formats

### 1) Plain text (default/centered)
Any non-JSON line is rendered as centered text (word-wrapped).

Example:
```
Hello InfoBoard\n
```

### 2) JSON console mode (top-left, wraps, preserves spaces)

Minimal:
```
{"mode":"console","text":"Line 1\nLine 2"}
```

#### Fields
- `mode`: set to `"console"` to enable console rendering.
- `text`: the text to render or append.
- `append` (bool, optional): if true, append to console buffer instead of replacing it.
- `final` (bool, optional): if true, trigger a render immediately. If omitted, defaults to `false` when `append=true`.
- `clear` (bool, optional): if true, clear console buffer before applying `text`.

#### Batching examples
Append multiple chunks without redraw, then render:
```
{"mode":"console","append":true,"text":"line 1\n"}
{"mode":"console","append":true,"text":"line 2\n"}
{"mode":"console","append":true,"text":"line 3\n","final":true}
```

Clear then render:
```
{"mode":"console","clear":true,"final":true}
```

## Button Events (device → host)
When InfoBoard is active, button releases are sent to the host as JSON lines **prefixed** with `IBEV ` for easy
filtering. This allows bidirectional flows (e.g., host updates display on button presses).

Example:
```
IBEV {"v":1,"event":"button","button":"down","action":"release"}
```

Buttons emitted:
- `back` (exits InfoBoard after sending the event)
- `confirm`
- `up`, `down`, `left`, `right`
- `page_back`, `page_forward`

## Rendering details
- Console mode uses UI_12 font and wraps at the screen width.
- The **last** visible lines are rendered (rolling-console behavior).
- Centered mode word-wraps and vertically centers content.

## Suggested tools
- `scripts/send_infoboard.py` (supports console append/clear/final flags)
- `scripts/rolling_console_demo.py`
- `scripts/infoboard_btop_demo.py`
