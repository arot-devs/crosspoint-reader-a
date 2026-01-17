#include "PomodoroActivity.h"

#include <Arduino.h>
#include <GfxRenderer.h>

#include <algorithm>
#include <cmath>

#include "CrossPointSettings.h"
#include "CrossPointState.h"
#include "MappedInputManager.h"
#include "fontIds.h"

namespace {
constexpr float kPi = 3.1415926f;
constexpr float kTwoPi = 6.2831853f;
constexpr float kStartAngle = -kPi / 2.0f;

constexpr unsigned long kQuitHoldMs = 1200;
constexpr unsigned long kStatusMessageMs = 1500;
constexpr unsigned long kFlashIntervalMs = 180;
constexpr int kFlashCycles = 5;
constexpr int kPointerInset = 6;
constexpr int kPointerTip = 2;
constexpr int kHubRadius = 4;
constexpr int kOuterPadding = 12;
constexpr int kNumberInset = 18;
constexpr int kTickMajorLength = 12;
constexpr int kTickMinorLength = 6;
constexpr int kFooterHeight = 80;
constexpr int kMargin = 20;
}  // namespace

PomodoroActivity::PomodoroActivity(GfxRenderer& renderer, MappedInputManager& mappedInput,
                                   const std::function<void()>& onGoHome)
    : Activity("Pomodoro", renderer, mappedInput), onGoHome(onGoHome) {}

void PomodoroActivity::onEnter() {
  Activity::onEnter();

  const int storedMinutes = APP_STATE.pomodoroMinutes;
  if (storedMinutes >= PomodoroModel::kMinMinutes && storedMinutes <= PomodoroModel::kMaxMinutes) {
    model.setDurationMinutes(storedMinutes);
  } else {
    model.setDurationMinutes(25);
  }

  useGrayscale = SETTINGS.textAntiAliasing != 0;

  lastDisplayedMinutes = -1;
  lastDisplayedSecondsBucket = -1;
  lastShowSeconds = false;
  lastResetPromptActive = false;

  requestRender(EInkDisplay::FULL_REFRESH);
}

void PomodoroActivity::onExit() { Activity::onExit(); }

bool PomodoroActivity::preventAutoSleep() { return flashActive || model.isRunning(); }

void PomodoroActivity::requestRender(const EInkDisplay::RefreshMode mode) {
  renderRequired = true;
  if (mode == EInkDisplay::FULL_REFRESH) {
    pendingRefresh = mode;
  } else if (pendingRefresh != EInkDisplay::FULL_REFRESH) {
    pendingRefresh = mode;
  }
}

