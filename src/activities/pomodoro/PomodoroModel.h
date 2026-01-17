#pragma once

#include <cstdint>

enum class PomodoroState { Idle, Running, Paused, Done };

class PomodoroModel {
 public:
  static constexpr int kMinMinutes = 5;
  static constexpr int kMaxMinutes = 60;
  static constexpr int kStepMinutes = 5;
  static constexpr uint32_t kResetConfirmWindowMs = 2000;
  static constexpr int kSecondsBucket = 10;

  PomodoroModel();

  PomodoroState getState() const { return state; }
  int getDurationMinutes() const { return durationMinutes; }
  int getRemainingSeconds() const { return remainingSeconds; }
  int getDisplayMinutes() const;
  bool shouldShowSeconds() const;
  int getDisplaySecondsBucket() const;
  bool isRunning() const { return state == PomodoroState::Running; }

  void setDurationMinutes(int minutes);
  bool adjustDurationSteps(int steps);
  bool adjustRemainingSteps(int steps);

  void start(uint32_t nowMs);
  void pause();
  void resume(uint32_t nowMs);
  void resetToIdle();
  bool tick(uint32_t nowMs);

  bool requestReset(uint32_t nowMs);
  bool isResetPromptActive(uint32_t nowMs);
  void clearResetPrompt();

 private:
  int normalizeDuration(int minutes) const;
  void applyDuration(int minutes);

  PomodoroState state = PomodoroState::Idle;
  int durationMinutes = 25;
  int remainingSeconds = 25 * 60;
  uint32_t lastTickMs = 0;
  bool resetPromptActive = false;
  uint32_t resetPromptStartMs = 0;
};
