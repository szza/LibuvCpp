#include "src/Http/Request.h"
#include "src/NanoLog.h"

using namespace uv;
using namespace uv::http;

const char* Request::rootPath_ = "/";

Request::Request(HttpVersion version, RequestMethon method)
: version_(version), 
  method_(method) 
{ 

}
 
void Request::addPair(const std::string& key, const std::string& value) { 
  headers_[key] = value;
}

 
std::string Request::headerMap(const std::string& key) const { 
  auto it = headers_.find(key);
  return it == headers_.end() ? "" : it->second;
}

std::string Request::headerMap(const char* key) const { 
  std::string k (key);
  return headerMap(k);
} 

void Request::addUrlParam(const std::string& key, const std::string& value) { 
  urlParams_.emplace(key, value); 
}

std::string Request::urlParams(const std::string& key) const { 
  auto it = urlParams_.find(key);
  return it == urlParams_.end() ? "" : it->second;
}

std::string Request::urlParams(const char* key) const { 
  std::string k(key);
  return urlParams(k);
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


// GET /some HTTP/1.1
// Host: 192.168.0.108:5432
// Connection: keep-alive
// Cache-Control: max-age=0
// Upgrade-Insecure-Requests: 1
// User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/81.0.4044.129 Safari/537.36 Edg/81.0.416.68
// Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.9
// Accept-Encoding: gzip, deflate
// Accept-Language: zh-CN,zh;q=0.9,en;q=0.8,en-GB;q=0.7,en-US;q=0.6

ParseResult Request::parse(const std::string& data) { 
  std::vector<std::string> headerList;
  int pos = splitRequestByCRLF(data, headerList);
  if(pos == -1) 
  { 
    return ParseResult::Fail;
  }

  // 解析请求行
  if(!parseUrl(headerList[0])) 
  { 
    LOG_WARN<<"Fail to parse request Line.";
    return ParseResult::Error;
  }
  
  for(size_t i=1; i < headerList.size(); ++i) { 
    if(!appendHeaderPair(headerList[i], headers_)) 
    { 
       LOG_WARN<<"Fail to parse request header.";
      return ParseResult::Error;
    }
  }

  // content 
  content_ = std::move(std::string(data, pos+4));
  return ParseResult::Success;
}

ParseResult Request::parseAndComplete(const std::string& data) { 
  ParseResult st = parse(data);
  if(st != ParseResult::Success) {
    LOG_WARN<<"Fail to parse origin data from browser";
    return st;
  }

  auto it = headers_.find("Content-Length");
  if(it == headers_.end()) 
  { 
    it = headers_.find("content-length");
  }

  if(it != headers_.end()) {
    if(std::stoul(it->second) == content_.size()) 
    { 
      return ParseResult::Success;
    }
    else 
    { 
      LOG_WARN<<"the size of content real size: "<<content_.length()<<" but in theory shoubd be: "<<it->second;
      return ParseResult::Fail;
    }
  }

  LOG_WARN<<"Can't find <Content-Length: num> in request headers";
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

RequestMethon Request::strToMethod(const std::string& str) { 
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

// GET /some HTTP/1.1
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

// Get    /cgi-bin/my_cgi?uin=12345&appID=20&content=xxx
// post  POST /cgi-bin/my_cgi HTTP/1.1
bool Request::parsePath(const std::string& path) {
  
  urlParams_.reserve(128);
  auto pos = path.find(": ");
  if(pos != Npos) {
    
    path_  = std::move(std::string(path, 0, pos+1));
    value_ = std::move(std::string(path, pos+1, path.size() -1 -pos));
    return true;
  }

  pos = path.find('?');
  // Post请求
  // 不带参数的 Get 请求
  if(pos == Npos) 
  { 
    path_ = path;
  }
  else 
  {
    // 带参数的Get请求
    path_= std::move(std::string(path, 0, pos));
    //Get 请求
    for(size_t i=pos; i < path.length();) { 
      size_t p = path.find('=', i);

      if(p == Npos || p - i < 1) 
      { 
        break;
      }

      std::string key(path, i+1, p-i-1);
      i = p;
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
  }

  return true;
}
