#!/usr/bin/env python3
import argparse
import json
import sys
import time
from collections import deque

try:
    import serial
    from serial.tools import list_ports
except ImportError:  # pragma: no cover - runtime dependency check
    sys.exit("pyserial is required. Install with: pip install pyserial")


def detect_ports() -> list[str]:
    ports: list[str] = []
    for port in list_ports.comports():
        ports.append(port.device)
    return ports


def pick_default_port(ports: list[str]) -> str | None:
    if not ports:
        return None
    if len(ports) == 1:
        return ports[0]
    preferred_prefixes = (
        "/dev/cu.usbmodem",
        "/dev/cu.usbserial",
        "/dev/ttyACM",
        "/dev/ttyUSB",
        "COM",
    )
    for prefix in preferred_prefixes:
        for port in ports:
            if port.startswith(prefix):
                return port
    return None


CODEX_LINES = [
    "- The console shows raw JSON, indicating JSON parsing failed -- likely due to truncated or double-encoded input",
    "  despite increased payload size. The logic falls back to centered display when parsing fails, explaining the JSON",
    "  string appearing as raw text. Next, I'll verify how rolling_console_demo and send_infoboard handle JSON encoding",
    "  and chunking to pinpoint the parsing issue.",
    "",
    "  Identifying truncation and firmware mismatch",
    "",
    "  The raw JSON display and stuck render queue likely result from truncated JSON messages exceeding MAX_PAYLOAD_SIZE,",
    "  causing discards. The rolling console demo trims payloads to 10k bytes, matching firmware limits, but if the",
    "  firmware isn't updated or uses a lower limit, truncation and parse failures occur. The displayed raw JSON inside",
    "  the console buffer suggests partial JSON data got appended due to failed parsing and buffer handling.",
    "  Next, I'll confirm firmware version and MAX_PAYLOAD_SIZE alignment and verify console buffer trimming logic.",
    "",
    "  Confirming firmware update and planning append-based console fix",
    "",
    "  The raw JSON and stuck console likely come from running old firmware with 2048-byte limit while sending 10k",
    "  payloads from updated scripts. They must flash the new firmware with the larger buffer and last-lines rendering.",
    "  To improve stability and avoid JSON truncation, rolling_console_demo should switch to sending incremental console",
    "  lines using append/final flags, drastically reducing payload size and preventing serial overflow.",
    "",
    "  Planning incremental console updates and parse failure guard",
    "",
    "  I'll update rolling_console_demo to send incremental lines using append/final flags by default, greatly reducing",
    "  payload size and avoiding truncation issues. Also, in firmware's serial input handler, I'll add a check to ignore",
    "  parse failures on JSON-looking messages to prevent raw JSON rendering. This combined approach improves console",
    "  robustness and user experience without breaking existing workflows.",
    "",
    "- Edited src/activities/infoboard/InfoBoardActivity.cpp (+5 -0)",
    "    92",
    "    93 +      if (!isJson && !message.empty() && message.front() == '{') {",
    "    94 +        Serial.printf(\"[%lu] [INFO] Dropping invalid JSON payload\\n\", millis());",
    "    95 +        continue;",
    "    96 +      }",
    "    97 +",
    "    98        if (!isJson && message == lastMessage) {",
    "",
    "- Edited scripts/rolling_console_demo.py (+35 -10)",
    "    79",
    "    80 +def build_append_payload(text: str, final: bool) -> str:",
    "    81 +    payload: dict[str, object] = {\"mode\": \"console\", \"append\": True, \"text\": text}",
    "    82 +    if final:",
    "    83 +        payload[\"final\"] = True",
    "    84 +    return json.dumps(payload)",
    "    85 +",
    "    86 +",
    "    87  def payload_bytes(text: str) -> bytes:",
    "       ...",
    "   150          default=20,",
    "   144 -        help=\"Lines to keep in view (match display capacity; default: 20)\",",
    "   151 +        help=\"Lines to keep in view for full-frame mode (default: 20)\",",
    "   152      )",
    "       ...",
    "   156          default=MAX_PAYLOAD_BYTES,",
    "   150 -        help=f\"Max JSON payload size before trimming (default: {MAX_PAYLOAD_BYTES})\",",
    "   157 +        help=f\"Max JSON payload size before trimming in full-frame mode (default: {MAX_PAYLOAD_BYTES})\",",
    "   158      )",
    "       ...",
    "   170      )",
    "   171 +    parser.add_argument(",
    "   172 +        \"--full-frame\",",
    "   173 +        action=\"store_true\",",
    "   174 +        help=\"Send full-screen payloads each update (legacy mode).\",",
    "   175 +    )",
    "   176 +    parser.add_argument(",
    "   177 +        \"--clear-first\",",
    "   178 +        action=\"store_true\",",
    "   179 +        help=\"Clear the console buffer before starting (append mode only).\",",
    "   180 +    )",
    "   181      parser.add_argument(\"--list\", action=\"store_true\", help=\"List available serial ports and exit\")",
    "       ...",
    "   215",
    "   216 +    mode_label = \"full-frame\" if args.full_frame else \"append\"",
    "   217      print(",
    "   218          \"Rolling console demo: \"",
    "   201 -        f\"{args.lines} lines, {args.interval:.2f}s/line for {args.duration:.1f}s.\")",
    "   219 +        f\"{args.lines} lines, {args.interval:.2f}s/line for {args.duration:.1f}s ({mode_label}).\")",
    "   220      )",
    "       ...",
    "   228          warned_trim = False",
    "   229 +        if not args.full_frame and args.clear_first:",
    "   230 +            clear_payload = json.dumps({\"mode\": \"console\", \"clear\": True, \"final\": True})",
    "   231 +            write_payload(ser, clear_payload.encode(\"utf-8\"), args.chunk_size, args.chunk_delay)",
    "   232          while True:",
    "       ...",
    "   236              log_line = format_log_line(line_index, start_time)",
    "   216 -            buffer.append(log_line)",
    "   217 -            trimmed = trim_buffer_to_fit(buffer, args.max_bytes)",
    "   218 -            if trimmed and not warned_trim:",
    "   219 -                print(\"Trimmed payload to fit max bytes; reduce --lines or --max-bytes for full history.\")",
    "   221 -            payload = payload_bytes(\"\\n\".join(buffer))",
    "   222 -            write_payload(ser, payload, args.chunk_size, args.chunk_delay)",
    "   237 +            if args.full_frame:",
    "   238 +                buffer.append(log_line)",
    "   239 +                trimmed = trim_buffer_to_fit(buffer, args.max_bytes)",
    "   240 +                if trimmed and not warned_trim:",
    "   241 +                    print(\"Trimmed payload to fit max bytes; reduce --lines or --max-bytes for full history.\")",
    "   242 +                    warned_trim = True",
    "   243 +                payload = payload_bytes(\"\\n\".join(buffer))",
    "   245 +            else:",
    "   246 +                payload = build_append_payload(log_line + \"\\n\", final=True).encode(\"utf-8\")",
    "   248              line_index += 1",
    "",
    "- The payload handling properly appends newlines after encoding JSON strings to bytes, matching expectations in",
    "  write_payload. The firmware now tolerates invalid JSON with a 10k line buffer, so I'll rebuild it and copy the",
    "  output to _custom_firmware for deployment.",
    "",
    "-> Interacted with background terminal · /home/ubuntu/.local/bin/platformio run",
    "  (waited)",
    "",
    "- Ran /home/ubuntu/.local/bin/platformio run",
    "  Processing default (platform: espressif32 @ 6.12.0; board: esp32-c3-devkitm-1; framework: arduino)",
    "  ...",
    "  default        SUCCESS   00:00:06.894",
    "",
    "- Ran cp .pio/build/default/firmware.bin /home/ubuntu/dev/crosspoint-reader/_custom_firmware/crosspoint_infoboard.bin",
    "  (no output)",
    "",
    "- Ran git status -sb",
    "  ## master...origin/master",
    "   M _custom_firmware/crosspoint_infoboard.bin",
    "   M scripts/rolling_console_demo.py",
    "   M src/activities/infoboard/InfoBoardActivity.cpp",
]

