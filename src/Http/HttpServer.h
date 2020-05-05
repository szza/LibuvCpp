#pragma once

#include "src/Http/Response.h"
#include "src/Http/Request.h"
#include "src/TcpServer.h"

#include <unordered_map>
#include <string>
#include <array>

namespace uv {

namespace http {

class HttpServer : public TcpServer { 
public:
  typedef std::function<void(Request&, Response*)> OnHttpReqCallback;
  
  HttpServer(EventLoop* loop);

  void Get(std::string path, OnHttpReqCallback cb);

  void Put(std::string path, OnHttpReqCallback cb);

  void Post(std::string path, OnHttpReqCallback cb);
  void Head(std::string path, OnHttpReqCallback cb);

  void Trace(std::string path, OnHttpReqCallback cb);
  void Patch(std::string path, OnHttpReqCallback cb);
  
  void Delete(std::string path, OnHttpReqCallback cb);

  void Connect(std::string path, OnHttpReqCallback cb);
  void Options(std::string path, OnHttpReqCallback cb);

private:
  void onMessageRecv(ConnectionPtr connPtr, const char* buf, ssize_t size);

  // 根据请求方法  -->回调函数
  std::array<std::unordered_map<std::string, OnHttpReqCallback>, 9> route_;

}; // class HttpServer

} // namespace http
} // namespace uv
