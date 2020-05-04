#pragma once 

#include <unordered_map>
#include "src/Http/Internel.h"

namespace uv { 

namespace http { 

class Request { 
public:
  Request(HttpVersion version=HttpVersion::Http1_1, RequestMethon method=RequestMethon::Get);
  ~Request();

  template<typename String> void swapContent(String&& str);
  template<typename String> void addHeaderPair(String&& key, String&& value);
  template<typename String> void appendUrlParam(String&& key, String&& value);

  template<typename String> void setPath(const String& path);
  void setVersion(HttpVersion version) { version_ = version; }
  void setMethod(RequestMethon method) { method_  = method;  }

  template<typename String> const std::string& headerMap(String&& key) const;
  template<typename String> const std::string& urlParams(String&& key) const; 

  const std::string& content() const { return content_; }
  const std::string& value()   const { return value_;   }
  const std::string& path()    const { return path_;    }
  HttpVersion   version()      const { return version_; }
  RequestMethon method()       const { return method_;  }
  int           methodToNum()  const;

  void encode(std::string& data);
  ParseResult parse(const std::string& data);
  ParseResult parseAndComplete(const std::string& data);

  static std::string methodToStr(RequestMethon method);
  template<typename String>
  static RequestMethon strToMethod(String&& str);
private:
  void encodePath(std::string& path);
  bool parseUrl(const std::string& url);
  bool parsePath(const std::string& path);

  HttpVersion   version_;
  RequestMethon method_;
  std::string   path_;
  std::string   value_;

  std::unordered_map<std::string, std::string> urlParams_;
  std::unordered_map<std::string, std::string> headers_;
  std::string content_;
}; // class Request

} // namespace http
} // namespace uv

