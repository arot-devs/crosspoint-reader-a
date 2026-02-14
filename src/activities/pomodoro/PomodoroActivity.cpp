#include "PomodoroActivity.h"

#include <Arduino.h>
#include <GfxRenderer.h>

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <cstdio>
#include <ctime>

#include "CrossPointSettings.h"
#include "HalStorage.h"
#include "MappedInputManager.h"
#include "components/UITheme.h"
#include "fontIds.h"

namespace {
constexpr float kPi = 3.1415926f;
constexpr float kTwoPi = 6.2831853f;
constexpr float kStartAngle = -kPi / 2.0f;

constexpr unsigned long kQuitHoldMs = 1200;
constexpr unsigned long kStatusMessageMs = 1500;
constexpr unsigned long kFlashIntervalMs = 180;
constexpr int kFlashToggles = 3;
constexpr int kPointerInset = 6;
constexpr int kPointerTip = 2;
constexpr int kHubRadius = 3;
constexpr int kOuterPadding = 12;
constexpr int kNumberInset = 18;
constexpr int kTickMajorLength = 12;
constexpr int kTickMinorLength = 6;
constexpr int kMargin = 20;
constexpr int kDitherModulo = 4;
constexpr int kDitherThreshold = 3;
constexpr uint32_t kMinValidEpoch = 1704067200U;
constexpr uint32_t kSecondsPerDay = 86400U;
constexpr uint32_t kSecondsPerWeek = 7U * kSecondsPerDay;
constexpr char kPomodoroLogPath[] = "/.crosspoint/pomodoro_log.txt";
constexpr char kPomodoroStatePath[] = "/.crosspoint/pomodoro_state.txt";
}  // namespace

PomodoroActivity::PomodoroActivity(GfxRenderer& renderer, MappedInputManager& mappedInput,
                                   const std::function<void()>& onGoHome)
    : Activity("Pomodoro", renderer, mappedInput), onGoHome(onGoHome) {}

void PomodoroActivity::onEnter() {
  Activity::onEnter();

  loadPersistedDuration();

  lastDisplayedMinutes = -1;
  lastDisplayedSecondsBucket = -1;
  lastShowSeconds = false;
  lastResetPromptActive = false;
  sessionStartEpoch = 0;
  loadStats();

  requestRender(HalDisplay::FULL_REFRESH);
}

void PomodoroActivity::onExit() { Activity::onExit(); }

bool PomodoroActivity::preventAutoSleep() { return flashActive || model.isRunning(); }

void PomodoroActivity::requestRender(const HalDisplay::RefreshMode mode) {
  renderRequired = true;
  if (mode == HalDisplay::FULL_REFRESH) {
    pendingRefresh = mode;
  } else if (pendingRefresh != HalDisplay::FULL_REFRESH) {
    pendingRefresh = mode;
  }
}

void PomodoroActivity::setTransientStatus(const char* message, const unsigned long durationMs) {
  transientStatus = message;
  transientStatusUntilMs = millis() + durationMs;
  requestRender(HalDisplay::FAST_REFRESH);
}

std::string PomodoroActivity::buildStatusText(const unsigned long nowMs) {
  if (!transientStatus.empty()) {
    if (nowMs < transientStatusUntilMs) {
      return transientStatus;
    }
    transientStatus.clear();
  }

  if (model.isResetPromptActive(nowMs)) {
    return "Press EXIT again to reset";
  }

  switch (model.getState()) {
    case PomodoroState::Idle:
      return "Ready (" + std::to_string(model.getDurationMinutes()) + " min)";
    case PomodoroState::Running:
      return "Running";
    case PomodoroState::Paused:
      return "Paused";
    case PomodoroState::Done:
      return "Done";
  }

  return "";
}

