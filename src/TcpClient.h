#pragma once 

#include "Common.h"
#include "uv.h"

namespace uv { 

class EventLoop;
class Connection;
class Buffer;

class TcpClient { 
public:
  enum class ConnectSt 
  { 
    ConnectSuccess,
    ConnectFail,
    ConnectClose
  };

  typedef std::function<void(const char*, ssize_t)>  NewMessageCallback;
  typedef std::function<void(ConnectSt)>             ConnectStCallback;

  TcpClient(EventLoop* loop, bool tcpNoDelay=true);
  ~TcpClient();

  void connect(const char* ip, int port);
  void close(CloseCompleteCallback cb);

  void write(const char* buf, uint32_t size, WriteCompleteCallback cb=nullptr);
  void writeInLoop(const char* buf, uint32_t size, WriteCompleteCallback cb=nullptr);

  std::shared_ptr<Buffer> getCurrentBuff() const;
  EventLoop* loop()       const { return loop_;        }
  bool isTcpNoDelay()     const { return tcpNoDealy_ ; }

  void setTcoNoDelay(bool delay)                      { tcpNoDealy_ = delay; }
  void setConnectStatusCallback(ConnectStCallback cb) { connectStCallback_ = cb;}
  void setMessageCallback(NewMessageCallback cb)      { onMessageCallback_ = cb;}

private:
  void onConnect(bool successed);
  void ifConnecFail();
  void oncloseConnection(const std::string& name);
  void onClose(std::string& name);
  void onMessage(ConnectionPtr connPtr, const char* buf, ssize_t size);
  void ConnectStatusCallback(ConnectSt status);  

  EventLoop*                    loop_;
  std::shared_ptr<uv_tcp_t>     client_;
  std::unique_ptr<uv_connect_t> req_;
  ConnectionPtr                 connection_;
  
  NewMessageCallback onMessageCallback_;
  ConnectStCallback  connectStCallback_;

  bool tcpNoDealy_;
}; // class TcpClient

} // namespace uv