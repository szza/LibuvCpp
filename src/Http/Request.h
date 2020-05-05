#pragma once 

#include <unordered_map>
#include "src/Http/Internel.h"

namespace uv { 

namespace http { 

class Request { 
public:
  Request(HttpVersion version=HttpVersion::Http1_1, RequestMethon method=RequestMethon::Get);

  void addPair(const std::string& key, const std::string& value);
  void addUrlParam(const std::string& key, const std::string& value);

  void setPath(const std::string& path);
  void setContent(const std::string& str) { content_ = str;     }
  void setContent(const char* str)        { content_ = std::string(str); }
  void setVersion(HttpVersion version)    { version_ = version; }
  void setMethod(RequestMethon method)    { method_  = method;  }
  static 
  void setRootPath(const char* root)   { rootPath_ = root;}

  std::string headerMap(const std::string& key) const;
  std::string urlParams(const std::string& key) const;
  std::string headerMap(const char* key) const; 
  std::string urlParams(const char* key) const;

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
  
  static RequestMethon strToMethod(const std::string& str);
  void encodePath(std::string& path);
  bool parseUrl(const std::string& url);
  bool parsePath(const std::string& path);

private:
  HttpVersion   version_;
  RequestMethon method_;
  static const char* rootPath_;

  std::string   path_;
  std::string   value_;
  std::unordered_map<std::string, std::string> urlParams_;
  std::unordered_map<std::string, std::string> headers_;
  std::string content_;
}; // class Request

} // namespace http
} // namespace uv

