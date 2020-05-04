#include "EventLoop.h"
#include "Connection.h"
#include "NanoLog.h"
#include "Buffer.h"

using namespace uv;

namespace { 

struct WriteReq { 
  uv_write_t req;
  uv_buf_t   buf;
  WriteCompleteCallback cb;
};

} // unname - namespace 

Connection::Connection(EventLoop* loop,const std::string& name, std::shared_ptr<uv_tcp_t> client, bool isConnected)
: loop_(loop),
  client_(client), 
  name_(std::make_shared<std::string>(name)),
  buffer_(std::make_shared<Buffer>()),
  onMessageCallback_(nullptr),
  onCloseCallback_(nullptr),
  closeCompleteCallback_(nullptr),
  connected_(isConnected)
{ 
  client_->data = static_cast<void*>(this);

  uv_read_start((uv_stream_t*)client_.get(), 
                [](uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf)
                {
                  Connection* self = static_cast<Connection*>(handle->data);
                  buf->base = self->mallocBuff(suggested_size);
                  buf->len  = suggested_size;
                },
                Connection::onMessageRecv);
}

Connection::~Connection() { 
  if(this->active()) { 
    this->close();
  }
}

void Connection::onMessageRecv(uv_stream_t* client, ssize_t nread, const uv_buf_t* buf) { 
  Connection* self = static_cast<Connection*>(client->data);
  if(nread >0) 
  { 
    self->onMessage(buf->base, nread);
  }
  else if(nread < 0) 
  { 
    self->setConnectStatus(false);
    LOG_WARN<<"[Connection] Message receive fail("<<uv_strerror(nread)<<')';
    if(nread != UV_EOF) { 
      self->onClientClose();  
      return;
    }

    uv_shutdown_t* sreq = new uv_shutdown_t;
    sreq->data = static_cast<void*>(self);
    uv_shutdown(sreq, 
                (uv_stream_t*)client, 
                [](uv_shutdown_t* req, int status)
                {
                  Connection* self = static_cast<Connection*>(req->data);
                  self->onClientClose();  
                  delete req;
                });
  }
  else
  {
    /* Everything OK, but nothing read. */
  }
}

void Connection::onMessage(const char* buffer, ssize_t size) {
  if(onMessageCallback_)
  {
    onMessageCallback_(shared_from_this(), buffer, size);
  }
  // delete[] buffer;
}

void Connection::onClientClose() { 
  if(onCloseCallback_)
  {
    onCloseCallback_(*name_);
  }
}

void Connection::close() { 
  connected_ = false;
  uv_tcp_t* stream = client_.get();
  if(uv_is_active((uv_handle_t*)stream)) 
  {
    uv_read_stop((uv_stream_t*)stream);
  }

  if(uv_is_closing((const uv_handle_t*)stream)==0) 
  {
    uv_close((uv_handle_t*)stream, 
              [](uv_handle_t* handle)
               {
                Connection* self = static_cast<Connection*>(handle->data);
                self->closeComplete();
               });
  }
  else 
  {
    closeComplete();
  }
}

void Connection::closeComplete() {
  if(closeCompleteCallback_)
  {
    closeCompleteCallback_(*name_);
  }
}

void Connection::write(const char* buf, ssize_t size, WriteCompleteCallback cb) { 
  int err=0; 
  if(connected_) { 
    // 此处需要使用 new 出来的对象，使其生命周期延长到回调函数中
    // 不能用栈对象或者智能指针
    WriteReq* req    = new WriteReq;
    uv_tcp_t* handle = client_.get();
    req->buf = uv_buf_init(const_cast<char*>(buf), static_cast<uint32_t>(size));
    req->cb  = cb;

    err = uv_write((uv_write_t*)req,   
                   (uv_stream_t*)handle, 
                   &req->buf, 
                   1, 
                   [](uv_write_t* req, int status)
                   {
                    WriteReq* w = (WriteReq*)req;
                    if(w->cb) {
                      WriteInfo info {const_cast<char*>(w->buf.base), w->buf.len, status};
                      w->cb(info);
                    }

                    delete req;
                  });

    if(err !=0) { 
      LOG_WARN<<"[Connection] Write data error: "<<uv_strerror(err);;
      if(cb) { 
        struct WriteInfo info {const_cast<char*>(buf),static_cast<size_t>(size), err};
        cb(info);
      }
    }  
  }
  else 
  {
    LOG_WARN<<"[Connection] Connection has disconnected.";
    if(cb) { 
      struct WriteInfo info {const_cast<char*>(buf), static_cast<size_t>(size), err};
      cb(info);
    }
  }
}


void Connection::writeInLoop(const char* buf, ssize_t size, WriteCompleteCallback cb) { 
  loop_->runTaskInLoop(
    [this, buf, size, cb]()
    {
      this->write(buf, size, cb);
    });
} 

char* Connection::mallocBuff(size_t size) { 
  data_.resize(size);
  return data_.data();
}

