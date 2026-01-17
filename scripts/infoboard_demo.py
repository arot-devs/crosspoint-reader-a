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


SCENARIOS = [
    {
        "name": "Centered title",
        "mode": "default",
        "text": "InfoBoard Demo\nCentered text\n(press Enter for next)",
    },
    {
        "name": "Console boxed",
        "mode": "console",
        "text": "+----------------------+
|  ASCII CHART DEMO    |
+----------------------+
| #######   ######     |
| #######   ######     |
| #######   ######     |
+----------------------+",
    },
    {
        "name": "Console bar chart",
        "mode": "console",
        "text": "CPU  [##########      ] 50%\nRAM  [##############  ] 70%\nDISK [#####           ] 25%",
    },
    {
        "name": "Wrapped paragraph",
        "mode": "default",
        "text": "This is a longer message that should wrap nicely in the default centered mode. Useful for summaries or alerts.",
    },
    {
        "name": "Console log lines",
        "mode": "console",
        "text": "[12:00:00] Boot OK\n[12:00:05] Sync started\n[12:00:12] Sync complete",
    },
]


def build_payload(text: str, mode: str) -> str:
    if mode == "console":
        return json.dumps({"mode": "console", "text": text})
    return text


def main() -> int:
    parser = argparse.ArgumentParser(description="Send a sequence of InfoBoard demo messages.")
    parser.add_argument("port", nargs="?", help="Serial port (e.g. /dev/ttyACM0 or COM3)")
    parser.add_argument("--port", dest="port_override", help="Serial port (overrides positional port)")
    parser.add_argument("--baud", type=int, default=115200, help="Baud rate (default: 115200)")
    parser.add_argument("--delay", type=float, default=0.5, help="Seconds to wait after opening port")
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

    ports = detect_ports()
    port = args.port_override or args.port
    if not port:
        port = pick_default_port(ports)
        if not port:
            if not ports:
                sys.exit("No serial ports detected. Use --list to view ports.")
            sys.exit("Multiple serial ports detected. Pass one explicitly or use --list.")

    print("InfoBoard demo: press Enter to send next scenario; Ctrl+C to quit.")
    with serial.Serial(port, args.baud, timeout=1) as ser:
        if args.delay > 0:
            time.sleep(args.delay)
        for scenario in SCENARIOS:
            input(f"\n[{scenario['name']}] Press Enter to send...")
            payload = build_payload(scenario["text"], scenario["mode"]) + "\n"
            ser.write(payload.encode("utf-8"))
            ser.flush()

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