void PomodoroActivity::loadPersistedDuration() {
  model.setDurationMinutes(25);

  FsFile file = Storage.open(kPomodoroStatePath, O_RDONLY);
  if (!file) {
    return;
  }

  char buffer[16];
  size_t idx = 0;
  while (idx + 1 < sizeof(buffer)) {
    const int value = file.read();
    if (value < 0 || value == '\n' || value == '\r') {
      break;
    }
    buffer[idx++] = static_cast<char>(value);
  }
  buffer[idx] = '\0';
  file.close();

  if (idx == 0) {
    return;
  }

  char* end = nullptr;
  const long minutes = strtol(buffer, &end, 10);
  if (end == buffer || *end != '\0') {
    return;
  }

  if (minutes < PomodoroModel::kMinMinutes || minutes > PomodoroModel::kMaxMinutes) {
    return;
  }

  model.setDurationMinutes(static_cast<int>(minutes));
}

void PomodoroActivity::savePersistedDuration() {
  if (!Storage.ensureDirectoryExists("/.crosspoint")) {
    return;
  }

  FsFile file = Storage.open(kPomodoroStatePath, O_WRONLY | O_CREAT | O_TRUNC);
  if (!file) {
    return;
  }

  file.printf("%d\n", model.getDurationMinutes());
  file.close();
}

uint32_t PomodoroActivity::getNowEpochSeconds() {
  const time_t now = time(nullptr);
  if (now < 0) {
    return 0;
  }
  return static_cast<uint32_t>(now);
}

bool PomodoroActivity::isEpochValid(const uint32_t epochSeconds) { return epochSeconds >= kMinValidEpoch; }

std::string PomodoroActivity::formatHours(const float hours) {
  char buffer[16];
  snprintf(buffer, sizeof(buffer), "%.1f", hours);
  return std::string(buffer);
}

void PomodoroActivity::loadStats() {
  stats = {};
  const uint32_t nowEpoch = getNowEpochSeconds();
  stats.timeValid = isEpochValid(nowEpoch);

  FsFile file = Storage.open(kPomodoroLogPath, O_RDONLY);
  if (!file) {
    return;
  }

  uint32_t dayStart = 0;
  uint32_t weekStart = 0;
  if (stats.timeValid) {
    const time_t nowTime = static_cast<time_t>(nowEpoch);
    tm localTime = {};
    if (localtime_r(&nowTime, &localTime) != nullptr) {
      tm dayStartTm = localTime;
      dayStartTm.tm_hour = 0;
      dayStartTm.tm_min = 0;
      dayStartTm.tm_sec = 0;
      const time_t dayStartTime = mktime(&dayStartTm);
      if (dayStartTime >= 0) {
        dayStart = static_cast<uint32_t>(dayStartTime);
      }

      tm weekStartTm = dayStartTm;
      const int daysSinceSunday = weekStartTm.tm_wday;
      weekStartTm.tm_mday -= daysSinceSunday;
      const time_t weekStartTime = mktime(&weekStartTm);
      if (weekStartTime >= 0) {
        weekStart = static_cast<uint32_t>(weekStartTime);
      }
    }
  }

  if (stats.timeValid) {
    if (dayStart == 0) {
      dayStart = nowEpoch - kSecondsPerDay;
    }
    if (weekStart == 0) {
      weekStart = nowEpoch - kSecondsPerWeek;
    }
  }
  char line[64];
  size_t idx = 0;

  while (true) {
    const int value = file.read();
    if (value < 0) {
      break;
    }
    if (value == '\n' || idx + 1 >= sizeof(line)) {
      line[idx] = '\0';
      idx = 0;
      if (line[0] == '\0') {
        continue;
      }

      char* end = nullptr;
      (void)strtoul(line, &end, 10);
      if (!end || *end != ',') {
        continue;
      }
      const uint32_t endEpoch = static_cast<uint32_t>(strtoul(end + 1, &end, 10));
      if (!end || *end != ',') {
        continue;
      }
      const uint32_t durationSeconds = static_cast<uint32_t>(strtoul(end + 1, &end, 10));
      if (durationSeconds == 0) {
        continue;
      }

      const float hours = static_cast<float>(durationSeconds) / 3600.0f;
      if (!stats.timeValid) {
        stats.todayCount += 1;
        stats.weekCount += 1;
        stats.todayHours += hours;
        stats.weekHours += hours;
        continue;
      }

      if (!isEpochValid(endEpoch)) {
        continue;
      }

      if (endEpoch >= dayStart) {
        stats.todayCount += 1;
        stats.todayHours += hours;
      }
      if (endEpoch >= weekStart) {
        stats.weekCount += 1;
        stats.weekHours += hours;
      }
    } else {
      line[idx++] = static_cast<char>(value);
    }
  }

  if (idx > 0) {
    line[idx] = '\0';
    char* end = nullptr;
    (void)strtoul(line, &end, 10);
    if (end && *end == ',') {
      const uint32_t endEpoch = static_cast<uint32_t>(strtoul(end + 1, &end, 10));
      if (end && *end == ',') {
        const uint32_t durationSeconds = static_cast<uint32_t>(strtoul(end + 1, &end, 10));
        if (durationSeconds > 0) {
          const float hours = static_cast<float>(durationSeconds) / 3600.0f;
          if (!stats.timeValid) {
            stats.todayCount += 1;
            stats.weekCount += 1;
            stats.todayHours += hours;
            stats.weekHours += hours;
          } else if (isEpochValid(endEpoch)) {
            if (endEpoch >= dayStart) {
              stats.todayCount += 1;
              stats.todayHours += hours;
            }
            if (endEpoch >= weekStart) {
              stats.weekCount += 1;
              stats.weekHours += hours;
            }
          }
        }
      }
    }
  }

  file.close();
}

