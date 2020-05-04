#pragma once 

#include <mutex>

#include <stdint.h>

#include "Common.h"
#include "uv.h"

namespace uv { 

class EventLoop;

class Timer { 
public:
  typedef std::function<void(Timer* )> TimerCallback;

  Timer(EventLoop* loop, uint64_t timeout, uint64_t repeat, TimerCallback cb=nullptr);
  ~Timer(); 

  void start();
  void setCloseCallback(TimerCallback cb) { closeCallback_ = cb; };
  void setTimerRepeat(uint64_t ms);

private:
  void stop();
  void onTimeout();
  void closeDone();
  static void worker(uv_timer_t* handle);

  std::unique_ptr<uv_timer_t> handle_;
  uint64_t timeout_;
  uint64_t repeat_;
  bool     actived_;

  TimerCallback timeoutCallback_;
  TimerCallback closeCallback_;
};
} // namespace uv  