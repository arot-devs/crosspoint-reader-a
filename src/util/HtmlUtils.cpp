#include "HtmlUtils.h"

#include <cctype>

namespace {
void appendCodepoint(std::string& out, uint32_t codepoint) {
  if (codepoint <= 0x7F) {
    out.push_back(static_cast<char>(codepoint));
  } else if (codepoint <= 0x7FF) {
    out.push_back(static_cast<char>(0xC0 | ((codepoint >> 6) & 0x1F)));
    out.push_back(static_cast<char>(0x80 | (codepoint & 0x3F)));
  } else if (codepoint <= 0xFFFF) {
    out.push_back(static_cast<char>(0xE0 | ((codepoint >> 12) & 0x0F)));
    out.push_back(static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F)));
    out.push_back(static_cast<char>(0x80 | (codepoint & 0x3F)));
  } else if (codepoint <= 0x10FFFF) {
    out.push_back(static_cast<char>(0xF0 | ((codepoint >> 18) & 0x07)));
    out.push_back(static_cast<char>(0x80 | ((codepoint >> 12) & 0x3F)));
    out.push_back(static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F)));
    out.push_back(static_cast<char>(0x80 | (codepoint & 0x3F)));
  }
}

bool decodeEntity(const std::string& entity, std::string& out) {
  if (entity == "amp") {
    out.push_back('&');
    return true;
  }
  if (entity == "lt") {
    out.push_back('<');
    return true;
  }
  if (entity == "gt") {
    out.push_back('>');
    return true;
  }
  if (entity == "quot") {
    out.push_back('"');
    return true;
  }
  if (entity == "apos" || entity == "#39") {
    out.push_back('\'');
    return true;
  }
  if (entity == "nbsp") {
    out.push_back(' ');
    return true;
  }
  if (entity == "mdash") {
    out.append("--");
    return true;
  }
  if (entity == "ndash") {
    out.push_back('-');
    return true;
  }
  if (entity == "hellip") {
    out.append("...");
    return true;
  }

  if (!entity.empty() && entity[0] == '#') {
    uint32_t value = 0;
    if (entity.size() > 1 && (entity[1] == 'x' || entity[1] == 'X')) {
      for (size_t i = 2; i < entity.size(); ++i) {
        const char ch = entity[i];
        if (!std::isxdigit(static_cast<unsigned char>(ch))) {
          return false;
        }
        value *= 16;
        if (ch >= '0' && ch <= '9') {
          value += static_cast<uint32_t>(ch - '0');
        } else if (ch >= 'a' && ch <= 'f') {
          value += static_cast<uint32_t>(ch - 'a' + 10);
        } else if (ch >= 'A' && ch <= 'F') {
          value += static_cast<uint32_t>(ch - 'A' + 10);
        }
      }
    } else {
      for (size_t i = 1; i < entity.size(); ++i) {
        const char ch = entity[i];
        if (!std::isdigit(static_cast<unsigned char>(ch))) {
          return false;
        }
        value = value * 10 + static_cast<uint32_t>(ch - '0');
      }
    }

    if (value > 0) {
      appendCodepoint(out, value);
      return true;
    }
  }

  return false;
}

void appendNewline(std::string& out) {
  if (out.empty() || out.back() != '\n') {
    out.push_back('\n');
  }
}

std::string normalizeWhitespace(const std::string& text) {
  std::string out;
  out.reserve(text.size());

  bool inSpace = false;
  int newlineCount = 0;

  for (char ch : text) {
    if (ch == '\r') {
      ch = '\n';
    }

    if (ch == '\n') {
      if (!out.empty() && out.back() == ' ') {
        out.pop_back();
      }
      if (newlineCount < 2) {
        out.push_back('\n');
      }
      newlineCount++;
      inSpace = false;
      continue;
    }

    if (std::isspace(static_cast<unsigned char>(ch))) {
      if (!inSpace) {
        out.push_back(' ');
        inSpace = true;
      }
      newlineCount = 0;
      continue;
    }

    out.push_back(ch);
    inSpace = false;
    newlineCount = 0;
  }

  while (!out.empty() && (out.back() == ' ' || out.back() == '\n')) {
    out.pop_back();
  }

  return out;
}

}  // namespace

namespace HtmlUtils {
std::string toPlainText(const std::string& html) {
  std::string out;
  out.reserve(html.size());

  bool inTag = false;
  bool skipContent = false;
  std::string tagBuffer;

  for (size_t i = 0; i < html.size(); ++i) {
    char ch = html[i];

    if (inTag) {
      if (ch == '>') {
        inTag = false;

        std::string tag = tagBuffer;
        tagBuffer.clear();

        size_t start = 0;
        while (start < tag.size() && std::isspace(static_cast<unsigned char>(tag[start]))) {
          ++start;
        }

        bool isClosing = false;
        if (start < tag.size() && tag[start] == '/') {
          isClosing = true;
          ++start;
        }

        size_t end = start;
        while (end < tag.size() && std::isalnum(static_cast<unsigned char>(tag[end]))) {
          ++end;
        }

        std::string tagName = tag.substr(start, end - start);
        for (auto& c : tagName) {
          c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        }

        if (tagName == "script" || tagName == "style") {
          skipContent = !isClosing;
        }

        if (!skipContent) {
          if (tagName == "br" || tagName == "p" || tagName == "div" || tagName == "ul" || tagName == "ol" ||
              tagName == "h1" || tagName == "h2" || tagName == "h3" || tagName == "h4" || tagName == "h5" ||
              tagName == "h6" || tagName == "blockquote") {
            appendNewline(out);
          } else if (tagName == "li") {
            appendNewline(out);
            out.append("- ");
          }
        }
        continue;
      }

      tagBuffer.push_back(ch);
      continue;
    }

    if (ch == '<') {
      inTag = true;
      tagBuffer.clear();
      continue;
    }

    if (skipContent) {
      continue;
    }

    if (ch == '&') {
      const size_t semi = html.find(';', i + 1);
      if (semi != std::string::npos && semi - i <= 12) {
        const std::string entity = html.substr(i + 1, semi - i - 1);
        if (decodeEntity(entity, out)) {
          i = semi;
          continue;
        }
      }
    }

    out.push_back(ch);
  }

  return normalizeWhitespace(out);
}
}  // namespace HtmlUtils
