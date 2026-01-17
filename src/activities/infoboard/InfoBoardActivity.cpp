#include "InfoBoardActivity.h"

#include <Arduino.h>
#include <ArduinoJson.h>
#include <GfxRenderer.h>

#include <deque>
#include <cstring>
#include <vector>

#include "MappedInputManager.h"
#include "fontIds.h"
#include "util/StringUtils.h"

namespace {
std::string decodeEscapedNewlines(const std::string& message) {
  std::string decoded;
  decoded.reserve(message.size());
  for (size_t i = 0; i < message.size(); ++i) {
    if (message[i] == '\\' && i + 1 < message.size() && message[i + 1] == 'n') {
      decoded.push_back('\n');
      ++i;
    } else {
      decoded.push_back(message[i]);
    }
  }
  return decoded;
}

}  // namespace

void InfoBoardActivity::onEnter() {
  Activity::onEnter();

  if (!Serial) {
    Serial.begin(115200);
  }

  bufferLen = 0;
  discarding = false;
  lastMessage.clear();

  renderStatus("InfoBoard ready");
}

void InfoBoardActivity::loop() {
  if (mappedInput.wasReleased(MappedInputManager::Button::Back)) {
    sendButtonEvent("back");
    onGoHome();
    return;
  }
  if (mappedInput.wasReleased(MappedInputManager::Button::Confirm)) {
    sendButtonEvent("confirm");
  }
  if (mappedInput.wasReleased(MappedInputManager::Button::Up)) {
    sendButtonEvent("up");
  }
  if (mappedInput.wasReleased(MappedInputManager::Button::Down)) {
    sendButtonEvent("down");
  }
  if (mappedInput.wasReleased(MappedInputManager::Button::Left)) {
    sendButtonEvent("left");
  }
  if (mappedInput.wasReleased(MappedInputManager::Button::Right)) {
    sendButtonEvent("right");
  }
  if (mappedInput.wasReleased(MappedInputManager::Button::PageBack)) {
    sendButtonEvent("page_back");
  }
  if (mappedInput.wasReleased(MappedInputManager::Button::PageForward)) {
    sendButtonEvent("page_forward");
  }

  handleSerialInput();
}

void InfoBoardActivity::sendButtonEvent(const char* button) {
  if (!Serial) {
    return;
  }
  Serial.printf("IBEV {\"v\":1,\"event\":\"button\",\"button\":\"%s\",\"action\":\"release\"}\n", button);
}

void InfoBoardActivity::renderStatus(const char* message) {
  renderCenteredMessage(message, EInkDisplay::FAST_REFRESH, false);
}

void InfoBoardActivity::handleSerialInput() {
  if (!Serial) {
    return;
  }

  while (Serial.available() > 0) {
    const int value = Serial.read();
    if (value < 0) {
      break;
    }

    const char ch = static_cast<char>(value);
    if (ch == '\r') {
      continue;
    }

    if (ch == '\n') {
      if (discarding) {
        discarding = false;
        bufferLen = 0;
        continue;
      }

      buffer[bufferLen] = '\0';
      std::string message(buffer, bufferLen);
      bufferLen = 0;

      bool append = false;
      bool finalRender = true;
      bool clear = false;
      RenderMode mode = RenderMode::Centered;
      std::string text = message;
      const bool isJson = tryParseJsonMessage(message, text, mode, append, finalRender, clear);

      if (!isJson && !message.empty() && message.front() == '{') {
        Serial.printf("[%lu] [INFO] Dropping invalid JSON payload\n", millis());
        continue;
      }

      if (!isJson && message == lastMessage) {
        continue;
      }
      lastMessage = message;

      if (mode == RenderMode::Console) {
        if (clear) {
          consoleBuffer.clear();
        }
        if (append) {
          consoleBuffer.append(text);
        } else if (!text.empty() || !clear) {
          consoleBuffer = text;
        }

        if (consoleBuffer.size() > MAX_CONSOLE_BUFFER) {
          size_t start = consoleBuffer.size() - MAX_CONSOLE_BUFFER;
          const size_t newline = consoleBuffer.find('\n', start);
          if (newline != std::string::npos && newline + 1 < consoleBuffer.size()) {
            start = newline + 1;
          }
          consoleBuffer.erase(0, start);
        }

        if (finalRender) {
          renderConsoleMessage(consoleBuffer, EInkDisplay::FAST_REFRESH);
        }
      } else {
        renderCenteredMessage(text, EInkDisplay::FAST_REFRESH, !isJson);
      }
      continue;
    }

    if (discarding) {
      continue;
    }

    if (bufferLen < MAX_PAYLOAD_SIZE) {
      buffer[bufferLen++] = ch;
    } else {
      discarding = true;
      bufferLen = 0;
    }
  }
}

