# Repository Guidelines

## Project Structure & Module Organization
- `src/` holds the firmware implementation (C++/Arduino), with feature areas in subfolders like `src/activities/`, `src/network/`, and `src/util/`.
- `include/` contains shared headers, and `lib/` contains local libraries used by PlatformIO.
- `open-x4-sdk/` is a git submodule providing device and display support libraries.
- `docs/` and `USER_GUIDE.md` contain project documentation; images live in `docs/images/` and `src/images/`.
- `scripts/` contains build helpers such as `scripts/build_html.py` (invoked by PlatformIO).
- `test/` is reserved for PlatformIO unit tests (currently empty).

## Build, Test, and Development Commands
- `git submodule update --init --recursive` to fetch `open-x4-sdk` after cloning.
- `pio run` to build the default firmware environment.
- `pio run --target upload` to flash the ESP32-C3 device over USB (see README for setup).
- `pio check` to run static analysis (cppcheck is configured in `platformio.ini`).
- `pio test` to run PlatformIO tests if/when tests are added.

## Coding Style & Naming Conventions
- Use the repository `.clang-format` (2-space indentation, C++ style). Run `bin/clang-format-fix` or `bin/clang-format-fix -g` to format modified files.
- Match existing naming patterns: `PascalCase` for class headers/implementations (e.g., `CrossPointSettings.h/.cpp`), and lower-case folder names (e.g., `src/network/`).
- Keep platform-specific constants and settings in `platformio.ini` and `partitions.csv` rather than hard-coding.

## Testing Guidelines
- Follow PlatformIO’s test layout: create suite folders under `test/` and keep firmware-facing tests small and deterministic.
- Include any required hardware assumptions in the test README or PR description.
- There is no explicit coverage target yet; focus on critical parsing, rendering, and storage logic.

## Commit & Pull Request Guidelines
- Commit messages generally follow a `type: summary` format (e.g., `feat:`, `fix:`, `docs:`, `chore:`), often with a PR reference like `(#123)`.
- For PRs, include a clear summary, what was tested (`pio run`, `pio test`, or manual device checks), and any hardware/UX impact. Add screenshots for UI or web-server changes when applicable.

## Security & Configuration Notes
- Do not commit device-specific secrets or WiFi credentials. Prefer config files or runtime setup steps documented in `USER_GUIDE.md`.
