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


LOG_TEMPLATES = [
    ("INFO", "Booting subsystem {n}"),
    ("INFO", "WiFi link up, RSSI={rssi}dBm"),
    ("INFO", "Time sync OK (drift {drift}ms)"),
    ("WARN", "Packet retry {retry}/3"),
    ("INFO", "Render queue size={qsize}"),
    ("INFO", "Sensor temp={temp}C"),
    ("INFO", "Heap free={heap}KB"),
    ("WARN", "Slow frame draw {ms}ms"),
    ("INFO", "MQTT publish id={msgid}"),
    ("INFO", "Idle until next update"),
]

MAX_PAYLOAD_BYTES = 2048


def format_log_line(counter: int, start_time: float) -> str:
    timestamp = time.strftime("%H:%M:%S", time.localtime(start_time + counter))
    level, template = LOG_TEMPLATES[counter % len(LOG_TEMPLATES)]
    return (
        f"[{timestamp}] {level:<5} "
        + template.format(
            n=(counter % 5) + 1,
            rssi=-58 - (counter % 5),
            drift=4 + (counter % 7),
            retry=(counter % 3) + 1,
            qsize=2 + (counter % 8),
            temp=23 + (counter % 6),
            heap=412 - (counter % 15) * 3,
            ms=120 + (counter % 40),
            msgid=1000 + counter,
        )
    )


def build_payload(text: str) -> str:
    return json.dumps({"mode": "console", "text": text})


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
        help="Lines to keep in view (match display capacity; default: 20)",
    )
    parser.add_argument(
        "--max-bytes",
        type=int,
        default=MAX_PAYLOAD_BYTES,
        help=f"Max JSON payload size before trimming (default: {MAX_PAYLOAD_BYTES})",
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

    print(
        "Rolling console demo: "
        f"{args.lines} lines, {args.interval:.2f}s/line for {args.duration:.1f}s."
    )
    print("Press Ctrl+C to stop.")
    with serial.Serial(port, args.baud, timeout=1) as ser:
        if args.delay > 0:
            time.sleep(args.delay)
        buffer = deque(maxlen=args.lines)
        start_time = time.time()
        line_index = 0
        warned_trim = False
        while True:
            next_tick = start_time + (line_index * args.interval)
            if next_tick - start_time >= args.duration:
                break
            log_line = format_log_line(line_index, start_time)
            buffer.append(log_line)
            trimmed = trim_buffer_to_fit(buffer, args.max_bytes)
            if trimmed and not warned_trim:
                print("Trimmed payload to fit max bytes; reduce --lines or --max-bytes for full history.")
                warned_trim = True
            payload = payload_bytes("\n".join(buffer))
            write_payload(ser, payload, args.chunk_size, args.chunk_delay)
            line_index += 1
            sleep_for = max(0.0, next_tick + args.interval - time.time())
            time.sleep(sleep_for)

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
