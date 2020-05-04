#include "src/libuvCpp.h"
#include <atomic>

int main(int argc, char const *argv[]) {
  nanolog::set_log_level(nanolog::LogLevel::INFO);
  nanolog::initialize(nanolog::NonGuaranteedLogger(10), 
                      "/media/szz/Others/Self_study/Cpp/MyPro/LibuvCpp/log/", 
                      "echoServer", 
                      10);
  uv::EventLoop loop; 
  uv::TcpServer server(&loop);
  std::atomic<int64_t> totalSize(0);

  server.setMessageCallback([&totalSize](uv::ConnectionPtr connPtr, const char* data, ssize_t size)
                           { 
                            totalSize += size;
                            connPtr->write(data, size, nullptr);
                            // LOG_INFO<<"[Server] receive data from client:"<<data;
                           });
  
  server.bindAndListen("0.0.0.0", 5432);

  // 1S
  uv::Timer timer(&loop,
                  1000,
                  1000, 
                  [&totalSize](uv::Timer* ptr)
                  { 
                    // LOG_INFO<<"[Server] Send data:" << (totalSize >> 10) << " kbyte/s";
                  });
  timer.start();
  loop.runLoop();
  return 0;
}
