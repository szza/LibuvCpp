#include <string>
#include <iostream>
#include <string.h>

#include <string>
#include <memory>
#include <unordered_map>

int main(int argc, char const *argv[]) {
  std::cout<<std::boolalpha;
  
  std::shared_ptr<int> intPtr =std::make_shared<int>(10);
  {
    auto iptr =  intPtr;
    iptr.reset();
  }
  std::cout<<*intPtr<<std::endl;
  return 0;
}