MAX_PAYLOAD_BYTES = 10240


def format_log_line(counter: int, start_time: float) -> str:
    _ = start_time
    return CODEX_LINES[counter % len(CODEX_LINES)]


def build_payload(text: str) -> str:
    return json.dumps({"mode": "console", "text": text})


def build_append_payload(text: str, final: bool) -> str:
    payload: dict[str, object] = {"mode": "console", "append": True, "text": text}
    if final:
        payload["final"] = True
    return json.dumps(payload)


def payload_bytes(text: str) -> bytes:
    return build_payload(text).encode("utf-8")


def truncate_line_to_fit(line: str, max_bytes: int) -> str:
    if not line or max_bytes <= 0:
        return ""
    suffix = "..."
    low = 0
    high = len(line)
    best = ""
    while low <= high:
        mid = (low + high) // 2
        candidate = line[:mid]
        if mid < len(line):
            candidate = candidate.rstrip() + suffix
        if len(payload_bytes(candidate)) <= max_bytes:
            best = candidate
            low = mid + 1
        else:
            high = mid - 1
    return best


def trim_buffer_to_fit(buffer: deque[str], max_bytes: int) -> bool:
    trimmed = False
    if max_bytes <= 0:
        return trimmed
    while len(buffer) > 1 and len(payload_bytes("\n".join(buffer))) > max_bytes:
        buffer.popleft()
        trimmed = True
    if buffer and len(payload_bytes(buffer[-1])) > max_bytes:
        buffer[-1] = truncate_line_to_fit(buffer[-1], max_bytes)
        trimmed = True
    return trimmed