void PomodoroActivity::logPomodoroCompletion(const uint32_t endEpochSeconds) {
  uint32_t durationSeconds = static_cast<uint32_t>(model.getDurationMinutes() * 60);
  if (!Storage.ensureDirectoryExists("/.crosspoint")) {
    return;
  }

  FsFile file = Storage.open(kPomodoroLogPath, O_WRONLY | O_CREAT | O_APPEND);
  if (!file) {
    return;
  }

  uint32_t startEpoch = sessionStartEpoch;
  if (isEpochValid(startEpoch) && isEpochValid(endEpochSeconds) && endEpochSeconds >= startEpoch) {
    durationSeconds = endEpochSeconds - startEpoch;
  } else if (isEpochValid(endEpochSeconds) && endEpochSeconds >= durationSeconds) {
    startEpoch = endEpochSeconds - durationSeconds;
  }
  file.printf("%lu,%lu,%lu\n", static_cast<unsigned long>(startEpoch), static_cast<unsigned long>(endEpochSeconds),
              static_cast<unsigned long>(durationSeconds));
  file.close();
  loadStats();
}

void PomodoroActivity::loop() {
  const unsigned long nowMs = millis();

  if (flashActive) {
    if (mappedInput.wasAnyPressed() || mappedInput.wasAnyReleased()) {
      flashActive = false;
      requestRender(HalDisplay::FAST_REFRESH);
      return;
    }

    if (nowMs >= nextFlashMs) {
      renderer.invertScreen();
      renderer.displayBuffer(HalDisplay::FAST_REFRESH);
      flashRemainingToggles--;
      nextFlashMs = nowMs + kFlashIntervalMs;
      if (flashRemainingToggles <= 0) {
        flashActive = false;
        requestRender(HalDisplay::FAST_REFRESH);
      }
    }
    return;
  }

  if (mappedInput.isPressed(MappedInputManager::Button::Back) && mappedInput.getHeldTime() >= kQuitHoldMs) {
    onGoHome();
    return;
  }

  const bool confirmReleased = mappedInput.wasReleased(MappedInputManager::Button::Confirm);
  const bool backReleased = mappedInput.wasReleased(MappedInputManager::Button::Back);
  const bool upReleased = mappedInput.wasReleased(MappedInputManager::Button::Up);
  const bool downReleased = mappedInput.wasReleased(MappedInputManager::Button::Down);

  if (confirmReleased) {
    if (model.getState() == PomodoroState::Running) {
      model.pause();
      requestRender(HalDisplay::FAST_REFRESH);
    } else if (model.getState() == PomodoroState::Paused) {
      model.resume(nowMs);
      requestRender(HalDisplay::FAST_REFRESH);
    } else {
      sessionStartEpoch = getNowEpochSeconds();
      model.start(nowMs);
      flashActive = true;
      flashRemainingToggles = kFlashToggles;
      nextFlashMs = nowMs;
      return;
    }
  }

  if (upReleased || downReleased) {
    const int delta = upReleased ? PomodoroModel::kStepMinutes : -PomodoroModel::kStepMinutes;
    bool changed = false;
    if (model.getState() == PomodoroState::Paused) {
      changed = model.adjustRemainingSteps(delta / PomodoroModel::kStepMinutes);
    } else if (model.getState() == PomodoroState::Idle || model.getState() == PomodoroState::Done) {
      changed = model.adjustDurationSteps(delta / PomodoroModel::kStepMinutes);
    } else {
      setTransientStatus("Pause to adjust", kStatusMessageMs);
    }

    if (changed) {
      savePersistedDuration();
      requestRender(HalDisplay::FAST_REFRESH);
    } else if (model.getState() != PomodoroState::Running) {
      if (delta > 0) {
        setTransientStatus("Max 60 min", kStatusMessageMs);
      } else {
        setTransientStatus("Min 5 min", kStatusMessageMs);
      }
    }
  }

  if (backReleased) {
    if (model.requestReset(nowMs)) {
      sessionStartEpoch = 0;
      setTransientStatus("Reset", kStatusMessageMs);
      requestRender(HalDisplay::FAST_REFRESH);
    } else if (model.getState() == PomodoroState::Running || model.getState() == PomodoroState::Paused) {
      requestRender(HalDisplay::FAST_REFRESH);
    }
  }

  if (model.tick(nowMs)) {
    logPomodoroCompletion(getNowEpochSeconds());
    sessionStartEpoch = 0;
    flashActive = true;
    flashRemainingToggles = kFlashToggles;
    nextFlashMs = nowMs;
    return;
  }

  if (!transientStatus.empty() && nowMs >= transientStatusUntilMs) {
    transientStatus.clear();
    requestRender(HalDisplay::FAST_REFRESH);
  }

  const bool resetPromptActive = model.isResetPromptActive(nowMs);
  if (resetPromptActive != lastResetPromptActive) {
    lastResetPromptActive = resetPromptActive;
    requestRender(HalDisplay::FAST_REFRESH);
  }

  const bool showSeconds = model.shouldShowSeconds();
  if (showSeconds) {
    const int secondsBucket = model.getDisplaySecondsBucket();
    if (!lastShowSeconds || secondsBucket != lastDisplayedSecondsBucket) {
      requestRender(HalDisplay::FAST_REFRESH);
    }
  } else {
    const int minutes = model.getDisplayMinutes();
    if (lastShowSeconds || minutes != lastDisplayedMinutes) {
      requestRender(HalDisplay::FAST_REFRESH);
    }
  }

  if (renderRequired) {
    render(pendingRefresh);
    pendingRefresh = HalDisplay::FAST_REFRESH;
    renderRequired = false;
  }
}

