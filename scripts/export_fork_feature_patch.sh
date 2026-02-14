#!/usr/bin/env bash
set -euo pipefail

usage() {
  cat <<'USAGE'
Usage:
  scripts/export_fork_feature_patch.sh list
  scripts/export_fork_feature_patch.sh <feature_id> [fork_ref] [upstream_ref]
  scripts/export_fork_feature_patch.sh all [fork_ref] [upstream_ref] [output_dir]

Defaults:
  fork_ref     = master
  upstream_ref = daveallie/master
  output_dir   = /tmp/fork-feature-patches

Feature IDs:
  infoboard_text_board
  pomodoro
  wifi_file_transfer_autoconnect
  external_reader_fonts
  math_latex_cjk_pipeline
  docs_and_repo_meta
  local_skill_scaffold
  binary_or_scratch_artifacts
  history_merge_commits
USAGE
}

feature_paths() {
  local feature_id="$1"
  case "$feature_id" in
    infoboard_text_board)
      cat <<'PATHS'
src/activities/infoboard
src/activities/home/HomeActivity.cpp
src/activities/home/HomeActivity.h
src/main.cpp
scripts/send_infoboard.py
scripts/rolling_console_demo.py
scripts/infoboard_demo.py
scripts/infoboard_btop_demo.py
scripts/infoboard_tweet_demo.py
scripts/gradio_infoboard_demo.py
docs/infoboard_console_notes.md
PATHS
      ;;
    pomodoro)
      cat <<'PATHS'
src/activities/pomodoro
src/activities/home/HomeActivity.cpp
src/activities/home/HomeActivity.h
src/main.cpp
src/CrossPointState.cpp
src/CrossPointState.h
platformio.ini
test/test_pomodoro_model/test_main.cpp
docs_for_llm/PLAN_pomodoro.md
docs_for_llm/SPEC_initial_pomodoro.md
PATHS
      ;;
    wifi_file_transfer_autoconnect)
      cat <<'PATHS'
src/activities/network/CrossPointWebServerActivity.cpp
src/activities/network/CrossPointWebServerActivity.h
src/activities/network/WifiSelectionActivity.cpp
src/CrossPointState.cpp
src/CrossPointState.h
PATHS
      ;;
    external_reader_fonts)
      cat <<'PATHS'
lib/ExternalFont
scripts/generate_external_font.py
src/activities/settings/FontSelectActivity.cpp
src/activities/settings/FontSelectActivity.h
src/activities/settings/SettingsActivity.cpp
lib/GfxRenderer/GfxRenderer.cpp
lib/GfxRenderer/GfxRenderer.h
lib/Epub/Epub/Section.cpp
lib/Epub/Epub/Section.h
src/activities/reader/EpubReaderActivity.cpp
src/activities/reader/TxtReaderActivity.cpp
src/CrossPointSettings.cpp
src/CrossPointSettings.h
src/fontIds.h
src/main.cpp
fonts/Bookerly_20_20x24.bin
README.md
PATHS
      ;;
    math_latex_cjk_pipeline)
      cat <<'PATHS'
lib/Epub/Epub/parsers/ChapterHtmlSlimParser.cpp
lib/EpdFont/scripts/convert-builtin-fonts.sh
lib/EpdFont/scripts/fontconvert.py
:(glob)lib/EpdFont/builtinFonts/notosans_*.h
lib/EpdFont/builtinFonts/source/NotoSansMath/NotoSansMath-Regular.ttf
scripts/html_to_epub_mathfix.py
scripts/html_to_epub_mathfix.ps1
scripts/latex_to_epub_noimg.py
docs/HTML_MATH_EPUB.md
docs/ARXIV_LATEX_TO_EPUB.md
list-fix.css
PATHS
      ;;
    docs_and_repo_meta)
      cat <<'PATHS'
README.md
docs/UPLOAD.md
docs/TESTING.md
AGENTS.md
AGENTS.epub
PATHS
      ;;
    local_skill_scaffold)
      cat <<'PATHS'
skills/x4-file-handling
PATHS
      ;;
    binary_or_scratch_artifacts)
      cat <<'PATHS'
_custom_firmware
:(glob)*.epub
:(glob)*.html
edit
PATHS
      ;;
    history_merge_commits)
      cat <<'PATHS'
# merge-only commits have no standalone path payload
PATHS
      ;;
    *)
      return 1
      ;;
  esac
}

list_features() {
  cat <<'LIST'
infoboard_text_board
pomodoro
wifi_file_transfer_autoconnect
external_reader_fonts
math_latex_cjk_pipeline
docs_and_repo_meta
local_skill_scaffold
binary_or_scratch_artifacts
history_merge_commits
LIST
}

if [[ "${1:-}" == "" ]] || [[ "${1:-}" == "-h" ]] || [[ "${1:-}" == "--help" ]]; then
  usage
  exit 0
fi

cmd="$1"
fork_ref="${2:-master}"
upstream_ref="${3:-daveallie/master}"

if [[ "$cmd" == "list" ]]; then
  list_features
  exit 0
fi

if [[ "$cmd" == "all" ]]; then
  out_dir="${4:-/tmp/fork-feature-patches}"
  mkdir -p "$out_dir"
  while IFS= read -r feature_id; do
    if [[ "$feature_id" == "history_merge_commits" ]]; then
      echo "skipped $feature_id (no standalone path payload)"
      continue
    fi
    mapfile -t paths < <(feature_paths "$feature_id")
    base_ref=$(git merge-base "$fork_ref" "$upstream_ref")
    out_file="$out_dir/${feature_id}.patch"
    git diff --binary "$base_ref..$fork_ref" -- "${paths[@]}" > "$out_file"
    echo "wrote $out_file"
  done < <(list_features)
  exit 0
fi

feature_id="$cmd"
if ! mapfile -t paths < <(feature_paths "$feature_id"); then
  echo "Unknown feature_id: $feature_id" >&2
  usage
  exit 1
fi

if [[ "$feature_id" == "history_merge_commits" ]]; then
  cat <<'MSG' >&2
history_merge_commits has no standalone path payload.
Use commit-map.csv entries directly if you need to inspect or replay merge history.
MSG
  exit 0
fi

base_ref=$(git merge-base "$fork_ref" "$upstream_ref")
git diff --binary "$base_ref..$fork_ref" -- "${paths[@]}"
