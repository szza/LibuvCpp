#include "TimerWheel.h"

using namespace uv;

TimerWheel::TimerWheel(EventLoop* loop, uint32_t timeout) 
: index_(0), 
  expired_(timeout), 
  timer_(loop, 1000, 1000, std::bind(&TimerWheel::wheelCallback, this)) 
{ }

void TimerWheel::wheelCallback() { 
  if(! expired_) 
    return;
  // 这个函数一秒执行一次
  // index_ 每过一秒就刷新一次 ++index
  if(++index_ == expired_) { 
    index_ =0;
  }
  // clear则会是的引用计数减少1到0，那么对应的连接会被关闭。
  //
  wheels_[index_].clear();
}

void TimerWheel::start() { 
  if(expired_) 
  {
    wheels_.resize(expired_); 
    timer_.start();
  }
}

void TimerWheel::insert(ConnectionPtr connPtr) { 
  if(!expired_)
    return;

  std::shared_ptr<ConnectionItem> conn = connPtr->Item().lock();
  if(conn) { 
    wheels_[index_].insert(conn);
  }
}

void TimerWheel::insertNew(ConnectionPtr connPtr) { 
  if(!expired_) 
    return;
  
  std::shared_ptr<ConnectionItem> conn = std::make_shared<ConnectionItem>(connPtr); 
  connPtr->setConnectionItem(conn);
  wheels_[index_].insert(conn);
}

