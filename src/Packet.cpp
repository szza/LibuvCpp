#include "Packet.h"
#include "Buffer.h"

#include <string.h> // memcpy

using namespace uv;

Packet::Endian Packet::endian_ = Packet::Endian::LittleEndian;

Packet::Packet()
: dataSize_(0) 
{ }

bool Packet::readFromBuffer(Buffer* srcBuffer, Packet& dest) { 
  std::string data;

  while(true) {   
    uint64_t size = srcBuffer->readableSize();
    if(size < kPacketMinSize) 
      return false;
    
    uint16_t dataSize; 
    srcBuffer->readBufferN(data, sizeof(uint16_t)+1); // head + datasize
    
    if((uint8_t)data[0] != kHeadByte) { 
      data.clear();
      srcBuffer->clearBufferN(1); // 忽略掉第一个字节，从下一个字节开始读
      continue;
    }

    unpackNum((const uint8_t*)data.c_str()+1, dataSize); // 读取的数据大小，放到 dataSize中
    uint16_t packetSize = dataSize + kPacketMinSize;     // 一个完整的包大小
    // 如果可读的字节数，小于一个包的大小，那么就不读取
    // 等待下次读取
    if(size < packetSize) { 
      return false;
    }
    srcBuffer->clearBufferN(sizeof(dataSize) +1); // 已经正确读取的部分，划过
    // 将数据体+尾部读取到data中
    srcBuffer->readBufferN(data, dataSize +1);

    if((uint8_t)data.back() == kTailByte) { 
      srcBuffer->clearBufferN(dataSize +1);
      break;
    }
  } // while-end

  dest.swap(data);
  return true;
}

void Packet::pack(const char* data, uint16_t size) { 
  dataSize_ = size;
  buffer_.resize(size + kPacketMinSize);

  buffer_[0] = kHeadByte;
  packNum(&buffer_[1], size);
  ::memcpy(&buffer_[3], data, size);
  buffer_.back() = kTailByte;
}

void Packet::swap(std::string& str) {
  buffer_.swap(str);
  dataSize_ = static_cast<uint16_t>(buffer_.size() - kPacketMinSize);
}

template<typename Numeric>
void Packet::unpackNum(const uint8_t* data, Numeric& num) { 
  num=0; 
  int size = sizeof(Numeric);
  if(endian_ ==Endian::BigEndian) { 
    for(int i=0; i < size; ++i) { 
      num <<=8; 
      num |= data[i]; 
    }
  }
  else 
  {
    for(int i=size-1; i >=0; --i) { 
      num <<=8;
      num |= data[i];
    }
  }
}

template<typename Numeric>
void Packet::packNum(char* data, Numeric num) { 
  // 保证 data[0] 是最低为， 
  int size = sizeof(Numeric); 
  if(endian_ == Endian::BigEndian) { 
    for(int i=size-1; i>=0; --i) { 
      data[i] = num & 0xff; // 每次取出低8位
      num >>=8;
    }
  }
  else 
  {
    for(int i=0; i < size; ++i) { 
      data[i] = num & 0xff;
      num >>=8;
    }
  }
}