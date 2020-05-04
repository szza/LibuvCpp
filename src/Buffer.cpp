#include "Buffer.h"

#include <string.h>

using namespace uv;

Buffer::Buffer(uint64_t bufferSize)
: buffer_(new uint8_t[bufferSize]),
  writeIndex_(0),
  readIndex_(0),
  capacity_(bufferSize)
{ }

Buffer::~Buffer() { 
  delete[] buffer_;
}

bool Buffer::append(const char* data, uint64_t size) { 
  BufferStruct buff; 
  writeableInfo(buff); 
  // 可写的字节数 < 要写入的字节数
  if(buff.size < size) 
  { 
    return false;
  }

  if(size <= buff.part1) 
  { 
    ::memcpy(buffer_ + writeIndex_, data, size);
  }
  else 
  {
    ::memcpy(buffer_ + writeIndex_, data, buff.part1);
    ::memcpy(buffer_, data + buff.part1, size - buff.part1);
  }

  adjustWriteIndex(size);
  return true;
} 

bool Buffer::readBufferN(std::string& dest, uint64_t N) {
  BufferStruct buff; 
  readableInfo(buff);

  // 可读取的字节数 < 要读取的字节数
  if(readableSize() < N) 
  { 
    return false;
  }

  size_t oldSize = dest.size();
  dest.resize(oldSize + N);
  char* out = const_cast<char*>(dest.c_str()) + oldSize; // 指向新的数据地址

  if(N <= buff.part1) 
  { 
    ::memcpy(out, buffer_ + readIndex_, N);
  }
  else 
  {
    ::memcpy(out, buffer_ + readIndex_, buff.part1);
    ::memcpy(out + buff.part1, buffer_, N - buff.part1);
  }

  // 将下标调整留给使用者
  return true;
}

void Buffer::clearBufferN(uint64_t N) { 
  if( readableSize() < N) 
  {
    N = readableSize();
  }

  adjustReadIndex(N);
}

void Buffer::clear() {
  writeIndex_ =0; 
  readIndex_  =0;
}

uint64_t Buffer::writeableSize() const { 
  BufferStruct buff; 
  writeableInfo(buff);
  return buff.size;
}

void Buffer::writeableInfo(BufferStruct& buff) const {
  if(writeIndex_ < readIndex_) { 
    buff.part1 = readIndex_ - writeIndex_ -1;
    buff.part2 = 0;                         
  }
  else {
    buff.part1 = readIndex_ ==0 ? capacity_-writeIndex_-1 : capacity_ - writeIndex_;
    buff.part2 = readIndex_ ==0 ? 0 : readIndex_-1;
  }

  buff.size = buff.part1 + buff.part2;
} 

uint64_t Buffer::readableSize() const { 
  BufferStruct buff; 
  readableInfo(buff); 
  return buff.size;
}

void Buffer::readableInfo(BufferStruct& buff) const { 
  if(readIndex_ <= writeIndex_) { 
    buff.part1 = writeIndex_ - readIndex_; 
    buff.part2 = 0;
  }
  else { 
    buff.part1 = capacity_ - readIndex_;
    buff.part2 = writeIndex_;
  }
  
  buff.size = buff.part1 + buff.part2;
}

void Buffer::adjustWriteIndex(uint64_t size) { 
  if(capacity_ <  size) 
    return;

  writeIndex_ += size;
  if(writeIndex_ >= capacity_) { 
    writeIndex_ -= capacity_;
  }
}

void Buffer::adjustReadIndex(uint64_t size) {
  if(capacity_ < size) 
    return;
  
  readIndex_ += size;
  if(readIndex_ >= capacity_) { 
    readIndex_ -= capacity_;
  }
}