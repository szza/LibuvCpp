#include "src/libuvCpp.h"

int main(int argc, char const *argv[]) {

  nanolog::set_log_level(nanolog::LogLevel::INFO);
  nanolog::initialize(nanolog::NonGuaranteedLogger(10), 
                      "/media/szz/Others/Self_study/Cpp/MyPro/LibuvCpp/log/", 
                      "echoClient", 
                      10);

  uv::EventLoop loop;
  std::shared_ptr<uv::TcpClient> client = std::make_shared<uv::TcpClient>(&loop);
  client->setConnectStatusCallback([client](uv::TcpClient::ConnectSt status)
                                   { 
                                     if(status == uv::TcpClient::ConnectSt::ConnectSuccess) { 
                                       client->write("hello server", 13);
                                     }
                                     else 
                                     {
                                       LOG_WARN<<"[Client] Fail to connect server";
                                     }
                                   });

  client->setMessageCallback([client](const char* data, ssize_t size)
                             { 
                              //  LOG_INFO<<"[Client] receive message from server:"<<data;
                               client->write(data,size);
                             });

  client->connect("192.168.0.108", 5432);
  loop.runLoop();
  return 0;
}
