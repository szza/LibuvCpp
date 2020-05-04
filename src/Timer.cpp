#include "EventLoop.h"
#include "Timer.h"

using namespace uv;

Timer::Timer(EventLoop* loop, uint64_t timeout, uint64_t repeat, TimerCallback cb)
: handle_(new uv_timer_t), 
  timeout_(timeout),
  repeat_(repeat),
  actived_(false),
  timeoutCallback_(cb),
  closeCallback_(nullptr)
{
  handle_->data = static_cast<void*>(this);
  uv_timer_init(loop->loop(), handle_.get());
}

Timer::~Timer() 
{ 
  if(actived_) {
    this->stop();
  }
}

void Timer::start() { 
  if(!actived_) { 
    actived_ =true;
    uv_timer_start(handle_.get(), 
                  Timer::worker,
                  timeout_,
                  repeat_);
  }
}

void Timer::stop() {
  actived_ = false;
  if(uv_is_active((uv_handle_t*)handle_.get())) 
  { 
    uv_timer_stop(handle_.get());
  }

  if(uv_is_closing((uv_handle_t*)handle_.get())==0) { 
    uv_close((uv_handle_t*)handle_.get(), 
             [](uv_handle_t* handle)
             {
               Timer* self = static_cast<Timer*>(handle->data);
               self->closeDone();
             });
  }
  else 
  {
    closeDone();
  }
}

void Timer::setTimerRepeat(uint64_t milliseconds) { 
  repeat_ = milliseconds;
  uv_timer_set_repeat(handle_.get(), repeat_);
}

void Timer::onTimeout() { 
  if(timeoutCallback_) 
  {
    timeoutCallback_(this);
  }
}

void Timer::closeDone() { 
  if(closeCallback_)
  {
    closeCallback_(this);
  }
}

void Timer::worker(uv_timer_t* handle) { 
  Timer* self = static_cast<Timer*>(handle->data);
  self->onTimeout();
}