bool InfoBoardActivity::tryParseJsonMessage(const std::string& message, std::string& outText,
                                            InfoBoardActivity::RenderMode& outMode, bool& outAppend, bool& outFinal,
                                            bool& outClear) const {
  if (message.empty() || message.front() != '{') {
    return false;
  }

  JsonDocument doc;
  const auto error = deserializeJson(doc, message);
  if (error) {
    return false;
  }

  bool console = false;
  const char* mode = doc["mode"] | "";
  if (strcmp(mode, "console") == 0) {
    console = true;
  }
  if (doc["console"].is<bool>()) {
    console = doc["console"].as<bool>();
  }

  const char* text = doc["text"] | "";
  outText = text;
  outMode = console ? RenderMode::Console : RenderMode::Centered;

  outAppend = doc["append"].is<bool>() && doc["append"].as<bool>();
  outClear = doc["clear"].is<bool>() && doc["clear"].as<bool>();
  if (doc["final"].is<bool>()) {
    outFinal = doc["final"].as<bool>();
  } else {
    outFinal = !outAppend;
  }
  return true;
}

void InfoBoardActivity::renderCenteredMessage(const std::string& message, EInkDisplay::RefreshMode refreshMode,
                                              const bool decodeEscapes) {
  renderer.clearScreen();

  const int screenWidth = renderer.getScreenWidth();
  const int screenHeight = renderer.getScreenHeight();
  constexpr int margin = 20;
  constexpr int bottomMargin = 40;

  const int fontId = UI_12_FONT_ID;
  const int lineHeight = renderer.getLineHeight(fontId);
  const int maxWidth = screenWidth - margin * 2;
  const int maxHeight = screenHeight - margin - bottomMargin;
  int maxLines = maxHeight / lineHeight;
  if (maxLines < 1) {
    maxLines = 1;
  }

  std::vector<std::string> lines;
  lines.reserve(static_cast<size_t>(maxLines));

  const std::string normalized = decodeEscapes ? decodeEscapedNewlines(message) : message;
  size_t start = 0;
  while (start <= normalized.size()) {
    const size_t end = normalized.find('\n', start);
    const std::string segment = normalized.substr(start, end == std::string::npos ? std::string::npos : end - start);

    if (segment.empty()) {
      lines.emplace_back("");
    } else {
      std::vector<std::string> words;
      words.reserve(12);
      size_t pos = 0;
      while (pos < segment.size()) {
        while (pos < segment.size() && segment[pos] == ' ') {
          ++pos;
        }
        if (pos >= segment.size()) {
          break;
        }
        const size_t wordStart = pos;
        while (pos < segment.size() && segment[pos] != ' ') {
          ++pos;
        }
        words.emplace_back(segment.substr(wordStart, pos - wordStart));
      }

      std::string currentLine;
      const int spaceWidth = renderer.getSpaceWidth(fontId);
      for (auto& word : words) {
        int wordWidth = renderer.getTextWidth(fontId, word.c_str());
        while (wordWidth > maxWidth && !word.empty()) {
          StringUtils::utf8RemoveLastChar(word);
          const std::string withEllipsis = word + "...";
          wordWidth = renderer.getTextWidth(fontId, withEllipsis.c_str());
          if (wordWidth <= maxWidth) {
            word = withEllipsis;
            break;
          }
        }

        int lineWidth = renderer.getTextWidth(fontId, currentLine.c_str());
        int newLineWidth = wordWidth;
        if (!currentLine.empty()) {
          newLineWidth += spaceWidth + lineWidth;
        }

        if (newLineWidth > maxWidth && !currentLine.empty()) {
          lines.push_back(currentLine);
          currentLine = word;
        } else {
          if (!currentLine.empty()) {
            currentLine.append(" ");
          }
          currentLine.append(word);
        }
      }

      if (!currentLine.empty()) {
        lines.push_back(currentLine);
      }
    }

    if (end == std::string::npos) {
      break;
    }
    start = end + 1;
  }

  if (lines.size() > static_cast<size_t>(maxLines)) {
    lines.resize(static_cast<size_t>(maxLines));
    std::string& lastLine = lines.back();
    if (lastLine.empty()) {
      lastLine = "...";
    } else {
      lastLine.append("...");
      while (renderer.getTextWidth(fontId, lastLine.c_str()) > maxWidth && lastLine.size() > 3) {
        lastLine.resize(lastLine.size() - 3);
        StringUtils::utf8RemoveLastChar(lastLine);
        lastLine.append("...");
      }
      if (renderer.getTextWidth(fontId, lastLine.c_str()) > maxWidth) {
        lastLine = "...";
      }
    }
  }

  const int totalHeight = lineHeight * static_cast<int>(lines.size());
  int startY = margin + (maxHeight - totalHeight) / 2;

  for (const auto& line : lines) {
    renderer.drawCenteredText(fontId, startY, line.c_str());
    startY += lineHeight;
  }

  const auto labels = mappedInput.mapLabels("Back", "", "", "");
  renderer.drawButtonHints(UI_10_FONT_ID, labels.btn1, labels.btn2, labels.btn3, labels.btn4);

  renderer.displayBuffer(refreshMode);
}

