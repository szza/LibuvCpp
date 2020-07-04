#pragma once

#include <string>
#include <vector>
#include "Common.h"
#include "uv.h"

namespace uv {

class ConnectionItem;
class EventLoop;
class Buffer;

class Connection : public std::enable_shared_from_this<Connection> { 
public:
  Connection(EventLoop* loop, const std::string& name, std::shared_ptr<uv_tcp_t> client, bool isConnected=true, bool keepLive=false);
  ~Connection(); 

  void close();
  void onClientClose();

  void write(const char* buf, ssize_t size, WriteCompleteCallback cb);
  void writeInLoop(const char* buf, ssize_t size, WriteCompleteCallback cb);
  
  void setMessageCallback(OnMessageCallback cb)                { onMessageCallback_    = cb; }
  void setConnectCloseCallback(OnCloseCallback cb)             { onCloseCallback_      = cb; }
  void setCloseCompleteCallback(CloseCompleteCallback cb)      { closeCompleteCallback_= cb; }
  void setConnectStatus(bool connect)                          { connected_ = connect;       }
  void setConnectionItem(std::shared_ptr<ConnectionItem> conn) { item_ = conn;               }
  
  bool active() const { return connected_ && uv_is_active((uv_handle_t*)client_.get()); }
  const std::string&            name()          const { return *name_; }
  std::weak_ptr<ConnectionItem> Item()          const { return item_;  }
  std::shared_ptr<Buffer>       getPackBuffer() const { return buffer_;}
private:
  static 
  void onMessageRecv(uv_stream_t* client, ssize_t nread, const uv_buf_t* buf);
  void onMessage(const char* buffer, int64_t size); 
  void closeComplete(); 

  char* allocate(size_t size);

  EventLoop*                    loop_;
  std::shared_ptr<uv_tcp_t>     client_;
  std::shared_ptr<std::string>  name_;
  std::vector<char>             data_;
  std::shared_ptr<Buffer>       buffer_; // 一个 connection 对应一个 buffer_
  std::weak_ptr<ConnectionItem> item_;
  
  OnMessageCallback     onMessageCallback_;
  OnCloseCallback       onCloseCallback_;       // 关闭时执行的函数
  CloseCompleteCallback closeCompleteCallback_; // 关闭完成执行的函数

  bool connected_;
  bool keepLive_;
};
} // namespace uv