void PomodoroActivity::render(const HalDisplay::RefreshMode mode) {
  renderer.clearScreen();

  const int pageWidth = renderer.getScreenWidth();
  const int pageHeight = renderer.getScreenHeight();

  const int statsFont = SMALL_FONT_ID;
  const int statsLineHeight = renderer.getLineHeight(statsFont);
  const int titleHeight = renderer.getLineHeight(UI_12_FONT_ID);
  const int titleY = 8;
  renderer.drawCenteredText(UI_12_FONT_ID, titleY, "POMODORO", true, EpdFontFamily::BOLD);

  const int statsStartY = titleY + titleHeight + 6;
  const int statsBlockHeight = (2 * statsLineHeight) + 2;
  const int headerHeight = statsStartY + statsBlockHeight + 8;
  const std::string todayText = "You have completed " + std::to_string(stats.todayCount) + " pomodoros (" +
                                formatHours(stats.todayHours) + " hours) today";
  const std::string weekText = "You have completed " + std::to_string(stats.weekCount) + " pomodoros (" +
                               formatHours(stats.weekHours) + " hours) this week";
  renderer.drawCenteredText(statsFont, statsStartY, todayText.c_str());
  renderer.drawCenteredText(statsFont, statsStartY + statsLineHeight + 2, weekText.c_str());

  const int lineHeight = renderer.getLineHeight(UI_10_FONT_ID);
  const int footerHeight = (3 * lineHeight) + 48;
  const int availableHeight = pageHeight - headerHeight - footerHeight;
  const int dialSize = std::min(pageWidth - 2 * kMargin, availableHeight);
  const int dialX = (pageWidth - dialSize) / 2;
  const int dialY = headerHeight + (availableHeight - dialSize) / 2;

  renderer.drawRect(dialX, dialY, dialSize, dialSize);

  const int centerX = dialX + dialSize / 2;
  const int centerY = dialY + dialSize / 2;
  const int outerRadius = dialSize / 2 - kOuterPadding;
  const int innerRadius = static_cast<int>(outerRadius * 0.70f);
  const int numberRadius = innerRadius - kNumberInset;

  const bool showSeconds = model.shouldShowSeconds();
  const int displayValue = showSeconds ? model.getDisplaySecondsBucket() : model.getDisplayMinutes();
  const float remainingFraction =
      std::min(1.0f, std::max(0.0f, static_cast<float>(model.getRemainingSeconds()) / 3600.0f));
  const float sweepAngle = -remainingFraction * kTwoPi;
  const float pointerAngle = kStartAngle + sweepAngle;

  renderDial(centerX, centerY, outerRadius, innerRadius, sweepAngle);
  renderDialTicks(centerX, centerY, outerRadius - 2, true);
  renderDialNumerals(centerX, centerY, numberRadius, true);
  if (remainingFraction > 0.0f) {
    renderPointer(centerX, centerY, outerRadius, pointerAngle, true);
  }
  renderCenterHub(centerX, centerY, true);

  if (model.getState() == PomodoroState::Done) {
    const int doneFont = NOTOSANS_18_FONT_ID;
    const int subFont = UI_10_FONT_ID;
    const int doneHeight = renderer.getLineHeight(doneFont);
    const int subHeight = renderer.getLineHeight(subFont);
    const int totalHeight = doneHeight + subHeight + 6;
    const int topY = centerY - totalHeight / 2;
    renderer.drawCenteredText(doneFont, topY, "DONE", true, EpdFontFamily::BOLD);
    renderer.drawCenteredText(subFont, topY + doneHeight + 6, "Press SEL to start", true);
  } else {
    renderCenterReadout(centerX, centerY, showSeconds, displayValue);
  }

  const std::string statusText = buildStatusText(millis());
  const int hintTopY = pageHeight - 40;
  const int instructionsY2 = hintTopY - lineHeight - 4;
  const int instructionsY1 = instructionsY2 - lineHeight;
  const int statusY = instructionsY1 - lineHeight - 4;
  renderer.drawCenteredText(UI_10_FONT_ID, statusY, statusText.c_str());
  renderer.drawCenteredText(UI_10_FONT_ID, instructionsY1, "Press EXIT twice to reset");
  renderer.drawCenteredText(UI_10_FONT_ID, instructionsY2, "Long press EXIT to quit");

  const char* backLabel = "Exit";
  const char* confirmLabel = "";
  const char* prevLabel = "";
  const char* nextLabel = "";

  switch (model.getState()) {
    case PomodoroState::Idle:
    case PomodoroState::Done:
      confirmLabel = "Start";
      prevLabel = "-5";
      nextLabel = "+5";
      break;
    case PomodoroState::Running:
      confirmLabel = "Pause";
      prevLabel = "";
      nextLabel = "";
      break;
    case PomodoroState::Paused:
      confirmLabel = "Resume";
      prevLabel = "-5";
      nextLabel = "+5";
      break;
  }

  const auto labels = mappedInput.mapLabels(backLabel, confirmLabel, prevLabel, nextLabel);
  GUI.drawButtonHints(renderer, labels.btn1, labels.btn2, labels.btn3, labels.btn4);

  renderer.displayBuffer(mode);

  lastShowSeconds = showSeconds;
  lastDisplayedMinutes = model.getDisplayMinutes();
  lastDisplayedSecondsBucket = model.getDisplaySecondsBucket();
}

