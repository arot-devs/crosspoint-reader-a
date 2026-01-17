#!/usr/bin/env python3
import argparse
import json
import sys
import time

try:
    import psutil
except ImportError:  # pragma: no cover - runtime dependency check
    sys.exit("psutil is required. Install with: pip install psutil")

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


def format_gb(value_bytes: float) -> str:
    gb = value_bytes / (1024**3)
    return f"{gb:.1f}G"


def bar(percent: float, width: int = 16) -> str:
    percent = max(0.0, min(100.0, percent))
    filled = int(round(percent / 100.0 * width))
    filled = max(0, min(width, filled))
    return "#" * filled + "-" * (width - filled)


def safe_name(name: str, max_len: int) -> str:
    if len(name) <= max_len:
        return name
    if max_len <= 3:
        return name[:max_len]
    return name[: max_len - 3] + "..."

def build_console_payload(text: str, clear: bool = False, final: bool = False) -> str:
    payload: dict[str, object] = {"mode": "console", "append": True, "text": text}
    if clear:
        payload["clear"] = True
    if final:
        payload["final"] = True
    return json.dumps(payload)


def get_process_stats(top_n: int) -> list[dict[str, object]]:
    def percent_or_zero(value: object | None) -> float:
        if value is None:
            return 0.0
        try:
            return float(value)
        except (TypeError, ValueError):
            return 0.0

    processes = []
    for proc in psutil.process_iter(["pid", "name", "cpu_percent", "memory_percent"]):
        try:
            info = proc.info
        except (psutil.NoSuchProcess, psutil.AccessDenied):
            continue
        processes.append(info)

    processes.sort(
        key=lambda p: percent_or_zero(p.get("cpu_percent")),
        reverse=True,
    )
    return processes[:top_n]


def render_screen(top_n: int) -> str:
    now = time.strftime("%H:%M:%S")
    cpu = psutil.cpu_percent(interval=None)
    mem = psutil.virtual_memory()
    swap = psutil.swap_memory()
    try:
        disk = psutil.disk_usage("/")
    except Exception:
        disk = None

    lines: list[str] = []
    lines.append(f"InfoBoard Top  {now}")
    lines.append(f"CPU {cpu:5.1f}% [{bar(cpu)}]")
    lines.append(
        f"RAM {mem.percent:5.1f}% {format_gb(mem.used)}/{format_gb(mem.total)} [{bar(mem.percent)}]"
    )
    if swap.total > 0:
        lines.append(
            f"SWP {swap.percent:5.1f}% {format_gb(swap.used)}/{format_gb(swap.total)} [{bar(swap.percent)}]"
        )
    if disk is not None:
        lines.append(
            f"DSK {disk.percent:5.1f}% {format_gb(disk.used)}/{format_gb(disk.total)} [{bar(disk.percent)}]"
        )
    lines.append("")
    lines.append("Top CPU (pid name cpu mem)")

    for proc in get_process_stats(top_n):
        pid = proc.get("pid", 0)
        name = proc.get("name") or "?"
        name = safe_name(name, 16)
        cpu_pct = proc.get("cpu_percent", 0.0) or 0.0
        mem_pct = proc.get("memory_percent", 0.0) or 0.0
        lines.append(f"{pid:>5} {name:<16} {cpu_pct:>5.1f}% {mem_pct:>4.1f}%")

    return "\n".join(lines)


def warm_process_counters() -> None:
    psutil.cpu_percent(interval=None)
    for proc in psutil.process_iter(["cpu_percent"]):
        try:
            proc.cpu_percent(interval=None)
        except (psutil.NoSuchProcess, psutil.AccessDenied):
            continue


def main() -> int:
    parser = argparse.ArgumentParser(description="Render a btop-style console view on InfoBoard.")
    parser.add_argument("port", nargs="?", help="Serial port (e.g. /dev/ttyACM0 or COM3)")
    parser.add_argument("--port", dest="port_override", help="Serial port (overrides positional port)")
    parser.add_argument("--baud", type=int, default=115200, help="Baud rate (default: 115200)")
    parser.add_argument("--delay", type=float, default=0.5, help="Seconds to wait after opening port")
    parser.add_argument("--interval", type=float, default=1.0, help="Seconds between updates (default: 1.0)")
    parser.add_argument("--duration", type=float, default=0, help="Seconds to run (0 = forever)")
    parser.add_argument("--top", type=int, default=5, help="Number of top processes to show (default: 5)")
    parser.add_argument(
        "--batch-delay",
        type=float,
        default=0.01,
        help="Seconds to wait between line writes (default: 0.01)",
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

    if args.interval <= 0:
        sys.exit("Interval must be > 0 seconds.")
    if args.top <= 0:
        sys.exit("Top must be > 0.")
    if args.duration < 0:
        sys.exit("Duration must be >= 0.")
    if args.batch_delay < 0:
        sys.exit("Batch delay must be >= 0.")

    ports = detect_ports()
    port = args.port_override or args.port
    if not port:
        port = pick_default_port(ports)
        if not port:
            if not ports:
                sys.exit("No serial ports detected. Use --list to view ports.")
            sys.exit("Multiple serial ports detected. Pass one explicitly or use --list.")

    warm_process_counters()
    start_time = time.time()
    with serial.Serial(port, args.baud, timeout=1) as ser:
        if args.delay > 0:
            time.sleep(args.delay)
        while True:
            screen = render_screen(args.top)
            lines = screen.split("\n")
            for idx, line in enumerate(lines):
                payload = build_console_payload(
                    line + "\n",
                    clear=(idx == 0),
                    final=(idx == len(lines) - 1),
                )
                ser.write((payload + "\n").encode("utf-8"))
                if args.batch_delay > 0:
                    time.sleep(args.batch_delay)
            ser.flush()
            if args.duration and (time.time() - start_time) >= args.duration:
                break
            time.sleep(args.interval)

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
