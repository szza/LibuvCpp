#pragma once

#include "src/Http/Internel.h"
#include "src/Http/Response.h"
#include "src/Http/Request.h"
#include "src/TcpClient.h"

#include <memory>

namespace uv {

namespace http {

class HttpClient { 
public:
  typedef std::function<void(ParseResult, Response*)> OnResponseCallback;
  
  HttpClient(EventLoop* loop);
  ~HttpClient();

  void sendRequest(const char* ip, int port, Request& req);
  void setResponseCallback(OnResponseCallback cb) { onResponseCallback_ = cb; } 

private:
  void onResponse(ParseResult status, Response* resp);
  void onConnectStatus(TcpClient::ConnectSt status);
  void onMessage(const char* buff, ssize_t size);

  std::unique_ptr<TcpClient>  client_;
  std::shared_ptr<Request>     req_;
  std::unique_ptr<std::string> buffer_;
  bool connected_;

  OnResponseCallback onResponseCallback_;
};

} // namespace http
} // namespace uv



