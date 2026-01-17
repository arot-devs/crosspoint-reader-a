#include "NewsFeedActivity.h"

#include <GfxRenderer.h>
#include <HardwareSerial.h>
#include <WiFi.h>

#include "MappedInputManager.h"
#include <RssParser.h>
#include "fontIds.h"
#include "network/HttpDownloader.h"
#include "util/HtmlUtils.h"
#include "util/StringUtils.h"

namespace {
constexpr char FEED_URL[] = "https://www.dataengineeringweekly.com/feed";
constexpr char FEED_TITLE[] = "Data Eng Weekly";
constexpr int PAGE_ITEMS = 23;
constexpr int SKIP_PAGE_MS = 700;
constexpr size_t MAX_ITEMS = 30;
constexpr int TITLE_MAX_LINES = 3;
constexpr int TOP_MARGIN = 20;
constexpr int BOTTOM_MARGIN = 40;
constexpr int BODY_SPACING = 6;

int clampInt(const int value, const int minValue, const int maxValue) {
  if (value < minValue) {
    return minValue;
  }
  if (value > maxValue) {
    return maxValue;
  }
  return value;
}
}  // namespace

void NewsFeedActivity::taskTrampoline(void* param) {
  auto* self = static_cast<NewsFeedActivity*>(param);
  self->displayTaskLoop();
}

void NewsFeedActivity::onEnter() {
  ActivityWithSubactivity::onEnter();

  renderingMutex = xSemaphoreCreateMutex();
  state = NewsState::CHECK_WIFI;
  items.clear();
  selectorIndex = 0;
  currentItemIndex = -1;
  detailScroll = 0;
  detailTitleLines.clear();
  detailBodyLines.clear();
  detailDate.clear();
  errorMessage.clear();
  statusMessage = "Checking WiFi...";
  updateRequired = true;

  xTaskCreate(&NewsFeedActivity::taskTrampoline, "NewsFeedTask",
              4096,               // Stack size
              this,               // Parameters
              1,                  // Priority
              &displayTaskHandle  // Task handle
  );

  checkAndConnectWifi();
}

void NewsFeedActivity::onExit() {
  ActivityWithSubactivity::onExit();

  WiFi.mode(WIFI_OFF);

  xSemaphoreTake(renderingMutex, portMAX_DELAY);
  if (displayTaskHandle) {
    vTaskDelete(displayTaskHandle);
    displayTaskHandle = nullptr;
  }
  vSemaphoreDelete(renderingMutex);
  renderingMutex = nullptr;
  items.clear();
  detailTitleLines.clear();
  detailBodyLines.clear();
}

void NewsFeedActivity::loop() {
  if (state == NewsState::WIFI_SELECTION) {
    ActivityWithSubactivity::loop();
    return;
  }

  if (state == NewsState::ERROR) {
    if (mappedInput.wasReleased(MappedInputManager::Button::Confirm)) {
      checkAndConnectWifi();
    } else if (mappedInput.wasReleased(MappedInputManager::Button::Back)) {
      onGoHome();
    }
    return;
  }

  if (state == NewsState::CHECK_WIFI || state == NewsState::LOADING) {
    if (mappedInput.wasReleased(MappedInputManager::Button::Back)) {
      onGoHome();
    }
    return;
  }

  if (state == NewsState::LIST) {
    const bool prevReleased = mappedInput.wasReleased(MappedInputManager::Button::Up) ||
                              mappedInput.wasReleased(MappedInputManager::Button::Left);
    const bool nextReleased = mappedInput.wasReleased(MappedInputManager::Button::Down) ||
                              mappedInput.wasReleased(MappedInputManager::Button::Right);
    const bool skipPage = mappedInput.getHeldTime() > SKIP_PAGE_MS;

    if (mappedInput.wasReleased(MappedInputManager::Button::Confirm)) {
      if (!items.empty()) {
        openDetail(selectorIndex);
      }
    } else if (mappedInput.wasReleased(MappedInputManager::Button::Back)) {
      onGoHome();
    } else if (prevReleased && !items.empty()) {
      if (skipPage) {
        selectorIndex = ((selectorIndex / PAGE_ITEMS - 1) * PAGE_ITEMS + items.size()) % items.size();
      } else {
        selectorIndex = (selectorIndex + items.size() - 1) % items.size();
      }
      updateRequired = true;
    } else if (nextReleased && !items.empty()) {
      if (skipPage) {
        selectorIndex = ((selectorIndex / PAGE_ITEMS + 1) * PAGE_ITEMS) % items.size();
      } else {
        selectorIndex = (selectorIndex + 1) % items.size();
      }
      updateRequired = true;
    }
    return;
  }

  if (state == NewsState::VIEW) {
    if (mappedInput.wasReleased(MappedInputManager::Button::Back)) {
      closeDetail();
      return;
    }

    if (mappedInput.wasReleased(MappedInputManager::Button::Up) ||
        mappedInput.wasReleased(MappedInputManager::Button::Left)) {
      updateDetailScroll(-1);
    } else if (mappedInput.wasReleased(MappedInputManager::Button::Down) ||
               mappedInput.wasReleased(MappedInputManager::Button::Right)) {
      updateDetailScroll(1);
    } else if (mappedInput.wasReleased(MappedInputManager::Button::PageBack)) {
      updateDetailScrollPage(-1);
    } else if (mappedInput.wasReleased(MappedInputManager::Button::PageForward)) {
      updateDetailScrollPage(1);
    }
  }
}