void PomodoroActivity::renderDial(const int centerX, const int centerY, const int outerRadius, const int innerRadius,
                                  const float sweepAngle) {
  if (sweepAngle == 0.0f) {
    return;
  }
  drawWedgeDithered(centerX, centerY, innerRadius, outerRadius, sweepAngle);
}

void PomodoroActivity::renderDialTicks(const int centerX, const int centerY, const int outerRadius, const bool state) {
  for (int i = 0; i < 60; i++) {
    const bool major = (i % 5) == 0;
    const int length = major ? kTickMajorLength : kTickMinorLength;
    const float angle = kStartAngle + (static_cast<float>(i) / 60.0f) * kTwoPi;
    const float cosA = std::cos(angle);
    const float sinA = std::sin(angle);
    drawRadialLine(centerX, centerY, cosA, sinA, outerRadius - length, outerRadius, state);
  }
}

void PomodoroActivity::renderDialNumerals(const int centerX, const int centerY, const int numberRadius,
                                          const bool state) {
  for (int i = 0; i < 12; i++) {
    const int minutes = i * 5;
    const int labelMinutes = (60 - minutes) % 60;
    const std::string label = std::to_string(labelMinutes);
    const float angle = kStartAngle + (static_cast<float>(minutes) / 60.0f) * kTwoPi;
    const float cosA = std::cos(angle);
    const float sinA = std::sin(angle);
    const int x = centerX + static_cast<int>(std::round(cosA * numberRadius));
    const int y = centerY + static_cast<int>(std::round(sinA * numberRadius));
    const int textWidth = renderer.getTextWidth(UI_10_FONT_ID, label.c_str());
    const int textHeight = renderer.getLineHeight(UI_10_FONT_ID);
    renderer.drawText(UI_10_FONT_ID, x - textWidth / 2, y - textHeight / 2, label.c_str(), state);
  }
}

