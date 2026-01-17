# Pomodoro Activity Implementation Plan


## Purpose / Big Picture


This plan defines a firmware implementation for a calm, single-purpose Pomodoro timer on the 4.3 inch e-ink device with four buttons. Users will be able to launch Pomodoro from the home screen next to InfoBoard, set a 5 to 60 minute duration in 5 minute steps, start and pause a session, and see remaining time as a Time Timer style dial wedge with a clear center readout. The timer should be legible, low update, and predictable, with a completion flash and a reset and quit flow that avoid accidental triggers. The easiest way to see it working is to open Pomodoro from the home screen, adjust to a visible value like 25 minutes, start the timer, confirm that the wedge shrinks once per minute, pause and resume, and finally observe the five-cycle full-screen invert flash at completion.


## Progress


- [x] 2026-01-17 Reformatted SPEC_pomodoro.md into plan.md formatting per plan-writer spec.
- [x] 2026-01-17 Implement Pomodoro activity, UI, and state machine.
- [x] 2026-01-17 Wire Pomodoro into the home screen and main activity routing.
- [x] 2026-01-17 Add persistence for last-used duration and add unit tests.


## Surprises & Discoveries


InputManager only exposes per-update press and release events plus a single held-time duration, so the Pomodoro activity must implement its own double-press window and long-press gating for the Back/Exit button. Evidence lives in open-x4-sdk/libs/hardware/InputManager/include/InputManager.h.

PlatformIO native tests require a host GCC toolchain; the current environment is missing `gcc`/`g++`, so `pio test -e native` fails until those are installed.


## Decision Log


2026-01-17 (User): This plan drives firmware implementation, not just a design document, and the Pomodoro activity must be accessible from the home screen next to InfoBoard.

2026-01-17 (User): The last-used duration must persist across app exits and power cycles.

2026-01-17 (User): The app should feel calm, so the dial should update once per minute and only a small seconds text should update every 10 seconds, used only when under one minute remaining.

2026-01-17 (User): Long press quit should be immediate with no confirmation.

2026-01-17 (User): Use grayscale if available at 4 levels, otherwise use a dense dither wedge fill instead of sparse dots.

2026-01-17 (User): Use existing UI kit behavior that shows action labels above buttons via the mapped button hints instead of static legend rows.

2026-01-17 (User): Add unit tests that can be verified before building the full firmware.

2026-01-17 (User): Only flash the screen at pomodoro start and completion; add persistent daily/weekly completion stats with SD logging, add reset/quit instruction lines, extend the reset double-press window, and hide Up/Down hints while running.

2026-01-17 (Implementation): Switched the pomodoro wedge to a denser BW dither fill to keep shading consistent across fast refreshes, skipping grayscale overlay for the wedge.


## Outcomes & Retrospective


Pomodoro is now implemented with a dedicated activity, a testable model, and a Time Timer-inspired dial render. The activity is wired into the home screen and main routing, persists the last-used duration in CrossPointState v3, and prevents auto-sleep during runs and completion flashes. A native PlatformIO test environment and Unity tests were added to validate the model’s timing, reset flow, and seconds bucket behavior. No items were deferred.


## Context and Orientation


The repository firmware lives under src/ with activities in src/activities/. The home screen is implemented in src/activities/home/HomeActivity.cpp and uses mappedInput.mapLabels plus renderer.drawButtonHints to render action labels above the four buttons; this is the pattern to follow for Pomodoro. Activity routing and creation are in src/main.cpp, which constructs HomeActivity and routes to other activities like InfoBoard. Activity base behavior, including preventAutoSleep, is in src/activities/Activity.h. Persistent app state is stored in src/CrossPointState.h and src/CrossPointState.cpp, and settings are stored in src/CrossPointSettings.h and src/CrossPointSettings.cpp. Input events are provided by MappedInputManager in src/MappedInputManager.h and src/MappedInputManager.cpp, which adapts to button layout settings. Low-level input event support is in open-x4-sdk/libs/hardware/InputManager/include/InputManager.h. Rendering utilities and refresh modes are in lib/GfxRenderer/GfxRenderer.h and the EInkDisplay library in open-x4-sdk. There is no PLANS.md or equivalent found in the repository at the time of writing.


