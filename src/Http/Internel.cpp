#include "src/Http/Internel.h"

using namespace uv;

std::string http::httpVersionToStr(HttpVersion version) { 
  switch(version) { 
  case HttpVersion::Http1_0 : return "HTTP/1.0";
  case HttpVersion::Http1_1 : return "HTTP/1.1";
  default: return "";
  }
}

std::string http::statusCodeToStr(StatCode code) { 
  switch(code) { 
  case StatCode::OK          : return "200";
  case StatCode::BadRequest  : return "400";
  case StatCode::Unauthorized: return "401";
  case StatCode::Forbidden   : return "403";
  case StatCode::NotFound    : return "404";
  case StatCode::ServerInternalError: return "500";
  case StatCode::ServerUnavailable  : return "503";
  }
  return "000";
}

http::StatCode http::strToStatCode(const std::string& code) { 
  if(code == "200") return StatCode::OK;
  if(code == "401") return StatCode::Unauthorized;
  if(code == "403") return StatCode::Forbidden;
  if(code == "404") return StatCode::NotFound;
  if(code == "500") return StatCode::ServerInternalError;
  if(code == "503") return StatCode::ServerUnavailable;
  
  return StatCode::BadRequest; // 200
}

http::HttpVersion http::strToHttpVersion(const std::string& str) {
  if(str == "HTTP/1.0") { 
    return HttpVersion::Http1_0;
  }
  
  if(str == "HTTP/1.1") { 
    return HttpVersion::Http1_1;  
  }

  return  HttpVersion::Unknow;
}

int http::splitRequestByCRLF (const std::string& request, std::vector<std::string>& dest, int32_t defaltSize) { 
  size_t emptyLine = request.find("\r\n\r\n");

  if(emptyLine == std::string::npos) 
    return -1;
  
  dest.reserve(defaltSize);
  dest.clear();

  // 请求行和消息头加入dest
  for(size_t curr=0; curr < emptyLine; ) { 
    size_t prev = curr;
    curr = request.find('\r\n', curr+1); // 从下个位置找结尾处
    if(curr == std::string::npos) { 
      break;
    }

    if(prev !=0) { 
      prev +=2;
    }

    dest.emplace_back(std::string(request, prev, curr-prev));
  }

  return static_cast<int>(emptyLine);
}


bool http::splitLineBySpace(const std::string& line, std::vector<std::string>& dest, int32_t defaultSize) { 
  size_t curr = -1; 
  for(size_t i =0; i < 2;) { 
    size_t prev = curr;
    curr = line.find(" ", curr+1);

    if(curr == prev+1) // 跳过开头的空白
      continue;

    if(curr == std::string::npos)
      return false;
    
    dest.emplace_back(std::string(line, prev +1, curr - prev -1));
    ++i;
  }

  if(curr == line.length()-1) 
    return false;

  dest.emplace_back(std::string(line, curr+1, line.length()-1 - curr));
  return true;
}

bool http::appendHeaderPair(const std::string& line, std::unordered_map<std::string, std::string>& headers) { 
  // 在请求头中，以 ": " 分割 key-value
  auto curr = line.find(": ");
  if(curr == std::string::npos) 
    return false;

  headers.emplace(std::string(line, 0, curr), std::string(line, curr+2));
  return true;
}