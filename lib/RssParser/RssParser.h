#pragma once
#include <expat.h>

#include <string>
#include <vector>

struct RssItem {
  std::string title;
  std::string link;
  std::string summary;
  std::string content;
  std::string pubDate;
};

class RssParser {
 public:
  RssParser() = default;
  ~RssParser();

  RssParser(const RssParser&) = delete;
  RssParser& operator=(const RssParser&) = delete;

  bool parse(const char* xmlData, size_t length);
  const std::vector<RssItem>& getItems() const { return items; }
  void clear();

 private:
  static void XMLCALL startElement(void* userData, const XML_Char* name, const XML_Char** atts);
  static void XMLCALL endElement(void* userData, const XML_Char* name);
  static void XMLCALL characterData(void* userData, const XML_Char* s, int len);

  static const char* findAttribute(const XML_Char** atts, const char* name);
  static bool matchesElement(const char* name, const char* localName);

  XML_Parser parser = nullptr;
  std::vector<RssItem> items;
  RssItem currentItem;
  std::string currentText;

  bool inItem = false;
  bool inTitle = false;
  bool inLink = false;
  bool inDescription = false;
  bool inSummary = false;
  bool inContent = false;
  bool inContentEncoded = false;
  bool inPubDate = false;
  bool inUpdated = false;
};
