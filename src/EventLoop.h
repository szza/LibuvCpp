#pragma once

#include <thread>
#include <atomic>

#include "Common.h"
#include "uv.h"

namespace uv { 

class Async; 

class EventLoop { 
public:
   EventLoop(); 
  ~EventLoop();;

  int  runLoop(uv_run_mode mode=UV_RUN_DEFAULT);
  void stopLoop();
  bool inLoopThread(); 
  void runTaskInLoop(const Task task);
  
  bool       active() const { return uv_loop_alive(loop_); }
  uv_loop_t* loop()   const { return loop_;}

  static const char* errorMsg(int status);
private:
  uv_loop_t* loop_;
  std::unique_ptr<Async> async_;
  bool running_;
  std::thread::id threadId_;
};

} // namespace uv