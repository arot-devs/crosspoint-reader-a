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
    parser.add_argument("--lines", type=int, default=20, help="Lines to keep in view (default: 20)")
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
        buffer = deque([""] * args.lines, maxlen=args.lines)
        start_time = time.time()
        line_index = 0
        while True:
            next_tick = start_time + (line_index * args.interval)
            if next_tick - start_time >= args.duration:
                break
            log_line = format_log_line(line_index, start_time)
            buffer.append(log_line)
            payload = build_payload("\n".join(buffer)) + "\n"
            ser.write(payload.encode("utf-8"))
            ser.flush()
            line_index += 1
            sleep_for = max(0.0, next_tick + args.interval - time.time())
            time.sleep(sleep_for)

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
