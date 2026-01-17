#pragma once

#include <EInkDisplay.h>

#include <functional>
#include <string>

#include "../Activity.h"
#include "PomodoroModel.h"

class PomodoroActivity final : public Activity {
 public:
  explicit PomodoroActivity(GfxRenderer& renderer, MappedInputManager& mappedInput,
                            const std::function<void()>& onGoHome);
  void onEnter() override;
  void onExit() override;
  void loop() override;
  bool preventAutoSleep() override;

 private:
  struct PomodoroStats {
    int todayCount = 0;
    int weekCount = 0;
    float todayHours = 0.0f;
    float weekHours = 0.0f;
    bool timeValid = false;
  };

  PomodoroModel model;
  const std::function<void()> onGoHome;

  bool renderRequired = false;
  EInkDisplay::RefreshMode pendingRefresh = EInkDisplay::FAST_REFRESH;

  int lastDisplayedMinutes = -1;
  int lastDisplayedSecondsBucket = -1;
  bool lastShowSeconds = false;
  bool lastResetPromptActive = false;

  bool flashActive = false;
  int flashRemainingToggles = 0;
  unsigned long nextFlashMs = 0;

  PomodoroStats stats;
  uint32_t sessionStartEpoch = 0;

  std::string transientStatus;
  unsigned long transientStatusUntilMs = 0;

  void requestRender(EInkDisplay::RefreshMode mode);
  void setTransientStatus(const char* message, unsigned long durationMs);
  std::string buildStatusText(unsigned long nowMs);
  void persistDuration();
  void loadStats();
  void logPomodoroCompletion(uint32_t endEpochSeconds);
  static uint32_t getNowEpochSeconds();
  static bool isEpochValid(uint32_t epochSeconds);
  static std::string formatHours(float hours);

  void render(EInkDisplay::RefreshMode mode);
  void renderDial(int centerX, int centerY, int outerRadius, int innerRadius, float sweepAngle);
  void renderDialTicks(int centerX, int centerY, int outerRadius, bool state);
  void renderDialNumerals(int centerX, int centerY, int numberRadius, bool state);
  void renderPointer(int centerX, int centerY, int outerRadius, float angle, bool state);
  void renderCenterHub(int centerX, int centerY, bool state);
  void renderCenterReadout(int centerX, int centerY, bool showSeconds, int displayValue);
  void drawRadialLine(int centerX, int centerY, float cosA, float sinA, int innerRadius, int outerRadius, bool state);
  void drawDitheredRadialLine(int centerX, int centerY, float cosA, float sinA, int innerRadius, int outerRadius,
                              int phase);
  void drawWedgeSolid(int centerX, int centerY, int innerRadius, int outerRadius, float sweepAngle, bool state);
  void drawWedgeDithered(int centerX, int centerY, int innerRadius, int outerRadius, float sweepAngle);
};
