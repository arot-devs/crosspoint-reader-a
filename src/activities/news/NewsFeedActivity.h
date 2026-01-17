#pragma once
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>

#include <functional>
#include <string>
#include <vector>

#include <RssParser.h>

#include "activities/ActivityWithSubactivity.h"
#include "activities/network/WifiSelectionActivity.h"

class NewsFeedActivity final : public ActivityWithSubactivity {
 public:
  enum class NewsState {
    CHECK_WIFI,
    WIFI_SELECTION,
    LOADING,
    LIST,
    VIEW,
    ERROR
  };

  explicit NewsFeedActivity(GfxRenderer& renderer, MappedInputManager& mappedInput,
                            const std::function<void()>& onGoHome)
      : ActivityWithSubactivity("NewsFeed", renderer, mappedInput), onGoHome(onGoHome) {}

  void onEnter() override;
  void onExit() override;
  void loop() override;

 private:
  TaskHandle_t displayTaskHandle = nullptr;
  SemaphoreHandle_t renderingMutex = nullptr;
  bool updateRequired = false;

  NewsState state = NewsState::CHECK_WIFI;
  std::vector<RssItem> items;
  int selectorIndex = 0;
  int currentItemIndex = -1;
  int detailScroll = 0;
  std::vector<std::string> detailTitleLines;
  std::vector<std::string> detailBodyLines;
  std::string detailDate;
  std::string errorMessage;
  std::string statusMessage;

  const std::function<void()> onGoHome;

  static void taskTrampoline(void* param);
  [[noreturn]] void displayTaskLoop();
  void render() const;
  void renderList() const;
  void renderDetail() const;
  void renderError() const;

  void checkAndConnectWifi();
  void launchWifiSelection();
  void onWifiSelectionComplete(bool connected);
  void fetchFeed();

  void openDetail(int index);
  void closeDetail();
  void updateDetailScroll(int delta);
  void updateDetailScrollPage(int deltaPages);
  int getDetailMaxScroll() const;
  void buildDetailLines(const RssItem& item);
  std::vector<std::string> wrapText(const std::string& text, int fontId, int maxWidth, int maxLines) const;
};
