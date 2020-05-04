#include "EventLoop.h"
#include "TcpClient.h"
#include "Connection.h"
#include "NanoLog.h"
#include "Buffer.h"

#include <string>

using namespace uv;

TcpClient::TcpClient(EventLoop* loop, bool tcpNoDelay)
: loop_(loop), 
  client_(std::make_shared<uv_tcp_t>()),
  req_(std::make_unique<uv_connect_t>()), 
  connection_(nullptr),
  onMessageCallback_(nullptr),
  connectStCallback_(nullptr),
  tcpNoDealy_(tcpNoDelay)
{ 
  req_->data = static_cast<void*>(this);
  uv_tcp_init(loop_->loop(), client_.get());
  if(tcpNoDealy_) 
  { 
    uv_tcp_nodelay(client_.get(), 1);
  }
}

TcpClient::~TcpClient() {
  if(connection_ && connection_->active()) {
    this->close(nullptr);
  }
}

void TcpClient::connect(const char* ip, int port) { 

  struct sockaddr_in addr;
  uv_ip4_addr(ip, port, &addr);

  uv_tcp_connect(req_.get(),  
                client_.get(), 
                (const sockaddr*)&addr, 
                [](uv_connect_t* handle, int status)
                { 
                  TcpClient* self = static_cast<TcpClient*>(handle->data);

                  if(status !=0) { 
                    LOG_INFO<<"[Client] Connection fail("<<uv_strerror(status)<<')';
                    self->onConnect(false);
                    return;
                  }
                  self->onConnect(true);
                });
}

void TcpClient::onConnect(bool successed) { 
  if(successed) { 
    struct sockaddr_storage addr;
    int len = sizeof(struct sockaddr_storage);
    uv_tcp_getpeername(client_.get(), (struct sockaddr*)&addr, &len);

    struct sockaddr_in* addr4 = (struct sockaddr_in*)&addr; 
    const char* ip = inet_ntoa(addr4->sin_addr);
    int16_t port   = htons(addr4->sin_port);

    std::string ipAndPort(ip);
    ipAndPort.push_back(':');
    ipAndPort.append(std::to_string(port)); 

    LOG_INFO<<"[Client] success to connect server("<<ipAndPort<<')';
    
    connection_ = std::make_shared<Connection>(loop_, ipAndPort, client_, true); 
    connection_->setMessageCallback(std::bind(&TcpClient::onMessage, 
                                              this,
                                              std::placeholders::_1,
                                              std::placeholders::_2,
                                              std::placeholders::_3));
    connection_->setConnectCloseCallback(std::bind(&TcpClient::oncloseConnection, 
                                                   this, 
                                                   std::placeholders::_1));
    ConnectStatusCallback(ConnectSt::ConnectSuccess);
  }
  else 
  {
    if(uv_is_active((uv_handle_t*)client_.get())) 
    {
      uv_read_stop((uv_stream_t*)client_.get());
    }

    if(uv_is_closing((uv_handle_t*)client_.get()) ==0)
    {
      client_->data = static_cast<void*>(this);
      ::uv_close((uv_handle_t*)client_.get(), 
                  [](uv_handle_t* handle)
                  {
                    TcpClient* self = static_cast<TcpClient*>(handle->data);
                    self->ifConnecFail();
                  });
    }
  }
}

void TcpClient::oncloseConnection(const std::string& name) {
  if(connection_) 
  { 
    connection_->setCloseCompleteCallback(std::bind(&TcpClient::onClose, 
                                                    this, 
                                                    std::placeholders::_1));
    connection_->close();
  }
}

void TcpClient::onClose(std::string& name) { 
  LOG_INFO<<"[Client] Close tcp client connection complete.";
  connectStCallback_(ConnectSt::ConnectClose);  
}

void TcpClient::ifConnecFail() { 
  ConnectStatusCallback(ConnectSt::ConnectFail);
}

void TcpClient::ConnectStatusCallback(ConnectSt status) { 
  if(connectStCallback_) 
  {
    connectStCallback_(status);
  }
}

void TcpClient::close(CloseCompleteCallback cb) {
  if(connection_) { 
    connection_->setCloseCompleteCallback([this, cb](std::string& name)
                                          { 
                                            if(cb)  cb(name);
                                          });
    connection_->close();
  }
  else 
  { 
    std::string empty;
    cb(empty);
  } 
}

void TcpClient::onMessage(ConnectionPtr connPtr, const char* buf, ssize_t size) { 
  if(onMessageCallback_)
  {
    onMessageCallback_(buf, size);
  }
}

void TcpClient::write(const char* buf, uint32_t size, WriteCompleteCallback cb) { 
  if(connection_) 
  { 
    connection_->write(buf, size, cb);
  }
  else if(cb)
  {
    LOG_INFO<<"[Client] try write a disconnected connection";
    WriteInfo info{ const_cast<char*>(buf), static_cast<size_t>(size), -1};
    cb(info);
  }
  else 
  {
    // do nothing
  }
}

void TcpClient::writeInLoop(const char* buf, uint32_t size, WriteCompleteCallback cb) { 
  if(connection_) 
  { 
    connection_->writeInLoop(buf, static_cast<size_t>(size), cb);
  }
  else if(cb)
  {
    LOG_INFO<<"[Client] try write a disconnected connection";
    WriteInfo info{ const_cast<char*>(buf), static_cast<size_t>(size), -1};
    cb(info);
  }
  else 
  {
    // do nothing
  }
}

std::shared_ptr<Buffer> TcpClient::getCurrentBuff() const { 
  if(connection_) 
  {
    return connection_->getPackBuffer();
  }
  return nullptr;
}