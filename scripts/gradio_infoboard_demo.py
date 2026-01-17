#!/usr/bin/env python3
import json
import sys
import time

try:
    import gradio as gr
except ImportError:  # pragma: no cover - runtime dependency check
    sys.exit("gradio is required. Install with: pip install gradio")

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


def refresh_ports() -> dict:
    ports = detect_ports()
    default_port = pick_default_port(ports)
    return gr.update(choices=ports, value=default_port)


def send_message(port: str, mode: str, message: str) -> str:
    if not message or not message.strip():
        return "Message is empty."

    ports = detect_ports()
    port = port or pick_default_port(ports)
    if not port:
        return "No serial ports detected. Click Refresh Ports."

    payload_message = message
    if mode == "console":
        payload_message = json.dumps({"mode": "console", "text": message})

    try:
        with serial.Serial(port, 115200, timeout=1) as ser:
            time.sleep(0.5)
            payload = (payload_message + "\n").encode("utf-8")
            ser.write(payload)
            ser.flush()
    except Exception as exc:  # pragma: no cover - runtime serial errors
        return f"Send failed: {exc}"

    return f"Sent to {port} ({mode})."


def build_demo() -> gr.Blocks:
    ports = detect_ports()
    default_port = pick_default_port(ports)

    with gr.Blocks(title="InfoBoard Sender") as demo:
        gr.Markdown("Simple InfoBoard sender (USB serial).")
        with gr.Row():
            port = gr.Dropdown(
                label="Serial port",
                choices=ports,
                value=default_port,
                allow_custom_value=True,
            )
            refresh = gr.Button("Refresh Ports")
        mode = gr.Radio(["normal", "console"], value="normal", label="Mode")
        message = gr.Textbox(
            label="Message",
            lines=4,
            placeholder="Type message to send...",
        )
        send = gr.Button("Send")
        status = gr.Textbox(label="Status", interactive=False)

        refresh.click(fn=refresh_ports, outputs=port)
        send.click(fn=send_message, inputs=[port, mode, message], outputs=status)

    return demo


if __name__ == "__main__":
    build_demo().launch()
