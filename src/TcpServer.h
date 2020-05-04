#pragma once

#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <set>

#include "TimerWheel.h"
#include "Common.h"
#include "uv.h"

namespace uv {

class Acceptor;
class Connection;

class TcpServer { 
public:
  typedef std::function<void(std::weak_ptr<Connection>)> ConnectionCallback;

  TcpServer(EventLoop* loop, bool tcpNoDelay=true); 
  ~TcpServer(); 

  int  bindAndListen(const char* ip, int port);
  void closeConnection(const std::string& name);
  
  ConnectionPtr getConnection(const std::string& name);

  void setTimeout(uint32_t seconds)                      { timerwheel_.setTimeout(seconds); }
  void setMessageCallback(OnMessageCallback cb)          { onMessageCallback_         = cb; }
  void setNewConnectionCallback(ConnectionCallback cb)   { onNewConnectionCallback_   = cb; }
  void setCloseConnectionCallback(ConnectionCallback cb) { onCloseConnectionCallback_ = cb; }

  void write(ConnectionPtr connPtr, const char* buf, uint32_t size, WriteCompleteCallback cb=nullptr);
  void write(std::string& name, const char* buf, uint32_t size, WriteCompleteCallback cb=nullptr);
  void writeInLoop(ConnectionPtr connPtr, const char* buf, uint32_t size, WriteCompleteCallback cb=nullptr);
  void writeInLoop(std::string& name, const char* buf, uint32_t size, WriteCompleteCallback cb=nullptr);

private:
  void addConnection(const std::string& name, ConnectionPtr connPtr);
  void removeConnection(const std::string& name);
  void onAccept(EventLoop* loop, std::shared_ptr<uv_tcp_t> client);
  void onMessage(ConnectionPtr connPtr, const char* buf, size_t  size);

  EventLoop* loop_;
  std::unique_ptr<Acceptor> server_;
  std::unordered_map<std::string, ConnectionPtr> nameConns_;

  OnMessageCallback  onMessageCallback_;
  ConnectionCallback onNewConnectionCallback_;
  ConnectionCallback onCloseConnectionCallback_;

  TimerWheel timerwheel_;
}; // class TcpServer

} // namespace uv

