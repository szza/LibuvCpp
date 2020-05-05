#include "src/Http/HttpServer.h"
#include "src/NanoLog.h"
#include "src/Buffer.h"

using namespace uv;
using namespace uv::http;

HttpServer::HttpServer(EventLoop* loop) 
: TcpServer(loop)
{ 
  this->setkeepLive(true);
  this->setMessageCallback(std::bind(&HttpServer::onMessageRecv, 
                                     this,
                                     std::placeholders::_1,
                                     std::placeholders::_2,
                                     std::placeholders::_3));
}

void HttpServer::onMessageRecv(ConnectionPtr connPtr, const char* buf, ssize_t size) { 
  LOG_INFO<<"[HttpServer] Receive data:\n"<<buf;

  std::shared_ptr<Buffer> buffer = connPtr->getPackBuffer();
  if(buffer ==nullptr) 
  {
    LOG_WARN<<"[Http Server] <class Connection> fail to create <class Buffer> object.";
    return;
  }

  buffer->append(buf, size);

  std::string dest;
  buffer->readBufferN(dest, buffer->readableSize());
  
  Request req;
  ParseResult status = req.parseAndComplete(dest);

  if(status == ParseResult::Error) {
    LOG_WARN<<"[HttpServer] Parse Error.";
    buffer->clear(); 
  }
  else if(status== ParseResult::Success) {
    buffer->clear();
    
    OnHttpReqCallback callback;
    auto& path   = req.path();
    auto& reqMap = route_[req.methodToNum()];
   
    if(reqMap.count(path) && (callback = reqMap[path])) {

      const std::string& connName = connPtr->name();
      Response resp;
      callback(req, &resp);

      std::string respInfo;
      resp.encode(respInfo);
      LOG_INFO<<"[HttpServer] Response data:\n"<<respInfo;

      connPtr->write(respInfo.c_str(), 
                      respInfo.size(), 
                      [this, connName](WriteInfo& )
                      { 
                        closeConnection(connName);
                      });
    }
    else 
    {
      if(reqMap.count(path) ==0)
      {
        LOG_WARN<<"[HttpServer]"<<"Request("<<req.methodToStr(req.method())<<" "<<path<<")not exist";
      }
      else 
      {
        LOG_WARN<<"[HttpServer] should set response callback for this request";
      }
    }
  }
}


void HttpServer::Get(std::string path, OnHttpReqCallback callback)
{ 
  route_[0].emplace(path, std::move(callback));
}


void HttpServer::Post(std::string path, OnHttpReqCallback callback)
{ 
  route_[1].emplace(path, std::move(callback));
}


void HttpServer::Head(std::string path, OnHttpReqCallback callback)
{ 
  route_[2].emplace(path, std::move(callback));
}


void HttpServer::Put(std::string path, OnHttpReqCallback callback)
{ 
  route_[3].emplace(path, std::move(callback));
}


void HttpServer::Delete(std::string path, OnHttpReqCallback callback)
{ 
  route_[4].emplace(path, std::move(callback));
}


void HttpServer::Connect(std::string path, OnHttpReqCallback callback)
{ 
  route_[5].emplace(path, std::move(callback));
}


void HttpServer::Options(std::string path, OnHttpReqCallback callback)
{   
  route_[6].emplace(path, std::move(callback));
}


void HttpServer::Trace(std::string path, OnHttpReqCallback callback)
{
  route_[7].emplace(path, std::move(callback));
}


void HttpServer::Patch(std::string path, OnHttpReqCallback callback)
{
  route_[8].emplace(path, std::move(callback));
}