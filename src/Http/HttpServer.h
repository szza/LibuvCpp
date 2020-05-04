#pragma once

#include "src/Http/Response.h"
#include "src/Http/Request.h"
#include "src/Http/RadixTree.h"

#include "src/TcpServer.h"

namespace uv {

namespace http {

class HttpServer : public TcpServer { 
public:
  typedef std::function<void(Request&, Response*)> OnHttpReqCallback;
  
  HttpServer(EventLoop* loop);

  template<typename String> void Get (String&& path, OnHttpReqCallback cb);
  template<typename String> void Post(String&& path, OnHttpReqCallback cb);
  template<typename String> void Head(String&& path, OnHttpReqCallback cb);
  template<typename String> void Put (String&& path, OnHttpReqCallback cb);
  template<typename String> void Delete (String&& path, OnHttpReqCallback cb);
  template<typename String> void Connect(String&& path, OnHttpReqCallback cb);
  template<typename String> void Options(String&& path, OnHttpReqCallback cb);
  template<typename String> void Trace(String&& path, OnHttpReqCallback cb);
  template<typename String> void Patch(String&& path, OnHttpReqCallback cb);

private:
  RadixTree<OnHttpReqCallback> route_[9]; 

  void onMessage(ConnectionPtr connPtr, const char* buf, ssize_t size);

}; // class HttpServer

} // namespace http
} // namespace uv
