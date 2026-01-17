#!/usr/bin/env python3
import argparse
import json
import sys
import time

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


def main() -> int:
    parser = argparse.ArgumentParser(description="Send a single InfoBoard line over USB serial.")
    parser.add_argument("port", nargs="?", help="Serial port (e.g. /dev/ttyACM0 or COM3)")
    parser.add_argument("message", nargs="?", help="Message to send. If omitted, read one line from stdin.")
    parser.add_argument("--port", dest="port_override", help="Serial port (overrides positional port)")
    parser.add_argument("--message", dest="message_override", help="Message to send (overrides positional message)")
    parser.add_argument("--baud", type=int, default=115200, help="Baud rate (default: 115200)")
    parser.add_argument("--delay", type=float, default=0.5, help="Seconds to wait after opening port")
    parser.add_argument("--list", action="store_true", help="List available serial ports and exit")
    parser.add_argument("--console", action="store_true", help="Send message as InfoBoard console JSON payload")
    parser.add_argument("--append", action="store_true", help="Append text to console buffer (JSON)")
    parser.add_argument("--final", action="store_true", help="Render immediately when using JSON append/clear")
    parser.add_argument("--clear", action="store_true", help="Clear console buffer before applying text (JSON)")
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

    ports = detect_ports()
    port = args.port_override or args.port
    message = args.message_override or args.message

    if port and message is None:
        # Single positional: treat as message if it doesn't match a detected port.
        if port not in ports:
            message = port
            port = None

    if not port:
        port = pick_default_port(ports)
        if not port:
            if not ports:
                sys.exit("No serial ports detected. Use --list to view ports.")
            sys.exit("Multiple serial ports detected. Pass one explicitly or use --list.")

    if message is None and args.clear:
        message = ""
    elif message is None:
        message = sys.stdin.readline().rstrip("\n")

    payload_message = message
    if args.console or args.append or args.clear or args.final:
        payload: dict[str, object] = {"mode": "console", "text": message}
        if args.append:
            payload["append"] = True
        if args.clear:
            payload["clear"] = True
        if args.final:
            payload["final"] = True
        payload_message = json.dumps(payload)

    with serial.Serial(port, args.baud, timeout=1) as ser:
        if args.delay > 0:
            time.sleep(args.delay)
        payload = (payload_message + "\n").encode("utf-8")
        ser.write(payload)
        ser.flush()

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
