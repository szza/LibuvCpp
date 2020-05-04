#include <string>
#include <iostream>
#include <string.h>

#include <string>

int main(int argc, char const *argv[]) {
  
  const char* src = "Hello Cpp";

  std::string buf;
  buf.resize(20);
  ::memcpy(&buf[0], src, ::strlen(src)+1);
  std::cout<<"resize: "<<buf<<std::endl;

  std::string buff;
  buff.reserve(20);
  
  ::memcpy(&buff[0], src, ::strlen(src)+1);
  std::cout<<"reserve: "<<buff<<std::endl;
  return 0;
}
