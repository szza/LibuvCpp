#include "Connection.h"
#include "EventLoop.h"
#include "TcpServer.h"
#include "Acceptor.h"
#include "NanoLog.h"

using namespace uv;

TcpServer::TcpServer(EventLoop* loop, bool tcpNoDelay)
: loop_(loop),
  server_(new Acceptor(loop, tcpNoDelay)),
  onMessageCallback_(nullptr),
  onNewConnectionCallback_(nullptr),
  onCloseConnectionCallback_(nullptr),
  timerwheel_(loop),
  keepLive_(false)
{ }

TcpServer::~TcpServer() { 

}

int TcpServer::bindAndListen(const char* ip, int port) { 
  int err = server_->bind(ip, port);
  if(err !=0) { 
    LOG_WARN<<"[Server] Fail to bind ("<<uv_strerror(err)<<")--"<<ip <<':'<<port<<'\n';
    return err;
  }
  server_->setNewConnectionCallback(std::bind(&TcpServer::onAccept, 
                                              this, 
                                              std::placeholders::_1,
                                              std::placeholders::_2));
  timerwheel_.start();
  err = server_->listen();
  if(err !=0) 
  { 
    LOG_WARN<<"[Server] Fail to listen("<<uv_strerror(err)<<")--"<<ip <<':'<<port<<'\n';
  }
 
  return err;
}

void TcpServer::onAccept(EventLoop* loop, std::shared_ptr<uv_tcp_t> client) {
  struct  sockaddr_storage addr; 
  int len = sizeof(struct sockaddr_storage);
  uv_tcp_getpeername(client.get(), (struct sockaddr*)&addr, &len);

  struct sockaddr_in* addr4 = (struct sockaddr_in *)&addr;
  const char* ip = inet_ntoa(addr4->sin_addr);
  uint16_t port  = htons(addr4->sin_port);

  std::string ipAndPort(ip);
  ipAndPort.push_back(':');
  ipAndPort.append(std::to_string(port)); 

  LOG_INFO<<"[Server] New Connection("<<ipAndPort<<')';

  std::shared_ptr<Connection> newConn = std::make_shared<Connection>(loop_, ipAndPort, client, true, keepLive_);
  if(newConn) { 
    newConn->setMessageCallback(std::bind(&TcpServer::onMessage, 
                                          this, 
                                          std::placeholders::_1,
                                          std::placeholders::_2,
                                          std::placeholders::_3));
    newConn->setConnectCloseCallback(std::bind(&TcpServer::closeConnection, 
                                                this, 
                                                std::placeholders::_1));

    addConnection(ipAndPort, newConn);
    timerwheel_.insert(newConn);
    if(onNewConnectionCallback_)
    {
      onNewConnectionCallback_(newConn);
    }     
  }
  else 
  {
    LOG_WARN<<"[Server] Create connection fail() "<<ipAndPort<<')';
  }
}

void TcpServer::onMessage(ConnectionPtr connPtr, const char* buf, size_t size) { 
  if(onMessageCallback_)
  {
    onMessageCallback_(connPtr, buf, size);
  }
  timerwheel_.insert(connPtr);
}

void TcpServer::closeConnection(const std::string& name) { 
  auto conn = getConnection(name);
  if(conn == nullptr) 
    return;
  
  conn->setConnectCloseCallback([this](const std::string& name)
                                { 
                                  auto conn = this->getConnection(name);
                                  if(conn ==nullptr) 
                                    return;
                                  
                                  if(this->onCloseConnectionCallback_) 
                                  {
                                    this->onCloseConnectionCallback_(conn);
                                  }
                                  this->removeConnection(name);
                                });
  conn->close();
}

void TcpServer::addConnection(const std::string& name, ConnectionPtr connPtr) { 
  nameConns_.insert({name, connPtr});
}

void TcpServer::removeConnection(const std::string& name) { 
  nameConns_.erase(name);
}

ConnectionPtr TcpServer::getConnection(const std::string& name) { 
  auto pos = nameConns_.find(name);
  if(pos == nameConns_.end()) { 
      return nullptr;
  }

  return pos->second;
}


void TcpServer::write(ConnectionPtr connPtr,  const char* buf, uint32_t size, WriteCompleteCallback cb) { 
  if(connPtr) 
  { 
    connPtr->write(buf, size, cb);
  }
  else if(cb) 
  {
    LOG_WARN<<"[Server] try write a disconnected connection.\n";
    WriteInfo info {const_cast<char*>(buf), size, -1};
    cb(info);
  }
}

void TcpServer::write(std::string& name, const char* buf, uint32_t size, WriteCompleteCallback cb) { 
  this->write(getConnection(name), buf, size, cb);
}


void TcpServer::writeInLoop(ConnectionPtr connPtr, const char* buf, uint32_t size, WriteCompleteCallback cb) { 
  if(connPtr) 
  { 
    connPtr->writeInLoop(buf, size, cb);
  }
  else if(cb) 
  {
    LOG_WARN<<"[Server] try write a disconnected connection.\n";
    WriteInfo info { const_cast<char*>(buf), size, -1};
    cb(info);
  }
}

void TcpServer::writeInLoop(std::string& name, const char* buf, uint32_t size, WriteCompleteCallback cb) { 
  this->writeInLoop(getConnection(name), buf, size, cb);
}
