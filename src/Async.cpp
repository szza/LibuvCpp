#include "EventLoop.h"
#include "Async.h" 

using namespace uv; 

Async::Async(EventLoop* loop) 
: async_(new uv_async_t) 
{
  uv_async_init(loop->loop(), 
               async_.get(), 
               Async::callback);

  async_->data = static_cast<void*>(this); 
}

Async::~Async() 
{
  this->close();
}

void Async:: runInThisLoop(Task cb) { 
  {
    MutexLock lock(mutex_);
    taskList_.emplace(std::move(cb));
  }

  uv_async_send(async_.get());
}

void Async::worker() {
  MutexLock lock(mutex_);
  while(!taskList_.empty()) {
    auto func = taskList_.front();
    func();
    taskList_.pop();
  }
}

void Async::close() { 
  
  if(uv_is_closing((uv_handle_t*)async_.get()) ==0) { 
    uv_close((uv_handle_t*)async_.get(), nullptr);
  }
}
