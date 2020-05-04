#pragma once

#include <unordered_map>
#include <string>
#include <vector>

#include <stdint.h>

namespace uv { 

namespace http { 

constexpr const size_t Npos = std::string::npos;

enum class ParseResult {
  Success, 
  Fail, 
  Error
};

enum class HttpVersion { 
  Unknow,  
  Http1_0, 
  Http1_1,
};

enum class RequestMethon {
  Get,
  Post,
  Head,
  Put,
  Delete,
  Connect,
  Options,
  Trace,
  Patch,
  Invalid,
};

enum class StatCode {
  OK                  = 200,  // 客户端请求成功
  BadRequest          = 400,  // 客户端请求有语法错误，不能被服务器所理解
  Unauthorized        = 401,  // 请求未经授权，这个状态代码必须和 WWW-Authenticate 报头域一起使用 
  Forbidden           = 403 , // 服务器收到请求，但是拒绝提供服务
  NotFound            = 404 , // 请求资源不存在，eg：输入了错误的URL
  ServerInternalError = 500 , // 服务器发生不可预期的错误
  ServerUnavailable   = 503,  // 服务器当前不能处理客户端的请求，一段时间后可能恢复正常
};

constexpr const char crlf[2] = {'\r', '\n'};
std::string statusCodeToStr(StatCode code);
StatCode strToStatCode(const std::string& str);
std::string httpVersionToStr(HttpVersion version);
HttpVersion strToHttpVersion(const std::string& );
int  splitRequestByCRLF (const std::string& , std::vector<std::string>& , int32_t defaltSize=64);
bool splitLineBySpace(const std::string& , std::vector<std::string>& , int32_t defaultSize=64);
bool appendHeaderPair(const std::string& , std::unordered_map<std::string, std::string>& );

} // namespace http

} // namespace uv  