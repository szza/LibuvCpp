#include "EventLoop.h"
#include "Signal.h"

using namespace uv;

Signal::Signal(EventLoop* loop, int sig, SignalHandler handler)
: signal_(new uv_signal_t), 
  handler_(handler), 
  closeCallback_(nullptr)
{ 
  uv_signal_init(loop->loop(), signal_.get()); 
  signal_->data = static_cast<void*>(this);
  uv_signal_start(signal_.get(), Signal::onSignal, sig); 
}

Signal::~Signal() 
{
  this->close();
}

void Signal::close() { 
  if(uv_is_active((uv_handle_t*)signal_.get()) ==0) 
  { 
    uv_signal_stop(signal_.get());
  }

  if(uv_is_closing((uv_handle_t*)signal_.get())==0) {
    uv_close((uv_handle_t*)signal_.get(), 
             [](uv_handle_t* handle)
             {
               Signal* self = static_cast<Signal*>(handle->data);
               self->closeDone();
             }) ;

  }
  else 
  {
    closeDone();
  }
}

void Signal::closeDone() { 
  if(closeCallback_) { 
    closeCallback_();
  }
}

void Signal::onSignal(uv_signal_t* handle, int signum) { 
  Signal* self = static_cast<Signal*>(handle->data);
  if(self->handler_) { 
    self->handler_(signum);
  }
}
