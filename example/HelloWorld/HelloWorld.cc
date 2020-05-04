#include "src/libuvCpp.h"

int main(int argc, char const *argv[]) {
  
  nanolog::set_log_level(nanolog::LogLevel::INFO);
  nanolog::initialize(nanolog::NonGuaranteedLogger(10), 
                      "/media/szz/Others/Self_study/Cpp/MyPro/LibuvCpp/log/", 
                      "helloworld", 
                      1);
  uv::EventLoop loop;

  // 捕捉SIGINT信号
  uv::Signal sig(&loop, 
                  SIGINT, 
                  [&loop](int signum)
                  { 
                    LOG_WARN<<"Capture signal(SIGINT)";
                    loop.stopLoop();
                  });

  uv::TcpServer server(&loop);
  server.setMessageCallback(
                  [](uv::ConnectionPtr connPtr, const char* data, ssize_t size)
                  {
                    LOG_INFO<<"[Server] New message: "<<data;
                    connPtr->write(data, size, nullptr);
                  }); 

  server.setCloseConnectionCallback(
                  [](std::weak_ptr<uv::Connection> connPtr)
                  { 
                    LOG_WARN<<"[Server] Connection is closing";
                  });
  
  server.bindAndListen("0.0.0.0", 5432); 

  uv::TcpClient client(&loop);
  client.setConnectStatusCallback(
                [&client](uv::TcpClient::ConnectSt status) 
                { 
                  if(status == uv::TcpClient::ConnectSt::ConnectSuccess) { 
                    client.write("hello world", 12);
                  }
                  else 
                  {
                    LOG_WARN<< "[Client] Fail to connect server\n";
                  }
                });
  
                
  client.connect("192.168.0.108", 5432); 
  loop.runLoop();
  return 0;
}