## Plan of Work


Create a new Pomodoro activity under src/activities/pomodoro/ with a PomodoroActivity.h and PomodoroActivity.cpp that implement rendering, input handling, and timing. The activity should own a small, testable state machine class (for example PomodoroModel.h/.cpp in the same folder or in src/util/) that handles state transitions, remaining time, and reset and quit flows without any rendering code so it can be unit tested. Wire the activity into src/main.cpp by adding an include and an onGoToPomodoro function, and add a new callback parameter to HomeActivity so it can open Pomodoro. Update src/activities/home/HomeActivity.h and src/activities/home/HomeActivity.cpp to insert a new menu tile labeled Pomodoro adjacent to InfoBoard, adjust menu count and index mapping accordingly, and ensure the selection and rendering logic remain correct when Continue Reading or OPDS entries are present.

Implement persistence for the last-used duration by adding a new field to CrossPointState, updating loadFromFile and saveToFile, and updating the Pomodoro activity to read this value on entry and write it whenever the user changes the duration or completes a session. Use the Activity.preventAutoSleep hook to keep the device awake while RUNNING or during the completion flash, and fall back to IDLE on app exit or power loss as described in the spec. Implement the e-ink rendering strategy in the Pomodoro activity with a square frame, circular dial, minute ticks, numerals at 5 minute increments, a ring-band wedge for remaining time, and a center readout of the remaining minutes. Use grayscale rendering when available by switching the renderer render mode as shown in other activities, and use a dense dither pattern fill for the wedge when grayscale is not available. Use mappedInput.mapLabels and renderer.drawButtonHints to present action labels above buttons and avoid a second static legend row, keeping the status line as the single source of truth for feedback like pause prompts and reset hints.

Add unit tests under test/ that validate the Pomodoro state machine transitions, timing boundaries, and double-press window behavior. Because there is no existing host test environment, add a native test environment in platformio.ini that builds only the model and tests without Arduino dependencies, so pio test can be run before building or flashing. Keep the model free of Arduino headers to allow host compilation.


