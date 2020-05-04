#pragma once 

#include <string>
#include <stdint.h>

//Packet:
//------------------------------------------------
//  head  |  size   | data   |  end   |
// 1 byte | 2 bytes | N bytes| 1 byte |
//------------------------------------------------
//头部标志位  数据大小  数据    尾部标志位
// 包的大小最少是4个字节
// 包的整个大小 4 + 数据大小

namespace uv {

class Buffer;

class Packet { 
public:
  enum class Endian {BigEndian, LittleEndian};

   Packet();
  ~Packet() =default;

  void pack(const char* data, uint16_t size);
  void swap(std::string& str);

  static bool readFromBuffer(Buffer* buff, Packet& packet);
  static void setEndian(Endian endian) { endian_ = endian; }

  template<typename Numeric>
  static void unpackNum(const uint8_t* data, Numeric& num);
 
  template<typename Numeric>
  static void packNum(char* data, Numeric num);

  const char*        data()       const   { return buffer_.c_str() + 3; }
  const std::string& buffer()     const   { return buffer_;        }
  const uint16_t     size()       const   { return dataSize_;      }
  const uint64_t     packetSize() const   { return buffer_.size(); }

private:  
  constexpr static const uint8_t  kHeadByte      = 0x7f;
  constexpr static const uint8_t  kTailByte      = 0x7f;
  constexpr static const uint64_t kPacketMinSize = 4;

  static Endian endian_ ;
  std::string   buffer_;
  uint16_t      dataSize_;
}; 

} // namespace uv