void PomodoroActivity::renderPointer(const int centerX, const int centerY, const int outerRadius, const float angle,
                                     const bool state) {
  const float cosA = std::cos(angle);
  const float sinA = std::sin(angle);

  const float tipRadius = outerRadius + kPointerTip;
  const float baseRadius = outerRadius - kPointerInset;
  const float spread = 0.08f;

  const int xPoints[3] = {
    centerX + static_cast<int>(std::round(cosA * tipRadius)),
    centerX + static_cast<int>(std::round(std::cos(angle + spread) * baseRadius)),
    centerX + static_cast<int>(std::round(std::cos(angle - spread) * baseRadius)),
  };
  const int yPoints[3] = {
    centerY + static_cast<int>(std::round(sinA * tipRadius)),
    centerY + static_cast<int>(std::round(std::sin(angle + spread) * baseRadius)),
    centerY + static_cast<int>(std::round(std::sin(angle - spread) * baseRadius)),
  };

  renderer.fillPolygon(xPoints, yPoints, 3, state);
}

void PomodoroActivity::renderCenterHub(const int centerX, const int centerY, const bool state) {
  if (!state) {
    return;
  }
  drawDitheredDisk(centerX, centerY, kHubRadius);
}

void PomodoroActivity::renderCenterReadout(const int centerX, const int centerY, const bool showSeconds,
                                           const int displayValue) {
  const int valueFont = NOTOSANS_18_FONT_ID;
  const int labelFont = UI_10_FONT_ID;
  const int valueHeight = renderer.getLineHeight(valueFont);
  const int labelHeight = renderer.getLineHeight(labelFont);
  const int totalHeight = valueHeight + labelHeight;
  const int topY = centerY - totalHeight / 2;

  const std::string valueText = std::to_string(displayValue);
  const char* label = showSeconds ? "s" : "min";

  renderer.drawCenteredText(valueFont, topY, valueText.c_str(), true, EpdFontFamily::BOLD);
  renderer.drawCenteredText(labelFont, topY + valueHeight, label, true);

  if (model.getState() == PomodoroState::Paused) {
    const int pausedY = topY - renderer.getLineHeight(UI_10_FONT_ID) - 4;
    renderer.drawCenteredText(UI_10_FONT_ID, pausedY, "PAUSED", true, EpdFontFamily::BOLD);
  }
}

