#include "RssParser.h"

#include <HardwareSerial.h>

#include <cstring>

RssParser::~RssParser() {
  if (parser) {
    XML_StopParser(parser, XML_FALSE);
    XML_SetElementHandler(parser, nullptr, nullptr);
    XML_SetCharacterDataHandler(parser, nullptr);
    XML_ParserFree(parser);
    parser = nullptr;
  }
}

bool RssParser::parse(const char* xmlData, const size_t length) {
  clear();

  parser = XML_ParserCreate(nullptr);
  if (!parser) {
    Serial.printf("[%lu] [RSS] Couldn't allocate memory for parser\n", millis());
    return false;
  }

  XML_SetUserData(parser, this);
  XML_SetElementHandler(parser, startElement, endElement);
  XML_SetCharacterDataHandler(parser, characterData);

  const char* currentPos = xmlData;
  size_t remaining = length;
  constexpr size_t chunkSize = 1024;

  while (remaining > 0) {
    void* const buf = XML_GetBuffer(parser, chunkSize);
    if (!buf) {
      Serial.printf("[%lu] [RSS] Couldn't allocate memory for buffer\n", millis());
      XML_ParserFree(parser);
      parser = nullptr;
      return false;
    }

    const size_t toRead = remaining < chunkSize ? remaining : chunkSize;
    memcpy(buf, currentPos, toRead);

    const bool isFinal = (remaining == toRead);
    if (XML_ParseBuffer(parser, static_cast<int>(toRead), isFinal) == XML_STATUS_ERROR) {
      Serial.printf("[%lu] [RSS] Parse error at line %lu: %s\n", millis(), XML_GetCurrentLineNumber(parser),
                    XML_ErrorString(XML_GetErrorCode(parser)));
      XML_ParserFree(parser);
      parser = nullptr;
      return false;
    }

    currentPos += toRead;
    remaining -= toRead;
  }

  XML_ParserFree(parser);
  parser = nullptr;

  Serial.printf("[%lu] [RSS] Parsed %zu items\n", millis(), items.size());
  return true;
}

void RssParser::clear() {
  items.clear();
  currentItem = RssItem{};
  currentText.clear();
  inItem = false;
  inTitle = false;
  inLink = false;
  inDescription = false;
  inSummary = false;
  inContent = false;
  inContentEncoded = false;
  inPubDate = false;
  inUpdated = false;
}

const char* RssParser::findAttribute(const XML_Char** atts, const char* name) {
  for (int i = 0; atts[i]; i += 2) {
    if (strcmp(atts[i], name) == 0) {
      return atts[i + 1];
    }
  }
  return nullptr;
}

bool RssParser::matchesElement(const char* name, const char* localName) {
  if (strcmp(name, localName) == 0) {
    return true;
  }
  const char* colon = strrchr(name, ':');
  return colon && strcmp(colon + 1, localName) == 0;
}

void XMLCALL RssParser::startElement(void* userData, const XML_Char* name, const XML_Char** atts) {
  auto* self = static_cast<RssParser*>(userData);

  if (matchesElement(name, "item") || matchesElement(name, "entry")) {
    self->inItem = true;
    self->currentItem = RssItem{};
    return;
  }

  if (!self->inItem) {
    return;
  }

  if (matchesElement(name, "title")) {
    self->inTitle = true;
    self->currentText.clear();
    return;
  }

  if (matchesElement(name, "link")) {
    const char* href = findAttribute(atts, "href");
    if (href) {
      const char* rel = findAttribute(atts, "rel");
      if ((!rel || strcmp(rel, "alternate") == 0) && self->currentItem.link.empty()) {
        self->currentItem.link = href;
      }
    } else {
      self->inLink = true;
      self->currentText.clear();
    }
    return;
  }

  if (matchesElement(name, "description")) {
    self->inDescription = true;
    self->currentText.clear();
    return;
  }

  if (matchesElement(name, "summary")) {
    self->inSummary = true;
    self->currentText.clear();
    return;
  }

  if (matchesElement(name, "content")) {
    self->inContent = true;
    self->currentText.clear();
    return;
  }

  if (strcmp(name, "content:encoded") == 0 || (strstr(name, "content") && matchesElement(name, "encoded"))) {
    self->inContentEncoded = true;
    self->currentText.clear();
    return;
  }

  if (matchesElement(name, "pubDate")) {
    self->inPubDate = true;
    self->currentText.clear();
    return;
  }

  if (matchesElement(name, "updated")) {
    self->inUpdated = true;
    self->currentText.clear();
    return;
  }
}

void XMLCALL RssParser::endElement(void* userData, const XML_Char* name) {
  auto* self = static_cast<RssParser*>(userData);

  if (matchesElement(name, "item") || matchesElement(name, "entry")) {
    if (!self->currentItem.title.empty()) {
      self->items.push_back(self->currentItem);
    }
    self->inItem = false;
    self->currentItem = RssItem{};
    return;
  }

  if (!self->inItem) {
    return;
  }

  if (matchesElement(name, "title")) {
    if (self->inTitle) {
      self->currentItem.title = self->currentText;
    }
    self->inTitle = false;
    return;
  }

  if (matchesElement(name, "link")) {
    if (self->inLink && self->currentItem.link.empty()) {
      self->currentItem.link = self->currentText;
    }
    self->inLink = false;
    return;
  }

  if (matchesElement(name, "description")) {
    if (self->inDescription && self->currentItem.summary.empty()) {
      self->currentItem.summary = self->currentText;
    }
    self->inDescription = false;
    return;
  }

  if (matchesElement(name, "summary")) {
    if (self->inSummary && self->currentItem.summary.empty()) {
      self->currentItem.summary = self->currentText;
    }
    self->inSummary = false;
    return;
  }

  if (matchesElement(name, "content")) {
    if (self->inContent && self->currentItem.content.empty()) {
      self->currentItem.content = self->currentText;
      if (self->currentItem.summary.empty()) {
        self->currentItem.summary = self->currentText;
      }
    }
    self->inContent = false;
    return;
  }

  if (strcmp(name, "content:encoded") == 0 || (strstr(name, "content") && matchesElement(name, "encoded"))) {
    if (self->inContentEncoded && self->currentItem.content.empty()) {
      self->currentItem.content = self->currentText;
      if (self->currentItem.summary.empty()) {
        self->currentItem.summary = self->currentText;
      }
    }
    self->inContentEncoded = false;
    return;
  }

  if (matchesElement(name, "pubDate")) {
    if (self->inPubDate) {
      self->currentItem.pubDate = self->currentText;
    }
    self->inPubDate = false;
    return;
  }

  if (matchesElement(name, "updated")) {
    if (self->inUpdated && self->currentItem.pubDate.empty()) {
      self->currentItem.pubDate = self->currentText;
    }
    self->inUpdated = false;
    return;
  }
}

void XMLCALL RssParser::characterData(void* userData, const XML_Char* s, const int len) {
  auto* self = static_cast<RssParser*>(userData);

  if (self->inTitle || self->inLink || self->inDescription || self->inSummary || self->inContent ||
      self->inContentEncoded || self->inPubDate || self->inUpdated) {
    self->currentText.append(s, len);
  }
}
