#include "EventLoop.h" 
#include "Async.h"
#include <iostream>
using namespace uv;

EventLoop::EventLoop() 
: loop_(uv_default_loop()), 
  async_(new Async(this)),
  running_(false)
{ }

EventLoop:: ~EventLoop() 
{ 
  this->stopLoop();
}

int EventLoop::runLoop(uv_run_mode mode) { 
  threadId_ = std::this_thread::get_id();
  running_ = true;

  uv_run(loop_, mode);

  return 0;
}

void EventLoop::stopLoop() { 
  running_ = false;
  if(uv_loop_alive(loop_))
    uv_loop_close(loop_);
}

bool EventLoop::inLoopThread() {

  return running_ && std::this_thread::get_id() == threadId_;
}

void EventLoop::runTaskInLoop(const Task task) { 
  if(task == nullptr) return; 

  if(inLoopThread()) 
  {
    task();
  } 
  else 
  {
    async_->runInThisLoop(task);
  }
}

const char* EventLoop::errorMsg(int status) { 
  if(status ==-1) 
  { 
    return "the connection is disconnected";
  }
  else
  {
    return uv_strerror(status);
  }
}