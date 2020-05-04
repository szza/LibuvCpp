#include "src/libuvCpp.h"

using namespace uv::http;

void func1(Request& req, Response* resp) { 
  resp->setVersion(HttpVersion::Http1_1);
  resp->setStatus(StatCode::OK, "OK");
  resp->addHeaderPair(std::string("Server"), std::string("libuvCpp"));
  resp->swapContent(std::string("test~"));
}