#pragma once 

#include <functional>
#include <memory>
#include <mutex>

namespace uv {

struct WriteInfo { 
  char*   buf;
  size_t  len;
  int     status;
};

typedef std::lock_guard<std::mutex>  MutexLock;
typedef std::function<void()>        Task; 

class EventLoop;
class Connection;
class TcpServer;

typedef std::shared_ptr<EventLoop>  EventLoopPtr;
typedef std::shared_ptr<Connection> ConnectionPtr;

typedef std::function<void(WriteInfo&)>                          WriteCompleteCallback;
typedef std::function<void(ConnectionPtr, const char*, int64_t)> OnMessageCallback;
typedef std::function<void(std::string& )>                       OnCloseCallback;
typedef std::function<void(std::string& )>                       CloseCompleteCallback;
} // namespace uv
