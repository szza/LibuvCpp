#pragma once

#include <unordered_map>
#include "src/Http/Internel.h"

namespace uv {

namespace http {

class Response { 
public:
  Response(HttpVersion version=HttpVersion::Http1_1, StatCode code=StatCode::BadRequest);
  ~Response();

  void setVersion(HttpVersion version)                    { version_ = version; }
  void setStatus(StatCode stat, const std::string& info)  { statCode_ = stat;  statInfo_ = info; }

  HttpVersion        version()    const { return version_;  }
  StatCode           statusCode() const { return statCode_; }
  const std::string& statusInfo() const { return statInfo_; }
  const std::string& content()    const { return content_;  }

  template<typename String> const std::string& headerMap(String&& key) const;
  template<typename String> void swapContent(String&& str);
  template<typename String> void addHeaderPair(String&& key, String&& value);
  
  void encode(std::string& data);
  ParseResult parse(const std::string& data);
  ParseResult parseAndComplete(const std::string& data);
  ParseResult isCompletedChunked();
private:
  bool parseStatus(const std::string& info);

  HttpVersion version_;
  StatCode    statCode_;
  std::string statInfo_;
  std::unordered_map<std::string, std::string> headers_;
  std::string content_;
}; // class Response

} // namespace http
} // namespace uv
