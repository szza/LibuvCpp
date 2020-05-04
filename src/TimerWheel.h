#pragma once

#include <unordered_set>
#include <vector>
#include <memory>

#include "Connection.h"
#include "Common.h"
#include "Timer.h"

namespace uv {

class ConnectionItem : public std::enable_shared_from_this<ConnectionItem> { 
public:
  ConnectionItem(ConnectionPtr conn)
  : conn_(conn) 
  { }

  ~ConnectionItem() { 
    ConnectionPtr internel = conn_.lock(); 
    if(internel) { 
      internel->onClientClose();
    }
  }

private:
  std::weak_ptr<Connection> conn_;

}; // class ConnectionItem

class TimerWheel { 
public:
  TimerWheel(EventLoop* loop, uint32_t timeout=0);
  ~TimerWheel() = default;

  void start(); 
  void insert(ConnectionPtr conn);
  void insertNew(ConnectionPtr conn);

  void setTimeout(uint32_t timeout) { expired_ = timeout; }
private:  

  void wheelCallback(); 

  uint32_t index_;
  uint32_t expired_;
  Timer   timer_;

  std::vector<std::unordered_set<std::shared_ptr<ConnectionItem>>> wheels_;
}; // class TimerWheel

} // namespace uv
