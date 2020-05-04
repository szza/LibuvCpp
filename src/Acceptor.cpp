#include "Acceptor.h"

using namespace uv;

Acceptor::Acceptor(EventLoop* loop, bool tcpNoDelay) 
: loop_(loop), 
  server_(new uv_tcp_t),
  listened_(false),
  tcpNoDelay_(tcpNoDelay),
  callback_(nullptr)
{
  uv_tcp_init(loop_->loop(), server_.get());
  if(tcpNoDelay_) { 
    uv_tcp_nodelay(server_.get(), 1);
  }

  server_->data = static_cast<void*>(this);
}

Acceptor::~Acceptor() { 
  if(uv_is_active((uv_handle_t*)server_.get())) { 
    uv_close((uv_handle_t*)server_.get(), 
             nullptr);
  }
}

int Acceptor::bind(const char* ip, int port) { 
  struct sockaddr addr;
  uv_ip4_addr(ip, port, (struct sockaddr_in*)&addr);
  int err = uv_tcp_bind(server_.get(), &addr, 0);
  return err;
}

int Acceptor::listen() { 
  auto newConnection = 
   [](uv_stream_t* server, int status)
    { 
      if(status < 0) { 
        return ;
      }

      Acceptor* self = static_cast<Acceptor*>(server->data);
      std::shared_ptr<uv_tcp_t> client = std::make_shared<uv_tcp_t>();
      uv_tcp_init(self->loop()->loop(), client.get());
      if(self->isTcpNoDelay()) 
      {
        uv_tcp_nodelay(client.get(), 1);
      }

      if(uv_accept(server, (uv_stream_t*)client.get()) ==0) 
      { 
        self->onNewConnect(client);
      }
      else 
      {
        ::uv_close((uv_handle_t*)client.get(), nullptr);
      }
    };


  int res = uv_listen((uv_stream_t*)server_.get(), 
                      128, 
                      newConnection
                      );
  if(res ==0) 
  { 
    listened_ = true;
  }

  return res;
}

void Acceptor::onNewConnect(std::shared_ptr<uv_tcp_t> client) { 
  if(callback_)
  {
    callback_(loop_, client);
  }
}