void PomodoroActivity::drawDitheredDisk(const int centerX, const int centerY, const int radius) {
  for (int y = -radius; y <= radius; y++) {
    for (int x = -radius; x <= radius; x++) {
      if ((x * x + y * y) <= radius * radius) {
        if (((x + y + radius) % kDitherModulo) < kDitherThreshold) {
          renderer.drawPixel(centerX + x, centerY + y, true);
        }
      }
    }
  }
}

void PomodoroActivity::drawRadialLine(const int centerX, const int centerY, const float cosA, const float sinA,
                                      const int innerRadius, const int outerRadius, const bool state) {
  for (int r = innerRadius; r <= outerRadius; r++) {
    const int x = centerX + static_cast<int>(std::round(cosA * r));
    const int y = centerY + static_cast<int>(std::round(sinA * r));
    renderer.drawPixel(x, y, state);
  }
}

void PomodoroActivity::drawDitheredRadialLine(const int centerX, const int centerY, const float cosA, const float sinA,
                                              const int innerRadius, const int outerRadius, const int phase) {
  for (int r = innerRadius; r <= outerRadius; r++) {
    if (((r + phase) % kDitherModulo) < kDitherThreshold) {
      const int x = centerX + static_cast<int>(std::round(cosA * r));
      const int y = centerY + static_cast<int>(std::round(sinA * r));
      renderer.drawPixel(x, y, true);
    }
  }
}

void PomodoroActivity::drawWedgeSolid(const int centerX, const int centerY, const int innerRadius,
                                      const int outerRadius, const float sweepAngle, const bool state) {
  if (sweepAngle == 0.0f) {
    return;
  }
  const float sweep = std::max(-kTwoPi, std::min(kTwoPi, sweepAngle));
  const int steps = std::max(1, static_cast<int>(std::round(std::abs(sweep) * 180.0f / kPi)));
  for (int i = 0; i <= steps; i++) {
    const float angle = kStartAngle + sweep * (static_cast<float>(i) / steps);
    const float cosA = std::cos(angle);
    const float sinA = std::sin(angle);
    drawRadialLine(centerX, centerY, cosA, sinA, innerRadius, outerRadius, state);
  }
}

void PomodoroActivity::drawWedgeDithered(const int centerX, const int centerY, const int innerRadius,
                                         const int outerRadius, const float sweepAngle) {
  if (sweepAngle == 0.0f) {
    return;
  }
  const float sweep = std::max(-kTwoPi, std::min(kTwoPi, sweepAngle));
  const int steps = std::max(1, static_cast<int>(std::round(std::abs(sweep) * 180.0f / kPi)));
  for (int i = 0; i <= steps; i++) {
    const float angle = kStartAngle + sweep * (static_cast<float>(i) / steps);
    const float cosA = std::cos(angle);
    const float sinA = std::sin(angle);
    drawDitheredRadialLine(centerX, centerY, cosA, sinA, innerRadius, outerRadius, i);
  }
}
