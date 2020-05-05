#include "src/libuvCpp.h"
#include "uv.h"

using namespace uv::http;

int main(int argc, char const *argv[]) {

  nanolog::set_log_level(nanolog::LogLevel::INFO);
  nanolog::initialize(nanolog::NonGuaranteedLogger(10),
                      "/media/szz/Others/Self_study/Cpp/MyPro/LibuvCpp/log/",
                      "HttpServer",
                      1);

  uv::EventLoop loop;
  uv::http::HttpServer server(&loop);

  //example:  127.0.0.1:5432/test
  server.Get("/test", 
             [](Request& req, Response* resp)
             {
              resp->setVersion(HttpVersion::Http1_1);
              resp->setStatus(StatCode::OK, "OK");
              resp->addPair("Server", "libuvCpp");
              resp->setContent("test~");
             });

  // example:  127.0.0.1:5432/some
  server.Get("/some",
            [](Request& req, Response* resp)
            {         
              resp->setVersion(HttpVersion::Http1_1);
              resp->setStatus(StatCode::OK, "OK");
              resp->addPair("Server", "libuvCpp");
              resp->setContent(req.path());
            });

  //example:  127.0.0.1:5432/sum
  server.Get("/sum", 
            [](Request& req, Response* resp)
            { 
              resp->setVersion(HttpVersion::Http1_1);
              resp->setStatus(StatCode::OK, "OK");
              resp->addPair("Server", "libuvCpp");
              int num=0; 
              std::string str;
              try 
              { 
                num = 
                  std::stoi(req.urlParams("param1")) + std::stoi(req.urlParams("param2"));
                str =
                    std::string("num: " + std::to_string(num));
              }
              catch(...) 
              { 
                str = "Param is not num";
              }
              resp->setContent(str);
            });
  
  server.bindAndListen("0.0.0.0", 5432);
  loop.runLoop();
  return 0;
}
