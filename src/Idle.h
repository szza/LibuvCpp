#pragma once

#include "EventLoop.h"
#include "Common.h"
#include "uv.h"

namespace uv {

class Idle { 
public:
  Idle(EventLoop* loop)
  : idle_(new uv_idle_t())
  {
    idle_->data = static_cast<void*>(this);
    uv_idle_init(loop->loop(), idle_);
    uv_idle_start(idle_, Idle::idleCallback);
  }

  ~Idle() 
  { 
    uv_idle_stop(idle_);
    delete idle_;
    idle_ =nullptr;
  }

  void setCallback(Task cb) 
  { task_ = cb; }

private:
   void onCallback() 
  { if(task_) task_(); }

  static void idleCallback(uv_idle_t* handle) 
  { 
    Idle* self = static_cast<Idle*>(handle->data);
    self->onCallback();
  }

  uv_idle_t* idle_;
  Task       task_;
};


} // namespace uv