void NewsFeedActivity::displayTaskLoop() {
  while (true) {
    if (updateRequired) {
      updateRequired = false;
      xSemaphoreTake(renderingMutex, portMAX_DELAY);
      render();
      xSemaphoreGive(renderingMutex);
    }
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}

void NewsFeedActivity::render() const {
  renderer.clearScreen();

  if (state == NewsState::ERROR) {
    renderError();
    renderer.displayBuffer();
    return;
  }

  if (state == NewsState::VIEW) {
    renderDetail();
    renderer.displayBuffer();
    return;
  }

  renderList();
  renderer.displayBuffer();
}

void NewsFeedActivity::renderList() const {
  const auto pageWidth = renderer.getScreenWidth();
  const auto pageHeight = renderer.getScreenHeight();

  renderer.drawCenteredText(UI_12_FONT_ID, 15, FEED_TITLE, true, EpdFontFamily::BOLD);

  if (state == NewsState::CHECK_WIFI || state == NewsState::LOADING) {
    renderer.drawCenteredText(UI_10_FONT_ID, pageHeight / 2, statusMessage.c_str());
    const auto labels = mappedInput.mapLabels("« Back", "", "", "");
    renderer.drawButtonHints(UI_10_FONT_ID, labels.btn1, labels.btn2, labels.btn3, labels.btn4);
    return;
  }

  if (items.empty()) {
    renderer.drawCenteredText(UI_10_FONT_ID, pageHeight / 2, "No items found");
    const auto labels = mappedInput.mapLabels("« Back", "", "", "");
    renderer.drawButtonHints(UI_10_FONT_ID, labels.btn1, labels.btn2, labels.btn3, labels.btn4);
    return;
  }

  const auto pageStartIndex = selectorIndex / PAGE_ITEMS * PAGE_ITEMS;
  renderer.fillRect(0, 60 + (selectorIndex % PAGE_ITEMS) * 30 - 2, pageWidth - 1, 30);

  for (size_t i = pageStartIndex; i < items.size() && i < static_cast<size_t>(pageStartIndex + PAGE_ITEMS); i++) {
    const auto& item = items[i];
    auto title = renderer.truncatedText(UI_10_FONT_ID, item.title.c_str(), pageWidth - 40);
    renderer.drawText(UI_10_FONT_ID, 20, 60 + (i % PAGE_ITEMS) * 30, title.c_str(), i != selectorIndex);
  }

  const auto labels = mappedInput.mapLabels("« Back", "Open", "Up", "Down");
  renderer.drawButtonHints(UI_10_FONT_ID, labels.btn1, labels.btn2, labels.btn3, labels.btn4);
}

void NewsFeedActivity::renderDetail() const {
  const auto pageWidth = renderer.getScreenWidth();
  const auto pageHeight = renderer.getScreenHeight();
  const int maxWidth = pageWidth - TOP_MARGIN * 2;

  int y = TOP_MARGIN;
  const int titleLineHeight = renderer.getLineHeight(UI_12_FONT_ID);
  for (const auto& line : detailTitleLines) {
    renderer.drawText(UI_12_FONT_ID, TOP_MARGIN, y, line.c_str());
    y += titleLineHeight;
  }

  if (!detailDate.empty()) {
    const auto dateText = renderer.truncatedText(UI_10_FONT_ID, detailDate.c_str(), maxWidth);
    renderer.drawText(UI_10_FONT_ID, TOP_MARGIN, y, dateText.c_str());
    y += renderer.getLineHeight(UI_10_FONT_ID);
  }

  y += BODY_SPACING;

  const int bodyLineHeight = renderer.getLineHeight(UI_10_FONT_ID);
  int maxBodyLines = (pageHeight - BOTTOM_MARGIN - y) / bodyLineHeight;
  if (maxBodyLines < 1) {
    maxBodyLines = 1;
  }

  const int startIndex = detailScroll;
  for (int i = 0; i < maxBodyLines && startIndex + i < static_cast<int>(detailBodyLines.size()); ++i) {
    renderer.drawText(UI_10_FONT_ID, TOP_MARGIN, y + i * bodyLineHeight, detailBodyLines[startIndex + i].c_str());
  }

  const auto labels = mappedInput.mapLabels("« Back", "", "Up", "Down");
  renderer.drawButtonHints(UI_10_FONT_ID, labels.btn1, labels.btn2, labels.btn3, labels.btn4);
}

void NewsFeedActivity::renderError() const {
  const auto pageHeight = renderer.getScreenHeight();

  renderer.drawCenteredText(UI_10_FONT_ID, pageHeight / 2 - 20, "Error:");
  renderer.drawCenteredText(UI_10_FONT_ID, pageHeight / 2 + 10, errorMessage.c_str());
  const auto labels = mappedInput.mapLabels("« Back", "Retry", "", "");
  renderer.drawButtonHints(UI_10_FONT_ID, labels.btn1, labels.btn2, labels.btn3, labels.btn4);
}

void NewsFeedActivity::checkAndConnectWifi() {
  if (WiFi.status() == WL_CONNECTED && WiFi.localIP() != IPAddress(0, 0, 0, 0)) {
    state = NewsState::LOADING;
    statusMessage = "Loading...";
    updateRequired = true;
    fetchFeed();
    return;
  }

  launchWifiSelection();
}

void NewsFeedActivity::launchWifiSelection() {
  state = NewsState::WIFI_SELECTION;
  updateRequired = true;

  enterNewActivity(new WifiSelectionActivity(renderer, mappedInput,
                                             [this](const bool connected) { onWifiSelectionComplete(connected); }));
}

void NewsFeedActivity::onWifiSelectionComplete(const bool connected) {
  exitActivity();

  if (connected) {
    Serial.printf("[%lu] [NEWS] WiFi connected via selection, fetching feed\n", millis());
    state = NewsState::LOADING;
    statusMessage = "Loading...";
    updateRequired = true;
    fetchFeed();
  } else {
    Serial.printf("[%lu] [NEWS] WiFi selection cancelled/failed\n", millis());
    WiFi.disconnect();
    WiFi.mode(WIFI_OFF);
    state = NewsState::ERROR;
    errorMessage = "WiFi connection failed";
    updateRequired = true;
  }
}

void NewsFeedActivity::fetchFeed() {
  std::string content;
  if (!HttpDownloader::fetchUrl(FEED_URL, content)) {
    state = NewsState::ERROR;
    errorMessage = "Failed to fetch feed";
    updateRequired = true;
    return;
  }

  RssParser parser;
  if (!parser.parse(content.c_str(), content.size())) {
    state = NewsState::ERROR;
    errorMessage = "Failed to parse feed";
    updateRequired = true;
    return;
  }

  items = parser.getItems();
  if (items.size() > MAX_ITEMS) {
    items.resize(MAX_ITEMS);
  }

  for (auto& item : items) {
    item.title = HtmlUtils::toPlainText(item.title);
    for (auto& ch : item.title) {
      if (ch == '\n') {
        ch = ' ';
      }
    }
    if (!item.pubDate.empty()) {
      item.pubDate = HtmlUtils::toPlainText(item.pubDate);
    }
  }

  if (items.empty()) {
    state = NewsState::ERROR;
    errorMessage = "No items found";
    updateRequired = true;
    return;
  }

  selectorIndex = 0;
  state = NewsState::LIST;
  updateRequired = true;
}

void NewsFeedActivity::openDetail(const int index) {
  if (index < 0 || index >= static_cast<int>(items.size())) {
    return;
  }

  currentItemIndex = index;
  buildDetailLines(items[index]);
  detailScroll = 0;
  state = NewsState::VIEW;
  updateRequired = true;
}

void NewsFeedActivity::closeDetail() {
  state = NewsState::LIST;
  updateRequired = true;
}

void NewsFeedActivity::updateDetailScroll(const int delta) {
  const int maxScroll = getDetailMaxScroll();
  const int nextScroll = clampInt(detailScroll + delta, 0, maxScroll);
  if (nextScroll != detailScroll) {
    detailScroll = nextScroll;
    updateRequired = true;
  }
}

void NewsFeedActivity::updateDetailScrollPage(const int deltaPages) {
  const int maxScroll = getDetailMaxScroll();
  const int pageHeight = renderer.getScreenHeight();
  const int titleHeight = renderer.getLineHeight(UI_12_FONT_ID) * static_cast<int>(detailTitleLines.size());
  int y = TOP_MARGIN + titleHeight;
  if (!detailDate.empty()) {
    y += renderer.getLineHeight(UI_10_FONT_ID);
  }
  y += BODY_SPACING;

  const int bodyLineHeight = renderer.getLineHeight(UI_10_FONT_ID);
  int maxBodyLines = (pageHeight - BOTTOM_MARGIN - y) / bodyLineHeight;
  if (maxBodyLines < 1) {
    maxBodyLines = 1;
  }

  const int delta = maxBodyLines * deltaPages;
  const int nextScroll = clampInt(detailScroll + delta, 0, maxScroll);
  if (nextScroll != detailScroll) {
    detailScroll = nextScroll;
    updateRequired = true;
  }
}

int NewsFeedActivity::getDetailMaxScroll() const {
  if (detailBodyLines.empty()) {
    return 0;
  }

  const int pageHeight = renderer.getScreenHeight();
  const int titleHeight = renderer.getLineHeight(UI_12_FONT_ID) * static_cast<int>(detailTitleLines.size());
  int y = TOP_MARGIN + titleHeight;
  if (!detailDate.empty()) {
    y += renderer.getLineHeight(UI_10_FONT_ID);
  }
  y += BODY_SPACING;

  const int bodyLineHeight = renderer.getLineHeight(UI_10_FONT_ID);
  int maxBodyLines = (pageHeight - BOTTOM_MARGIN - y) / bodyLineHeight;
  if (maxBodyLines < 1) {
    maxBodyLines = 1;
  }

  const int totalLines = static_cast<int>(detailBodyLines.size());
  return totalLines > maxBodyLines ? totalLines - maxBodyLines : 0;
}

void NewsFeedActivity::buildDetailLines(const RssItem& item) {
  const int pageWidth = renderer.getScreenWidth();
  const int maxWidth = pageWidth - TOP_MARGIN * 2;

  detailTitleLines = wrapText(item.title, UI_12_FONT_ID, maxWidth, TITLE_MAX_LINES);
  detailDate = item.pubDate;

  std::string summary = item.summary.empty() ? item.content : item.summary;
  if (summary.empty()) {
    summary = "No summary available.";
  }

  const std::string plainText = HtmlUtils::toPlainText(summary);
  detailBodyLines = wrapText(plainText, UI_10_FONT_ID, maxWidth, 0);
}

std::vector<std::string> NewsFeedActivity::wrapText(const std::string& text, const int fontId, const int maxWidth,
                                                    const int maxLines) const {
  std::vector<std::string> lines;
  bool truncated = false;

  std::string normalized = text;
  for (auto& ch : normalized) {
    if (ch == '\r') {
      ch = '\n';
    }
  }

  const int spaceWidth = renderer.getSpaceWidth(fontId);
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

  if (maxLines > 0 && lines.size() > static_cast<size_t>(maxLines)) {
    truncated = true;
    lines.resize(static_cast<size_t>(maxLines));
  }

  if (maxLines > 0 && truncated && !lines.empty()) {
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

  return lines;
}
