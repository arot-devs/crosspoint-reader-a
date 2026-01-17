#include "PomodoroModel.h"

#include <algorithm>

PomodoroModel::PomodoroModel() = default;

int PomodoroModel::normalizeDuration(const int minutes) const {
  int clamped = std::min(kMaxMinutes, std::max(kMinMinutes, minutes));
  int rounded = ((clamped + (kStepMinutes / 2)) / kStepMinutes) * kStepMinutes;
  if (rounded < kMinMinutes) {
    rounded = kMinMinutes;
  } else if (rounded > kMaxMinutes) {
    rounded = kMaxMinutes;
  }
  return rounded;
}

void PomodoroModel::applyDuration(const int minutes) {
  durationMinutes = normalizeDuration(minutes);
  if (state == PomodoroState::Idle || state == PomodoroState::Done) {
    remainingSeconds = durationMinutes * 60;
  }
}

int PomodoroModel::getDisplayMinutes() const {
  if (remainingSeconds <= 0) {
    return 0;
  }
  return (remainingSeconds + 59) / 60;
}

bool PomodoroModel::shouldShowSeconds() const {
  if (state != PomodoroState::Running && state != PomodoroState::Paused) {
    return false;
  }
  return remainingSeconds < 60;
}

int PomodoroModel::getDisplaySecondsBucket() const {
  if (!shouldShowSeconds()) {
    return -1;
  }
  if (remainingSeconds <= 0) {
    return 0;
  }
  const int bucket = (remainingSeconds / kSecondsBucket) * kSecondsBucket;
  return bucket == 0 ? kSecondsBucket : bucket;
}

void PomodoroModel::setDurationMinutes(const int minutes) { applyDuration(minutes); }

bool PomodoroModel::adjustDurationSteps(const int steps) {
  const int before = durationMinutes;
  applyDuration(durationMinutes + steps * kStepMinutes);
  return durationMinutes != before;
}

bool PomodoroModel::adjustRemainingSteps(const int steps) {
  const int before = remainingSeconds;
  const int currentMinutes = (remainingSeconds + 59) / 60;
  const int updatedMinutes = normalizeDuration(currentMinutes + steps * kStepMinutes);
  durationMinutes = updatedMinutes;
  remainingSeconds = updatedMinutes * 60;
  return remainingSeconds != before;
}

void PomodoroModel::start(const uint32_t nowMs) {
  if (state == PomodoroState::Running) {
    return;
  }
  state = PomodoroState::Running;
  remainingSeconds = durationMinutes * 60;
  lastTickMs = nowMs;
  clearResetPrompt();
}

void PomodoroModel::pause() {
  if (state != PomodoroState::Running) {
    return;
  }
  state = PomodoroState::Paused;
}

void PomodoroModel::resume(const uint32_t nowMs) {
  if (state != PomodoroState::Paused) {
    return;
  }
  state = PomodoroState::Running;
  lastTickMs = nowMs;
  clearResetPrompt();
}

void PomodoroModel::resetToIdle() {
  state = PomodoroState::Idle;
  remainingSeconds = durationMinutes * 60;
  clearResetPrompt();
}

bool PomodoroModel::tick(const uint32_t nowMs) {
  if (state != PomodoroState::Running) {
    return false;
  }

  if (nowMs < lastTickMs) {
    lastTickMs = nowMs;
    return false;
  }

  const uint32_t elapsedSeconds = (nowMs - lastTickMs) / 1000;
  if (elapsedSeconds == 0) {
    return false;
  }

  remainingSeconds -= static_cast<int>(elapsedSeconds);
  lastTickMs += elapsedSeconds * 1000;

  if (remainingSeconds <= 0) {
    state = PomodoroState::Done;
    remainingSeconds = durationMinutes * 60;
    clearResetPrompt();
    return true;
  }

  return false;
}

bool PomodoroModel::requestReset(const uint32_t nowMs) {
  if (state != PomodoroState::Running && state != PomodoroState::Paused) {
    return false;
  }

  if (resetPromptActive) {
    const uint32_t elapsed = nowMs - resetPromptStartMs;
    if (elapsed <= kResetConfirmWindowMs) {
      resetToIdle();
      return true;
    }
  }

  resetPromptActive = true;
  resetPromptStartMs = nowMs;
  return false;
}

bool PomodoroModel::isResetPromptActive(const uint32_t nowMs) {
  if (!resetPromptActive) {
    return false;
  }

  const uint32_t elapsed = nowMs - resetPromptStartMs;
  if (elapsed > kResetConfirmWindowMs) {
    resetPromptActive = false;
    return false;
  }

  return true;
}

void PomodoroModel::clearResetPrompt() { resetPromptActive = false; }