def write_payload(ser: serial.Serial, payload: bytes, chunk_size: int, chunk_delay: float) -> None:
    packet = payload + b"\n"
    if chunk_size <= 0:
        ser.write(packet)
        ser.flush()
        return
    for offset in range(0, len(packet), chunk_size):
        ser.write(packet[offset : offset + chunk_size])
        if chunk_delay > 0 and offset + chunk_size < len(packet):
            time.sleep(chunk_delay)
    ser.flush()


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Stream a rolling console log to the InfoBoard display."
    )
    parser.add_argument("port", nargs="?", help="Serial port (e.g. /dev/ttyACM0 or COM3)")
    parser.add_argument("--port", dest="port_override", help="Serial port (overrides positional port)")
    parser.add_argument("--baud", type=int, default=115200, help="Baud rate (default: 115200)")
    parser.add_argument("--delay", type=float, default=0.5, help="Seconds to wait after opening port")
    parser.add_argument("--duration", type=float, default=30, help="Seconds to run (default: 30)")
    parser.add_argument("--interval", type=float, default=1, help="Seconds between lines (default: 1)")
    parser.add_argument(
        "--lines",
        type=int,
        default=20,
        help="Lines to keep in view for full-frame mode (default: 20)",
    )
    parser.add_argument(
        "--max-bytes",
        type=int,
        default=MAX_PAYLOAD_BYTES,
        help=f"Max JSON payload size before trimming in full-frame mode (default: {MAX_PAYLOAD_BYTES})",
    )
    parser.add_argument(
        "--chunk-size",
        type=int,
        default=0,
        help="Write payload in chunks to avoid UART overflow (0 disables)",
    )
    parser.add_argument(
        "--chunk-delay",
        type=float,
        default=0.01,
        help="Seconds to sleep between chunks (default: 0.01)",
    )
    parser.add_argument(
        "--full-frame",
        action="store_true",
        help="Send full-screen payloads each update (legacy mode).",
    )
    parser.add_argument(
        "--clear-first",
        action="store_true",
        help="Clear the console buffer before starting (append mode only).",
    )
    parser.add_argument("--list", action="store_true", help="List available serial ports and exit")
    args = parser.parse_args()

    if args.list:
        ports = detect_ports()
        if not ports:
            print("No serial ports detected.")
        else:
            print("Detected serial ports:")
            for port in ports:
                print(f"  {port}")
        return 0

    if args.duration <= 0:
        sys.exit("Duration must be > 0 seconds.")
    if args.interval <= 0:
        sys.exit("Interval must be > 0 seconds.")
    if args.lines <= 0:
        sys.exit("Lines must be > 0.")
    if args.max_bytes < 0:
        sys.exit("Max bytes must be >= 0.")
    if args.chunk_size < 0:
        sys.exit("Chunk size must be >= 0.")
    if args.chunk_delay < 0:
        sys.exit("Chunk delay must be >= 0.")

    ports = detect_ports()
    port = args.port_override or args.port
    if not port:
        port = pick_default_port(ports)
        if not port:
            if not ports:
                sys.exit("No serial ports detected. Use --list to view ports.")
            sys.exit("Multiple serial ports detected. Pass one explicitly or use --list.")

    mode_label = "full-frame" if args.full_frame else "append"
    print(
        "Rolling console demo (codex-style): "
        f"{args.lines} lines, {args.interval:.2f}s/line for {args.duration:.1f}s ({mode_label})."
    )
    print("Press Ctrl+C to stop.")
    with serial.Serial(port, args.baud, timeout=1) as ser:
        if args.delay > 0:
            time.sleep(args.delay)
        buffer = deque(maxlen=args.lines)
        start_time = time.time()
        line_index = 0
        warned_trim = False
        if not args.full_frame and args.clear_first:
            clear_payload = json.dumps({"mode": "console", "clear": True, "final": True})
            write_payload(ser, clear_payload.encode("utf-8"), args.chunk_size, args.chunk_delay)
        while True:
            next_tick = start_time + (line_index * args.interval)
            if next_tick - start_time >= args.duration:
                break
            log_line = format_log_line(line_index, start_time)
            if args.full_frame:
                buffer.append(log_line)
                trimmed = trim_buffer_to_fit(buffer, args.max_bytes)
                if trimmed and not warned_trim:
                    print("Trimmed payload to fit max bytes; reduce --lines or --max-bytes for full history.")
                    warned_trim = True
                payload = payload_bytes("\n".join(buffer))
                write_payload(ser, payload, args.chunk_size, args.chunk_delay)
            else:
                payload = build_append_payload(log_line + "\n", final=True).encode("utf-8")
                write_payload(ser, payload, args.chunk_size, args.chunk_delay)
            line_index += 1
            sleep_for = max(0.0, next_tick + args.interval - time.time())
            time.sleep(sleep_for)

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
