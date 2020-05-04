#pragma once

#include <functional>
#include <memory>
#include <mutex>
#include <queue>
#include <list>

#include "Common.h"
#include "uv.h"

namespace uv {

class EventLoop;

class Async : public std::enable_shared_from_this<Async> { 
public:
  Async(EventLoop* loop);
  ~Async();

  void runInThisLoop(Task cb);

private:
  void worker(); 
  void close();

  // 在 loop 中执行
  static void callback(uv_async_t* handle) { 
    auto async = static_cast<Async*>(handle->data);
    async->worker();
  }

  std::mutex   mutex_;
  std::unique_ptr<uv_async_t>       async_;
  std::queue<Task, std::list<Task>> taskList_;
};
  
} // namespace uv
