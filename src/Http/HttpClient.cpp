#include "src/Http/HttpClient.h" 
#include "src/NanoLog.h"

#include <string.h>

using namespace uv;
using namespace uv::http;

HttpClient::HttpClient(EventLoop* loop) 
: client_(std::make_unique<TcpClient>(loop)), 
  req_(nullptr),
  buffer_(std::make_unique<std::string>()),
  connected_(false),
  onResponseCallback_(nullptr)
{ 
  buffer_->resize(1024<<4);
}

HttpClient::~HttpClient() { 
  if(connected_) 
  {
    client_->close(nullptr);
  }
}

void HttpClient::sendRequest(const char* ip, int port, Request& req) { 
  buffer_->clear();
  req_ = std::make_shared<Request>(req);

  client_->setConnectStatusCallback(std::bind(&HttpClient::onConnectStatus, 
                                              this, 
                                              std::placeholders::_1));
  
  client_->setMessageCallback(std::bind(&HttpClient::onMessage, 
                                        this, 
                                        std::placeholders::_1, 
                                        std::placeholders::_2));
  client_->connect(ip,port);
}

void HttpClient::onConnectStatus(TcpClient::ConnectSt status) { 
  switch(status) {
  case TcpClient::ConnectSt::ConnectSuccess:
  {
    // 连接成功后，就要发送请求了
    connected_ = true;
    std::string data;
    req_->encode(data);
    client_->write(data.c_str(), data.size());
    break;
  }
  case TcpClient::ConnectSt::ConnectFail:
  {
    connected_ = false;
    onResponse(ParseResult::Fail, nullptr);
    break;
  }
  default:
  {
    connected_ = false;
    Response resp;
    if(resp.parse(*buffer_) == ParseResult::Success) 
    { 
      onResponse(ParseResult::Success, &resp); 
    }
    else 
    {
      onResponse(ParseResult::Fail, nullptr);
    }
  } // default 
  } // switch
}

void HttpClient::onMessage(const char* data, ssize_t size) { 
  uint32_t buffSize = static_cast<uint32_t>(buffer_->size());
  uint32_t dataSize = static_cast<uint32_t>(size) + buffSize;
  buffer_->resize(dataSize);

  // 将数据添加到 buffer_的末尾
  char* out = const_cast<char*>(buffer_->c_str()) + buffSize;
  ::memcpy(out, data, size);

  Response resp;
  ParseResult status = resp.parseAndComplete(*buffer_);
  if(status == ParseResult::Success) 
  { 
    onResponse(status, &resp);
  }
  else 
  {
    LOG_WARN<<"Parse http'response error";
    buffer_->clear();
  }
}

void HttpClient::onResponse(ParseResult status, Response* resp) { 
    if(onResponseCallback_) 
    {
      onResponseCallback_(status, resp);
    }
}