void PomodoroActivity::setTransientStatus(const char* message, const unsigned long durationMs) {
  transientStatus = message;
  transientStatusUntilMs = millis() + durationMs;
  requestRender(EInkDisplay::FAST_REFRESH);
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

void PomodoroActivity::persistDuration() {
  APP_STATE.pomodoroMinutes = static_cast<uint8_t>(model.getDurationMinutes());
  APP_STATE.saveToFile();
}

void PomodoroActivity::loop() {
  const unsigned long nowMs = millis();

  if (flashActive) {
    if (mappedInput.wasAnyPressed() || mappedInput.wasAnyReleased()) {
      flashActive = false;
      requestRender(EInkDisplay::FULL_REFRESH);
      return;
    }

    if (nowMs >= nextFlashMs) {
      renderer.invertScreen();
      renderer.displayBuffer(EInkDisplay::FAST_REFRESH);
      flashRemainingToggles--;
      nextFlashMs = nowMs + kFlashIntervalMs;
      if (flashRemainingToggles <= 0) {
        flashActive = false;
        requestRender(EInkDisplay::FULL_REFRESH);
      }
    }
    return;
  }

  if (mappedInput.isPressed(MappedInputManager::Button::Back) && mappedInput.getHeldTime() >= kQuitHoldMs) {
    onGoHome();
    return;
  }

  const PomodoroState prevState = model.getState();

  const bool confirmReleased = mappedInput.wasReleased(MappedInputManager::Button::Confirm);
  const bool backReleased = mappedInput.wasReleased(MappedInputManager::Button::Back);
  const bool upReleased = mappedInput.wasReleased(MappedInputManager::Button::Up);
  const bool downReleased = mappedInput.wasReleased(MappedInputManager::Button::Down);

  if (confirmReleased) {
    if (model.getState() == PomodoroState::Running) {
      model.pause();
      requestRender(EInkDisplay::FULL_REFRESH);
    } else if (model.getState() == PomodoroState::Paused) {
      model.resume(nowMs);
      requestRender(EInkDisplay::FULL_REFRESH);
    } else {
      model.start(nowMs);
      requestRender(EInkDisplay::FULL_REFRESH);
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
      persistDuration();
      requestRender(EInkDisplay::FAST_REFRESH);
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
      setTransientStatus("Reset", kStatusMessageMs);
      requestRender(EInkDisplay::FULL_REFRESH);
    } else if (model.getState() == PomodoroState::Running || model.getState() == PomodoroState::Paused) {
      requestRender(EInkDisplay::FAST_REFRESH);
    }
  }

  if (model.tick(nowMs)) {
    flashActive = true;
    flashRemainingToggles = kFlashCycles * 2;
    nextFlashMs = nowMs;
    return;
  }

  if (!transientStatus.empty() && nowMs >= transientStatusUntilMs) {
    transientStatus.clear();
    requestRender(EInkDisplay::FAST_REFRESH);
  }

  const bool resetPromptActive = model.isResetPromptActive(nowMs);
  if (resetPromptActive != lastResetPromptActive) {
    lastResetPromptActive = resetPromptActive;
    requestRender(EInkDisplay::FAST_REFRESH);
  }

  const bool showSeconds = model.shouldShowSeconds();
  if (showSeconds) {
    const int secondsBucket = model.getDisplaySecondsBucket();
    if (!lastShowSeconds || secondsBucket != lastDisplayedSecondsBucket) {
      requestRender(EInkDisplay::FAST_REFRESH);
    }
  } else {
    const int minutes = model.getDisplayMinutes();
    if (lastShowSeconds || minutes != lastDisplayedMinutes) {
      requestRender(EInkDisplay::FAST_REFRESH);
    }
  }

  if (model.getState() != prevState) {
    requestRender(EInkDisplay::FULL_REFRESH);
  }

  if (renderRequired) {
    render(pendingRefresh);
    pendingRefresh = EInkDisplay::FAST_REFRESH;
    renderRequired = false;
  }
}

void PomodoroActivity::render(const EInkDisplay::RefreshMode mode) {
  renderer.clearScreen();

  const int pageWidth = renderer.getScreenWidth();
  const int pageHeight = renderer.getScreenHeight();

  const int headerHeight = renderer.getLineHeight(UI_12_FONT_ID) + 12;
  renderer.drawCenteredText(UI_12_FONT_ID, 10, "POMODORO", true, EpdFontFamily::BOLD);

  const int availableHeight = pageHeight - headerHeight - kFooterHeight;
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
  const float fraction = std::min(1.0f, std::max(0.0f, (showSeconds ? 60.0f : displayValue * 60.0f) / 3600.0f));
  const float pointerAngle = kStartAngle + fraction * kTwoPi;

  renderDial(centerX, centerY, outerRadius, innerRadius, fraction, !useGrayscale);
  renderDialTicks(centerX, centerY, outerRadius - 2, true);
  renderDialNumerals(centerX, centerY, numberRadius, true);
  if (fraction > 0.0f) {
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
  const int statusY = pageHeight - kFooterHeight + 10;
  renderer.drawCenteredText(UI_10_FONT_ID, statusY, statusText.c_str());

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
      prevLabel = "--";
      nextLabel = "--";
      break;
    case PomodoroState::Paused:
      confirmLabel = "Resume";
      prevLabel = "-5";
      nextLabel = "+5";
      break;
  }

  const auto labels = mappedInput.mapLabels(backLabel, confirmLabel, prevLabel, nextLabel);
  renderer.drawButtonHints(UI_10_FONT_ID, labels.btn1, labels.btn2, labels.btn3, labels.btn4);

  renderer.displayBuffer(mode);

  if (useGrayscale && fraction > 0.0f) {
    applyGrayscaleWedgeMask(centerX, centerY, outerRadius, innerRadius, numberRadius, fraction, pointerAngle);
  }

  lastShowSeconds = showSeconds;
  lastDisplayedMinutes = model.getDisplayMinutes();
  lastDisplayedSecondsBucket = model.getDisplaySecondsBucket();
}

void PomodoroActivity::renderDial(const int centerX, const int centerY, const int outerRadius, const int innerRadius,
                                  const float fraction, const bool dither) {
  if (fraction <= 0.0f) {
    return;
  }
  if (dither) {
    drawWedgeDithered(centerX, centerY, innerRadius, outerRadius, fraction);
  } else {
    drawWedgeSolid(centerX, centerY, innerRadius, outerRadius, fraction, true);
  }
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
    const std::string label = std::to_string(minutes);
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
  const int points = 12;
  int xPoints[points];
  int yPoints[points];
  for (int i = 0; i < points; i++) {
    const float angle = (static_cast<float>(i) / points) * kTwoPi;
    xPoints[i] = centerX + static_cast<int>(std::round(std::cos(angle) * kHubRadius));
    yPoints[i] = centerY + static_cast<int>(std::round(std::sin(angle) * kHubRadius));
  }
  renderer.fillPolygon(xPoints, yPoints, points, state);
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

void PomodoroActivity::applyGrayscaleWedgeMask(const int centerX, const int centerY, const int outerRadius,
                                               const int innerRadius, const int numberRadius, const float fraction,
                                               const float pointerAngle) {
  renderer.storeBwBuffer();

  renderer.clearScreen(0x00);
  renderer.setRenderMode(GfxRenderer::GRAYSCALE_LSB);
  renderer.copyGrayscaleLsbBuffers();

  renderer.clearScreen(0x00);
  renderer.setRenderMode(GfxRenderer::GRAYSCALE_MSB);
  drawWedgeSolid(centerX, centerY, innerRadius, outerRadius, fraction, false);
  renderDialTicks(centerX, centerY, outerRadius - 2, true);
  renderDialNumerals(centerX, centerY, numberRadius, true);
  renderPointer(centerX, centerY, outerRadius, pointerAngle, true);
  renderCenterHub(centerX, centerY, true);
  renderer.copyGrayscaleMsbBuffers();

  renderer.displayGrayBuffer();
  renderer.setRenderMode(GfxRenderer::BW);
  renderer.restoreBwBuffer();
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
    if (((r + phase) & 1) == 0) {
      const int x = centerX + static_cast<int>(std::round(cosA * r));
      const int y = centerY + static_cast<int>(std::round(sinA * r));
      renderer.drawPixel(x, y, true);
    }
  }
}

void PomodoroActivity::drawWedgeSolid(const int centerX, const int centerY, const int innerRadius,
                                      const int outerRadius, const float fraction, const bool state) {
  if (fraction <= 0.0f) {
    return;
  }
  const float sweep = std::min(1.0f, fraction) * kTwoPi;
  const int steps = std::max(1, static_cast<int>(std::round(sweep * 180.0f / kPi)));
  for (int i = 0; i <= steps; i++) {
    const float angle = kStartAngle + sweep * (static_cast<float>(i) / steps);
    const float cosA = std::cos(angle);
    const float sinA = std::sin(angle);
    drawRadialLine(centerX, centerY, cosA, sinA, innerRadius, outerRadius, state);
  }
}

void PomodoroActivity::drawWedgeDithered(const int centerX, const int centerY, const int innerRadius,
                                         const int outerRadius, const float fraction) {
  if (fraction <= 0.0f) {
    return;
  }
  const float sweep = std::min(1.0f, fraction) * kTwoPi;
  const int steps = std::max(1, static_cast<int>(std::round(sweep * 180.0f / kPi)));
  for (int i = 0; i <= steps; i++) {
    const float angle = kStartAngle + sweep * (static_cast<float>(i) / steps);
    const float cosA = std::cos(angle);
    const float sinA = std::sin(angle);
    drawDitheredRadialLine(centerX, centerY, cosA, sinA, innerRadius, outerRadius, i);
  }
}
