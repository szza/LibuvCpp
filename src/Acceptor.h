#pragma once 

#include "EventLoop.h"
#include "Common.h"

namespace uv { 

typedef std::function<void(EventLoop*, std::shared_ptr<uv_tcp_t>)> NewConnectionCallback;

class Acceptor { 
public:
  Acceptor(EventLoop* loop, bool tcpNoDelay);
  ~Acceptor();

  int bind(const char* ip, int port);
  int listen();

  void setNewConnectionCallback(NewConnectionCallback cb) { callback_ = cb; }
  
  EventLoop* loop()   const { return loop_;     }
  bool isListening()  const { return listened_; }
  bool isTcpNoDelay() const { return tcpNoDelay_;}
  
private:
  void onNewConnect(std::shared_ptr<uv_tcp_t> client);
  
  EventLoop* loop_;
  std::unique_ptr<uv_tcp_t>  server_;
  bool listened_;
  bool tcpNoDelay_;
  NewConnectionCallback callback_;
};

} // namespace uv