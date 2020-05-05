#include "src/Http/Response.h" 

using namespace uv;
using namespace uv::http;

Response::Response(HttpVersion version, StatCode code)
: version_(version), 
  statCode_(code)
{ }

Response::~Response() 
{ 

}

 
void Response::addPair(const std::string& key, const std::string& value) { 
  headers_.emplace(key, value);
}

void Response::addPair(const char* key, const char* value) { 
  headers_.emplace(std::string(key), std::string(value));
}

 
const std::string& Response::headerMap(const std::string& key) const { 
  auto iter = headers_.find(key);
  return  iter == headers_.end() ? "" : iter->second;
}

void Response::setContent(const char* buf) { 
  std::string data(buf);
  content_ .swap(data);
}

void Response::encode(std::string& data) { 
  data.clear(); 
  data.reserve(1024);
  data += httpVersionToStr(version_);
  data += ' ';
  data += statusCodeToStr(statCode_);
  data += ' ';
  data += statInfo_;
  data.append(crlf, sizeof(crlf));

  for(const auto& entry : headers_) { 
    data += entry.first;
    data += ": ";
    data += entry.second;
    data.append(crlf, sizeof(crlf));
  }

  data.append(crlf, sizeof(crlf));
  data += content_;
}

ParseResult Response::parse(const std::string& data) { 
  std::vector<std::string> headerList;
  auto pos = splitRequestByCRLF(data, headerList);
  if(pos == -1) { 
    return ParseResult::Fail;
  } 
  // 消息头
  for(const auto& entry : headerList) { 
    if(appendHeaderPair(entry, headers_)==false) 
    {
      return ParseResult::Error;
    }
  }
  // body
  content_ = std::move(std::string(data, pos +4));
  return ParseResult::Success;
}

ParseResult Response::parseAndComplete(const std::string& data) { 
  ParseResult err = parse(data);

  if(err == ParseResult::Success) { 
    auto iter = headers_.find("Content-Length");
    if(iter == headers_.end()) { 
      iter = headers_.find("content-length");
    }

    if(iter != headers_.end()) {
      uint64_t size = std::stoul(iter->second);
      if(size == content_.size()) { 
        return ParseResult::Success;
      }
      return ParseResult::Fail;
    }
    else {
      iter = headers_.find("Transfer-Encoding");
      if(iter != headers_.end() && iter->second == "chunked") 
      { 
        return isCompletedChunked();
      }
    }
  }

  return err;
}

ParseResult Response::isCompletedChunked() { 
  std::string temp; 
  uint64_t p1=0, p2=0;

  while(true) { 
    p2 = content_.find("\r\n", p1 +1);
    if(p2 == Npos) 
    { 
      return ParseResult::Fail;
    }

    uint64_t num = std::stoul(content_.substr(p1, p2 - p1), nullptr, 16);
    if(num ==0) { 
      content_.swap(temp);
      return ParseResult::Success;
    }

    p1 = p2 + 2 + num;
    if(p1 > content_.size()) 
    { 
      return ParseResult::Fail;
    }

    temp += content_.substr(p2+2, num);
  }
}

bool Response::parseStatus(const std::string& info) { 
  std::vector<std::string> dest;
  splitLineBySpace(info, dest);
  if(dest.size() != 3) 
  { 
    return false;
  }

  version_  = strToHttpVersion(dest[0]);
  statCode_ = strToStatCode(dest[1]);
  statInfo_ = dest[2];
  return true;
}





