#!/usr/bin/env python3
import argparse
import json
import select
import sys
import time
import textwrap

try:
    import serial
    from serial.tools import list_ports
except ImportError:  # pragma: no cover - runtime dependency check
    sys.exit("pyserial is required. Install with: pip install pyserial")


TWEETS = [
    {
        "name": "Sam Altman",
        "handle": "@sama",
        "text": "Very fast Codex coming!",
    },
    {
        "name": "FPuget (UA/CA/GL)",
        "handle": "@JFPuget",
        "text": (
            "MMLU target leakage is dramatic. After @fujikanaeda who found that the presence of a leading white "
            "space was predictive in some cases, @peterbarnett_ finds that the length of proposed answers is predictive: "
            "the longest answer is often the correct answer.\n\n"
            "This information is used by LLMs. Each time there was a target leakage like this in @kaggle competitions, "
            "the LLMs used it.\n\n"
            "TL;DR MMLU results are not reliable at all to estimate how LLMs perform on similar data. Maybe it is "
            "reliable to rank LLMs, but even that is questionable."
        ),
    },
    {
        "name": "Peter Barnett",
        "handle": "@peterbarnett_",
        "text": (
            "It's even worse. You get a similar boost across the whole benchmark (21% vs 10% for random guessing) "
            "if just always guess the longest answer."
        ),
    },
    {
        "name": "Eric W. Tramel",
        "handle": "@fujikanaeda",
        "text": (
            "The presence of a leading whitespace leaks the correct choice selection in the MMLU-Pro benchmark. "
            "Am I missing something? Seems to impact Chemistry, Physics, and Math.\n\n"
            "HF issue in reply."
        ),
    },
    {
        "name": "Amjad Masad",
        "handle": "@amasad",
        "text": "Fascinating that Greg is doing o-1 style reasoning chains in his notebooks",
    },
    {
        "name": "vik",
        "handle": "@vikhyatk",
        "text": (
            "teaching computers how to see @moondreamai\n\n"
            "what actually happens when you train on your internal data:"
        ),
    },
]


NEXT_BUTTONS = {"down", "right", "page_forward", "confirm"}
PREV_BUTTONS = {"up", "left", "page_back"}


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


def build_payload(line: str, clear: bool, final: bool) -> str:
    payload: dict[str, object] = {"mode": "console", "append": True, "text": line}
    if clear:
        payload["clear"] = True
    if final:
        payload["final"] = True
    return json.dumps(payload)


def wrap_text(text: str, width: int) -> list[str]:
    lines: list[str] = []
    paragraphs = text.split("\n\n")
    for para in paragraphs:
        para_lines = textwrap.wrap(
            para,
            width=width,
            break_long_words=True,
            replace_whitespace=False,
        )
        if not para_lines:
            lines.append("")
        else:
            lines.extend(para_lines)
        lines.append("")
    if lines and lines[-1] == "":
        lines.pop()
    return lines


def render_tweet(tweet: dict[str, str], index: int, total: int, width: int) -> list[str]:
    header = f"{tweet['handle']} - {tweet['name']}"
    lines = [header, "-" * min(width, max(10, len(header)))]
    lines.extend(wrap_text(tweet["text"], width))
    lines.append("")
    lines.append(f"Tweet {index + 1}/{total} | Up/Down to navigate | Back to exit")
    return lines


def send_tweet(ser: serial.Serial, tweet: dict[str, str], index: int, total: int, width: int, batch_delay: float) -> None:
    lines = render_tweet(tweet, index, total, width)
    for idx, line in enumerate(lines):
        payload = build_payload(line + "\n", clear=(idx == 0), final=(idx == len(lines) - 1))
        ser.write((payload + "\n").encode("utf-8"))
        if batch_delay > 0:
            time.sleep(batch_delay)
    ser.flush()


def read_button_event(line: str) -> str | None:
    line = line.strip()
    if not line.startswith("IBEV "):
        return None
    try:
        data = json.loads(line[5:])
    except json.JSONDecodeError:
        return None
    if data.get("event") != "button" or data.get("action") != "release":
        return None
    return str(data.get("button"))


def read_stdin_command() -> str | None:
    if not select.select([sys.stdin], [], [], 0.0)[0]:
        return None
    return sys.stdin.readline().strip().lower()


def main() -> int:
    parser = argparse.ArgumentParser(description="InfoBoard tweet viewer demo (serial + button events).")
    parser.add_argument("port", nargs="?", help="Serial port (e.g. /dev/ttyACM0 or COM3)")
    parser.add_argument("--port", dest="port_override", help="Serial port (overrides positional port)")
    parser.add_argument("--baud", type=int, default=115200, help="Baud rate (default: 115200)")
    parser.add_argument("--delay", type=float, default=0.5, help="Seconds to wait after opening port")
    parser.add_argument("--wrap", type=int, default=62, help="Text wrap width (default: 62)")
    parser.add_argument("--batch-delay", type=float, default=0.01, help="Delay between line sends (default: 0.01)")
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

    if args.wrap <= 20:
        sys.exit("Wrap width too small; use a value > 20.")

    ports = detect_ports()
    port = args.port_override or args.port
    if not port:
        port = pick_default_port(ports)
        if not port:
            if not ports:
                sys.exit("No serial ports detected. Use --list to view ports.")
            sys.exit("Multiple serial ports detected. Pass one explicitly or use --list.")

    total = len(TWEETS)
    index = 0

    print("InfoBoard tweet demo")
    print("- Use device buttons (Up/Down/Page/Confirm) or type n/p/q + Enter.")
    print("- Listening for IBEV button events on serial.")

    with serial.Serial(port, args.baud, timeout=0.1) as ser:
        if args.delay > 0:
            time.sleep(args.delay)
        send_tweet(ser, TWEETS[index], index, total, args.wrap, args.batch_delay)

        while True:
            line_bytes = ser.readline()
            if line_bytes:
                try:
                    line = line_bytes.decode("utf-8", errors="ignore")
                except UnicodeDecodeError:
                    line = ""
                button = read_button_event(line)
                if button == "back":
                    print("Back pressed; exiting.")
                    break
                if button in NEXT_BUTTONS:
                    index = (index + 1) % total
                    send_tweet(ser, TWEETS[index], index, total, args.wrap, args.batch_delay)
                elif button in PREV_BUTTONS:
                    index = (index - 1 + total) % total
                    send_tweet(ser, TWEETS[index], index, total, args.wrap, args.batch_delay)

            cmd = read_stdin_command()
            if cmd in {"q", "quit", "exit"}:
                break
            if cmd in {"n", "next", "j"}:
                index = (index + 1) % total
                send_tweet(ser, TWEETS[index], index, total, args.wrap, args.batch_delay)
            elif cmd in {"p", "prev", "k"}:
                index = (index - 1 + total) % total
                send_tweet(ser, TWEETS[index], index, total, args.wrap, args.batch_delay)

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
