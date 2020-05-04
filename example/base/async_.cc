#include <iostream>

#include "src/EventLoop.h"
#include "src/Signal.h"
#include "src/Common.h"
#include "src/Async.h"
#include "src/Idle.h"

int main(int argc, char const *argv[]) {
  uv::EventLoop eventLoop;

  uv::Idle idle(&eventLoop);

  // idle.setCallback([]()
  //                 { 
  //                   std::cout<<"Idle state\n"; 
  //                 });

  uv::Signal sig(&eventLoop, 
                 SIGINT, 
                 [](int signum) 
                  { 
                   std::cout<<"capture SIGINT.\n"; 
                   exit(0); 
                  });

  eventLoop.runTaskInLoop(
                        []()
                        { 
                          std::cout<<"Run in Loop——1.\n";
                        });
  eventLoop.runTaskInLoop(
                        []()
                        { 
                          std::cout<<"Run in Loop——2.\n";
                        });



  std::cout<<"start to run\n";
  eventLoop.runLoop();
  std::cout<<"Over.\n";
  return 0;
}
