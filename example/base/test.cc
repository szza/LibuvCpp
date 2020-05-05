#include <string>
#include <iostream>
#include <string.h>

#include <string>
#include <unordered_map>

int main(int argc, char const *argv[]) {
  std::cout<<std::boolalpha;
  
  std::unordered_map<const char*, int> map_;

  const char* src1 = "some";
  const char* src2 = "some";
  std::string src3("some");

  map_.emplace(src1, 1);

  auto iter2 = map_.find(src2);
  auto iter3 = map_.find(src3.c_str());


  std::cout<<"can find:"<<src1<<" ? "<<(iter2 != map_.end())<<std::endl;
  std::cout<<"can find:"<<src1<<" ? "<<(iter3 != map_.end())<<std::endl;

}
