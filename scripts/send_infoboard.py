#!/usr/bin/env python3
import argparse
import sys
import time

try:
    import serial
except ImportError:  # pragma: no cover - runtime dependency check
    sys.exit("pyserial is required. Install with: pip install pyserial")


def main() -> int:
    parser = argparse.ArgumentParser(description="Send a single InfoBoard line over USB serial.")
    parser.add_argument("port", help="Serial port (e.g. /dev/ttyACM0 or COM3)")
    parser.add_argument("message", nargs="?", help="Message to send. If omitted, read one line from stdin.")
    parser.add_argument("--baud", type=int, default=115200, help="Baud rate (default: 115200)")
    parser.add_argument("--delay", type=float, default=0.5, help="Seconds to wait after opening port")
    args = parser.parse_args()

    message = args.message
    if message is None:
        message = sys.stdin.readline().rstrip("\n")

    with serial.Serial(args.port, args.baud, timeout=1) as ser:
        if args.delay > 0:
            time.sleep(args.delay)
        payload = (message + "\n").encode("utf-8")
        ser.write(payload)
        ser.flush()

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
