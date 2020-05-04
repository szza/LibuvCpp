#pragma once

#include <memory>

#include <stdint.h>

//Buffer(cycle) 循环数组缓存
//---------------------------------------
//  Null  |   byte   |  byte   |  Null
//---------------------------------------
//        ↑                      ↑
//   read position           write position

//---------------------------------------
//  byte   |   byte   |  byte   |  byte
//---------------------------------------
//         ↑          ↑
//   write position  read position

namespace uv {

class Buffer { 
public:
  Buffer(uint64_t bufferSize=kDefaultBufferSize);
  ~Buffer();
  
  bool append(const char* data, uint64_t size);
  bool readBufferN(std::string& dest, uint64_t N);
  void clearBufferN(uint64_t N);
  void clear();

  uint64_t readableSize()  const;
  uint64_t writeableSize() const;

private:
  struct BufferStruct { 
    uint64_t size;
    uint64_t part1;  
    uint64_t part2;  
  };

  void writeableInfo(BufferStruct& info) const ;
  void readableInfo(BufferStruct& info)  const;
  void adjustWriteIndex(uint64_t size);
  void adjustReadIndex(uint64_t size);

  static constexpr const uint64_t kDefaultBufferSize = 1024<<5;  // 32k

  uint8_t* buffer_;   // 能自动析构掉数组
  uint64_t writeIndex_;
  uint64_t readIndex_;
  uint64_t capacity_;
}; // buffer 
  
} // namespace  uv
