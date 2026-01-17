#pragma once

#include <EInkDisplay.h>

#include <functional>
#include <string>

#include "../Activity.h"

class InfoBoardActivity final : public Activity {
  static constexpr size_t MAX_PAYLOAD_SIZE = 2048;

  const std::function<void()> onGoHome;
  char buffer[MAX_PAYLOAD_SIZE + 1] = {0};
  size_t bufferLen = 0;
  bool discarding = false;
  std::string lastMessage;

  void renderMessage(const std::string& message, EInkDisplay::RefreshMode refreshMode);
  void renderStatus(const char* message);
  void handleSerialInput();

 public:
  explicit InfoBoardActivity(GfxRenderer& renderer, MappedInputManager& mappedInput,
                             const std::function<void()>& onGoHome)
      : Activity("InfoBoard", renderer, mappedInput), onGoHome(onGoHome) {}

  void onEnter() override;
  void loop() override;
  bool preventAutoSleep() override { return true; }
};