## Concrete Steps


    Working directory: /home/ubuntu/dev/crosspoint-reader
    $ rg -n "InfoBoard" src/main.cpp src/activities/home/HomeActivity.cpp
    20:#include "activities/infoboard/InfoBoardActivity.h"
    184:void onGoToInfoBoard() {
    502:  std::vector<const char*> menuItems = {"Browse Files", "File Transfer", "InfoBoard", "Settings"};

    $ mkdir -p src/activities/pomodoro
    $ $EDITOR src/activities/pomodoro/PomodoroActivity.h
    $ $EDITOR src/activities/pomodoro/PomodoroActivity.cpp
    $ $EDITOR src/activities/pomodoro/PomodoroModel.h
    $ $EDITOR src/activities/pomodoro/PomodoroModel.cpp

    $ $EDITOR src/main.cpp
    $ $EDITOR src/activities/home/HomeActivity.h
    $ $EDITOR src/activities/home/HomeActivity.cpp
    $ $EDITOR src/CrossPointState.h
    $ $EDITOR src/CrossPointState.cpp

    $ mkdir -p test/pomodoro_model
    $ $EDITOR test/pomodoro_model/test_main.cpp
    $ $EDITOR platformio.ini

    $ bin/clang-format-fix src/activities/pomodoro/PomodoroActivity.cpp
    $ pio test -e native
    (tests should report PASSED for pomodoro_model)
    $ pio run
    (build should succeed for env:default)


## Validation and Acceptance


The Pomodoro tile appears in the home screen menu next to InfoBoard and launches a new activity when selected, using the same action label style as other activities. From IDLE, Up and Down adjust time in 5 minute steps between 5 and 60, Select starts, and the status line reflects Ready with the current duration. While RUNNING, the dial wedge shrinks once per minute, the center readout shows remaining minutes, and when the remaining time is below one minute a small seconds text appears and updates every 10 seconds. Up and Down are ignored while running with a brief status prompt to pause to adjust. Select pauses and resumes, with the wedge and center text frozen while paused.

When remaining time reaches zero, the screen flashes full-screen invert for five cycles, any button press can interrupt the flashing early, and the DONE screen appears with Select starting a new session at the last-used duration. The start action also triggers the flash, while pause/resume/reset do not. A double press of Back within the reset window returns to IDLE and shows Reset, and a long press of Back quits immediately. The last-used duration persists across app exits and power cycles by storing it in CrossPointState. Daily and weekly completion totals (with hours) appear under the title and are derived from an SD card log of start/end/duration entries. Auto-sleep is prevented during RUNNING and the completion flash, and on wake from sleep or app relaunch the activity starts in IDLE rather than resuming a prior session.

Unit tests pass for the state machine, specifically covering duration clamping, transitions between IDLE, RUNNING, PAUSED, DONE, reset behavior, double-press timing, and that the 10 second tick for the small seconds display advances without per-second updates.


## Idempotence and Recovery


All steps are safe to repeat. Re-running formatting, tests, or builds should not change runtime behavior. If the Pomodoro activity causes unexpected behavior, it can be disabled by removing the HomeActivity entry and the onGoToPomodoro routing in src/main.cpp, and the persisted duration can be ignored by falling back to the default 25 minute duration when loading state fails. If the grayscale wedge is unreadable on device, switching to the dense dither fill is a safe fallback without changing the rest of the UI or logic.


## Artifacts and Notes


    Planned state model sketch:
    enum class PomodoroState { Idle, Running, Paused, Done };
    struct PomodoroModel {
      PomodoroState state;
      int durationMinutes;
      int remainingSeconds;
      unsigned long lastTickMs;
      bool updateForSecondStamp(unsigned long nowMs) const;
      void start(unsigned long nowMs);
      void pause();
      void resume(unsigned long nowMs);
      void resetToIdle();
      void tick(unsigned long nowMs);
    };

    Button labels should follow existing UI hints:
    const auto labels = mappedInput.mapLabels("Back", "Start", "-5", "+5");
    renderer.drawButtonHints(UI_10_FONT_ID, labels.btn1, labels.btn2, labels.btn3, labels.btn4);


## Interfaces and Dependencies


PomodoroActivity should subclass Activity and override onEnter, loop, and preventAutoSleep. It must depend on MappedInputManager for input mapping, GfxRenderer for drawing, and CrossPointState for persistence. Timing should use millis() and should not depend on delay. Rendering should use GfxRenderer primitives (drawLine, fillRect, fillPolygon, drawText) with EInkDisplay refresh modes; for grayscale, use GfxRenderer::setRenderMode and the grayscale buffer copy/display functions used in SleepActivity and reader activities.

    class PomodoroActivity final : public Activity {
     public:
      explicit PomodoroActivity(GfxRenderer& renderer, MappedInputManager& mappedInput, const std::function<void()>& onGoHome);
      void onEnter() override;
      void loop() override;
      bool preventAutoSleep() override;
    };

    MappedInputManager inputs to use:
    mappedInput.wasPressed(MappedInputManager::Button::Back);
    mappedInput.wasPressed(MappedInputManager::Button::Confirm);
    mappedInput.wasPressed(MappedInputManager::Button::Up);
    mappedInput.wasPressed(MappedInputManager::Button::Down);
    mappedInput.getHeldTime();


Change note: Reformatted docs_for_llm/SPEC_pomodoro.md to the plan.md structure and incorporated the clarified requirements on 2026-01-17.
