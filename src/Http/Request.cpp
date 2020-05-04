#include "Http/Request.h"

using namespace uv;
using namespace uv::http;

Request::Request(HttpVersion version, RequestMethon method)
: version_(version), 
  method_(method) 
{ 

}

template<typename String> 
void Request::swapContent(String&& str) { 
  content_.swap(std::forward<String>(str));
}

template<typename String> 
void Request::addHeaderPair(String&& key, String&& value) { 
  headers_[std::forward<String>(key)] = std::forward<String>(value);
}

template<typename String> 
const std::string& Request::headerMap(String&& key) const { 
  auto it = headers_.find(std::forward<String>(key));
  return it == headers_.end() ? "" : it->second;
}

template<typename String> 
void Request::appendUrlParam(String&& key, String&& value) { 
  urlParams_.emplace(std::forward<String>(key), std::forward<String>(value)); 
}

template<typename String> 
const std::string& Request::urlParams(String&& key) const { 
  auto it = urlParams_.find(std::forward<String>(key));
  return it == urlParams_.end() ? "" : it->second;
}

void Request::encode(std::string& data) {
  data.clear();
  data.reserve(1024);
  // 请求行
  data += methodToStr(method_);
  data += ' ';
  encodePath(data);
  data += ' ';
  data += httpVersionToStr(version_);
  data.append(crlf, sizeof(crlf));

  // 消息头
  for(const auto& entry : headers_) { 
    data += entry.first;
    data += ": ";
    data += entry.second;
    data.append(crlf, sizeof(crlf));
  }
  
  // 空行
  data.append(crlf, sizeof(crlf));
  data += content_;
}


ParseResult Request::parse(const std::string& data) { 
  std::vector<std::string> headerList;
  int pos = splitRequestByCRLF(data, headerList);
  if(pos == -1) 
  { 
    return ParseResult::Fail;
  }

  if(!parseUrl(headerList[0])) 
  { 
    return ParseResult::Error;
  }

  for(size_t i=0; i < headerList.size(); ++i) { 
    if(!appendHeaderPair(headerList[i], headers_)) 
    { 
      return ParseResult::Error;
    }
  }

  // content 
  content_ = std::move(std::string(data, pos+4));
  return ParseResult::Success;
}

ParseResult Request::parseAndComplete(const std::string& data) { 
  ParseResult st = parse(data);
  if(st != ParseResult::Success) 
    return st;

  auto it = headers_.find("Content-Length");
  if(it == headers_.end()) 
  { 
    it = headers_.find("content-length");
  }

  if(it != headers_.end()) {
    return (std::stoul(it->second) == content_.size()) ? 
            ParseResult::Success : 
            ParseResult::Fail;
  }

  return st;
}


std::string Request::methodToStr(RequestMethon method) { 
  switch(method) { 
  case RequestMethon::Get    : return "GET";
  case RequestMethon::Post   : return "POST";
  case RequestMethon::Head   : return "HEAD";
  case RequestMethon::Put    : return "PUT";
  case RequestMethon::Delete : return "DELETE";
  case RequestMethon::Connect: return "CONNECT";
  case RequestMethon::Options: return "OPTIONS";
  case RequestMethon::Trace  : return "TRACE";
  case RequestMethon::Patch  : return "PATCH";
  default:                     return "Invalid";
  }

 
}

template<typename String>
RequestMethon Request::strToMethod(String&& str) { 
  if (str == "GET")
  {
    return RequestMethon::Get;
  }
  if (str == "POST")
  {
    return RequestMethon::Post;
  }
  if (str == "HEAD")
  {
    return RequestMethon::Head;
  }
  if (str == "PUT")
  {
    return RequestMethon::Put;
  }
  if (str == "DELETE")
  {
    return RequestMethon::Delete;
  }
  if (str == "CONNECT")
  {
    return RequestMethon::Connect;
  }
  if (str == "OPTIONS")
  {
    return RequestMethon::Options;
  }
  if (str == "TRACE")
  {
    return RequestMethon::Trace;
  }
  if (str == "PATCH")
  {
    return RequestMethon::Patch;
  }

  return RequestMethon::Invalid;
}

int Request::methodToNum()  const { 
  switch(method_) { 
  case RequestMethon::Get    : return 0;
  case RequestMethon::Post   : return 1;
  case RequestMethon::Head   : return 2;
  case RequestMethon::Put    : return 3;
  case RequestMethon::Delete : return 4;
  case RequestMethon::Connect: return 5;
  case RequestMethon::Options: return 6;
  case RequestMethon::Trace  : return 7;
  case RequestMethon::Patch  : return 8;
  default                    : return 9; // Invalid
  }

}


void Request::encodePath(std::string& path) { 
  path.reserve(1024);

  if(path_.empty() || path_[0] != '/') {
    path += '/';
  }
  path += path_;

  if(urlParams_.empty()) return;

  path += '?';
  for(const auto& entry : urlParams_) { 
    path += entry.first;
    path += '=';
    path += entry.second;
    path += '&';
  }

  path.pop_back(); // 弹出最后一个 `&`
}

bool Request::parseUrl(const std::string& url) { 
  std::vector<std::string> dest;
  splitLineBySpace(url, dest);

  if(dest.size() != 3) { 
    return false;
  }

  method_ = strToMethod(dest[0]);
  if(method_ == RequestMethon::Invalid || parsePath(dest[1]) == false) 
  { 
    return false;
  }

  version_ = strToHttpVersion(dest[2]);
  return true;
}

bool Request::parsePath(const std::string& path) {
  urlParams_.clear();
  urlParams_.reserve(1024);

  auto pos = path.find(": ");
  if(pos != Npos) {
    std::string temp(path, 0, pos+1); 
    path_.swap(temp);

    std::string value(path, pos+1, path.size() -1 -pos);
    value_.swap(value);
    return true;
  }

  pos = path.find('?');
  std::string temp (path, 0, pos); 
  path_.swap(temp);

  for(size_t i=pos; i < path.length();) { 
    size_t p = path.find('=', i);

    if(p == Npos || p - i < 1) { 
      break;
    }

    std::string key(path, i+1, p-i-1);
    i =p;
    p = path.find('&', i);
    if(p == Npos) 
    { 
      p = path.size();
    }

    if(p - i < 1)  
      break;

    std::string value(path, i+1, p-i-1);
    urlParams_.emplace(std::move(key), std::move(value));
  }

  return true;
}
