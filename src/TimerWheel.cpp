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
  
  if(++index_ == expired_) { 
    index_ =0;
  }

  wheels_[index_].clear();
}

void TimerWheel::start() { 
  if(expired_) 
  {
    wheels_.resize(expired_); 
    timer_.start();
  }
}

void TimerWheel::insert(ConnectionPtr conn) { 
  if(!expired_)
    return;
  
  std::shared_ptr<ConnectionItem> con;
  if(con) { 
    wheels_[index_].insert(con);
  }
}

void TimerWheel::insertNew(ConnectionPtr conntion) { 
  if(!expired_) 
    return;
  
  std::shared_ptr<ConnectionItem> conn = std::make_shared<ConnectionItem>(conntion); 
  conntion->setConnectionItem(conn);
  wheels_[index_].insert(conn);
}

