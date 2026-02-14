# Fork Feature Salvage Map

Purpose: keep the current fork's `44` ahead commits organized as re-implementable feature buckets before syncing to upstream.

Baseline:
- Fork head: `6afa3df` (2026-01-18)
- Upstream head checked: `5816ab2` (2026-02-14)
- Merge base: `21277e0` (2026-01-15)
- Divergence at time of mapping: `+44 / -140`

## Feature Buckets

### `infoboard_text_board`
Summary: USB-serial "InfoBoard" activity + console-style text board + host demos/scripts.
Primary commits:
- `c754a59`
- `9c388a3`
- `4c7a1e6`
- `e6bbac4`
- `9dd81ea`
- `807c89c`
- `9b39ea2`
- `5d62d07`
- `1742709`
- `045e600`
- `9930af6`
- `6078657`
Main scope:
- `src/activities/infoboard/`
- `scripts/send_infoboard.py`
- `scripts/rolling_console_demo.py`
- `scripts/infoboard_demo.py`
- `scripts/infoboard_btop_demo.py`
- `scripts/infoboard_tweet_demo.py`
- `scripts/gradio_infoboard_demo.py`
- `docs/infoboard_console_notes.md`
- integration: `src/activities/home/HomeActivity.cpp`, `src/main.cpp`

### `pomodoro`
Summary: Pomodoro mode activity/model, persistence, UI integration, native model tests.
Primary commits:
- `fd2c348`
- `fe80777`
- `bd81cca`
- `7e585d4`
- `1e8b244`
- `a927215`
Main scope:
- `src/activities/pomodoro/`
- `test/test_pomodoro_model/test_main.cpp`
- `platformio.ini` (`[env:native]` additions)
- integration: `src/activities/home/HomeActivity.cpp`, `src/main.cpp`, `src/CrossPointState.cpp`

### `wifi_file_transfer_autoconnect`
Summary: File-transfer activity auto-connects to saved WiFi and remembers last SSID.
Primary commits:
- `1da8939`
Main scope:
- `src/activities/network/CrossPointWebServerActivity.cpp`
- `src/activities/network/WifiSelectionActivity.cpp`
- `src/CrossPointState.cpp`

### `external_reader_fonts`
Summary: External SD-card reader font pipeline and runtime rendering path.
Primary commits:
- `06c0f2c`
- `806f93a`
- `6afa3df`
Main scope:
- `lib/ExternalFont/`
- `scripts/generate_external_font.py`
- `src/activities/settings/FontSelectActivity.cpp`
- renderer/reader integration:
  - `lib/GfxRenderer/GfxRenderer.cpp`
  - `lib/Epub/Epub/Section.cpp`
  - `src/activities/reader/EpubReaderActivity.cpp`
  - `src/activities/reader/TxtReaderActivity.cpp`
  - `src/main.cpp`
  - `src/activities/settings/SettingsActivity.cpp`

### `math_latex_cjk_pipeline`
Summary: Math glyph coverage expansion + CJK tokenization tweak + HTML/LaTeX-to-EPUB tooling/docs.
Primary commits:
- `8beec79`
- `05a8fc2`
- `91cd571`
- `f08b753`
- `9daba1d`
- `de39649`
- `404790c`
Main scope:
- `lib/Epub/Epub/parsers/ChapterHtmlSlimParser.cpp`
- `lib/EpdFont/scripts/convert-builtin-fonts.sh`
- `lib/EpdFont/scripts/fontconvert.py`
- `lib/EpdFont/builtinFonts/notosans_*.h`
- `lib/EpdFont/builtinFonts/source/NotoSansMath/NotoSansMath-Regular.ttf`
- `scripts/html_to_epub_mathfix.py`
- `scripts/html_to_epub_mathfix.ps1`
- `scripts/latex_to_epub_noimg.py`
- `docs/HTML_MATH_EPUB.md`
- `docs/ARXIV_LATEX_TO_EPUB.md`

### `docs_and_repo_meta`
Summary: README/docs/meta updates not required for firmware behavior.
Primary commits:
- `4dd4aac`
- `3f7080f`
- `01e7407`
- `ba433fc`
- `9edf4ad`
Main scope:
- `README.md`
- `docs/UPLOAD.md`
- `docs/TESTING.md`
- `AGENTS.md`
- `AGENTS.epub`

### `local_skill_scaffold`
Summary: local skill reference content (not firmware runtime).
Primary commits:
- `afe8142`
Main scope:
- `skills/x4-file-handling/`

### `binary_or_scratch_artifacts`
Summary: generated binaries and scratch EPUB/HTML assets (not upstream PR targets).
Primary commits:
- `2982f5d`
- `446a1fb`
- `d3735ce`
- `3cf4dc8`
Main scope:
- `_custom_firmware/*.bin`
- generated sample/test `.epub` / `.html` files under repo root

### `history_merge_commits`
Summary: merge-only history commits from syncing/PR integration in the fork.
Primary commits:
- `6ebd344`
- `d1b7131`
- `fb901c4`
- `8d9a200`
- `9238963`
Main scope:
- no standalone feature payload; these are history/integration markers

## How to Use This Map

1. Keep a safety branch before syncing upstream:
   - `git branch backup/pre-upstream-sync master`
2. Move `master` to upstream when ready (not done here).
3. Export a feature patch with:
   - `scripts/export_fork_feature_patch.sh <feature_id> [fork_ref] [upstream_ref] > /tmp/<feature_id>.patch`
4. Re-apply selectively onto updated `master`:
   - `git apply --3way /tmp/<feature_id>.patch`

See `docs/fork-salvage/commit-map.csv` for the per-commit feature assignment.
