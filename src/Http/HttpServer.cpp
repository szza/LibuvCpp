#include "src/Http/HttpServer.h"
#include "src/NanoLog.h"
#include "src/Buffer.h"

using namespace uv;
using namespace uv::http;

HttpServer::HttpServer(EventLoop* loop) 
: TcpServer(loop)
{ 
  this->setMessageCallback(std::bind(&HttpServer::onMessage, 
                                     this,
                                     std::placeholders::_1,
                                     std::placeholders::_2,
                                     std::placeholders::_3));
}

void HttpServer::onMessage(ConnectionPtr connPtr, const char* buf, ssize_t size) { 
  std::shared_ptr<Buffer> packBuff = connPtr->getPackBuffer();
  if(packBuff ==nullptr) 
  {
    LOG_WARN<<"[Http Server] <class Connection> fail to create <class Buffer> object.";
    return;
  }
  packBuff->append(buf, size);

  std::string dest;
  Request req;

  packBuff->readBufferN(dest, packBuff->readableSize());
  packBuff->clear();
  ParseResult status = req.parseAndComplete(dest);

  
  if(status== ParseResult::Success) {
    OnHttpReqCallback callback; 

    if(route_[req.methodToNum()].get(req.path(), callback) && callback) {
        const std::string& connName = connPtr->name();
        Response resp;
        std::string respData;

        callback(req, &resp);
        resp.encode(respData);
        LOG_INFO<<"response data: "<<respData;

        connPtr->write(respData.c_str(), 
                       respData.size(), 
                       [this, connName](WriteInfo& )
                       { 
                         closeConnection(connName);
                       });
    }
  }
  
}

template<typename String>
void HttpServer::Get(String&& path, OnHttpReqCallback callback)
{
    route_[RequestMethon::Get].set(path, callback);
}

template<typename String>
void HttpServer::Post(String&& path, OnHttpReqCallback callback)
{
    route_[RequestMethon::Post].set(path, callback);
}

template<typename String>
void HttpServer::Head(String&& path, OnHttpReqCallback callback)
{
    route_[RequestMethon::Head].set(path, callback);
}

template<typename String>
void HttpServer::Put(String&& path, OnHttpReqCallback callback)
{
    route_[RequestMethon::Put].set(path, callback);
}

template<typename String>
void HttpServer::Delete(String&& path, OnHttpReqCallback callback)
{
    route_[RequestMethon::Delete].set(path, callback);
}

template<typename String>
void HttpServer::Connect(String&& path, OnHttpReqCallback callback)
{
    route_[RequestMethon::Connect].set(path, callback);
}

template<typename String>
void HttpServer::Options(String&& path, OnHttpReqCallback callback)
{
    route_[RequestMethon::Options].set(path, callback);
}

template<typename String>
void HttpServer::Trace(String&& path, OnHttpReqCallback callback)
{
    route_[RequestMethon::Trace].set(path, callback);
}

template<typename String>
void HttpServer::Patch(String&& path, OnHttpReqCallback callback)
{
    route_[RequestMethon::Patch].set(path, callback);
}