void InfoBoardActivity::renderConsoleMessage(const std::string& message, EInkDisplay::RefreshMode refreshMode) {
  renderer.clearScreen();

  const int screenWidth = renderer.getScreenWidth();
  const int screenHeight = renderer.getScreenHeight();
  constexpr int margin = 20;
  constexpr int bottomMargin = 40;

  const int fontId = UI_12_FONT_ID;
  const int lineHeight = renderer.getLineHeight(fontId);
  const int maxWidth = screenWidth - margin * 2;
  const int maxHeight = screenHeight - margin - bottomMargin;
  int maxLines = maxHeight / lineHeight;
  if (maxLines < 1) {
    maxLines = 1;
  }

  std::deque<std::string> lines;

  std::string normalized = message;
  for (auto& ch : normalized) {
    if (ch == '\r') {
      ch = '\n';
    }
  }

  size_t start = 0;
  while (start <= normalized.size()) {
    const size_t end = normalized.find('\n', start);
    std::string line = normalized.substr(start, end == std::string::npos ? std::string::npos : end - start);

    if (line.empty()) {
      lines.emplace_back("");
      if (lines.size() > static_cast<size_t>(maxLines)) {
        lines.pop_front();
      }
    } else {
      std::string segment;
      segment.reserve(line.size());
      for (const char ch : line) {
        segment.push_back(ch);
        if (renderer.getTextWidth(fontId, segment.c_str()) > maxWidth) {
          segment.pop_back();
          if (!segment.empty()) {
            lines.push_back(segment);
            if (lines.size() > static_cast<size_t>(maxLines)) {
              lines.pop_front();
            }
            segment.clear();
          }
          segment.push_back(ch);
          if (renderer.getTextWidth(fontId, segment.c_str()) > maxWidth) {
            lines.push_back(segment);
            segment.clear();
            if (lines.size() > static_cast<size_t>(maxLines)) {
              lines.pop_front();
            }
          }
        }
      }
      if (!segment.empty()) {
        lines.push_back(segment);
        if (lines.size() > static_cast<size_t>(maxLines)) {
          lines.pop_front();
        }
      }
    }

    if (end == std::string::npos) {
      break;
    }
    start = end + 1;
  }

  int y = margin;
  for (const auto& line : lines) {
    renderer.drawText(fontId, margin, y, line.c_str());
    y += lineHeight;
    if (y + lineHeight > screenHeight - bottomMargin) {
      break;
    }
  }

  const auto labels = mappedInput.mapLabels("Back", "", "", "");
  renderer.drawButtonHints(UI_10_FONT_ID, labels.btn1, labels.btn2, labels.btn3, labels.btn4);

  renderer.displayBuffer(refreshMode);
}
