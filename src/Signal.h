#pragma once 

#include <map>

#include "Common.h"
#include "uv.h"

namespace uv { 

class EventLoop; 

typedef std::function<void(int)>  SignalHandler;

class Signal { 
public:
  
  Signal(EventLoop* loop, int sig, SignalHandler handler=nullptr);
  ~Signal();

  void setCloseCallback(Task cb) { closeCallback_ = cb; }
  void setHandler(SignalHandler handler) { handler_ = handler; }

#ifdef __LINUX__
  static void ignore(int sig) { signal(sig, SIG_IGN); }
#endif

private:
  void close();
  void closeDone();

  std::unique_ptr<uv_signal_t> signal_;
  SignalHandler handler_;
  Task          closeCallback_; 
  
  static void onSignal(uv_signal_t* handle, int signum);
};
} // namespace uv