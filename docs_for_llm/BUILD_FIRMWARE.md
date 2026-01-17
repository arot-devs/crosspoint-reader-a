# Build Firmware (Linux)

This repo builds an ESP32‑C3 Arduino firmware using PlatformIO.

## What to install

Install PlatformIO (Python 3 required):

```bash
python3 -m pip install --user platformio
```

Optional: add user-local binaries to PATH (so you can run `platformio` without a full path):

```bash
export PATH="$HOME/.local/bin:$PATH"
```

## What is already installed on this machine

- PlatformIO installed via `pip --user`
- Binary location: `/home/ubuntu/.local/bin/platformio`

## Build command (from repo root)

```bash
/home/ubuntu/.local/bin/platformio run
```

PlatformIO will auto-download the ESP32‑C3 toolchain/SDK on first build.

## Output file

The built app image is:

```
.pio/build/default/firmware.bin
```

This is the correct format for OTA fast‑flash (app partition image).

## Where to put the built file for xteink-flasher

Copy the app image to the custom firmware folder. Use a descriptive filename that matches the feature branch or build (for InfoBoard builds, use `crosspoint_infoboard.bin`):

```bash
cp .pio/build/default/firmware.bin \
  /home/ubuntu/dev/crosspoint-reader/_custom_firmware/crosspoint_infoboard.bin
```

In xteink‑flasher, use “OTA fast flash controls → Flash firmware from file” and select that file.
