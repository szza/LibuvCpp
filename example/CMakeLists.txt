cmake_minimum_required (VERSION 3.0)
project (LiubuvCpp)

add_executable(async ./base/async_.cc)
target_link_libraries(async libuvCpp uv)

add_executable(helloWorld ./HelloWorld/HelloWorld.cc)
target_link_libraries(helloWorld libuvCpp uv pthread)

add_executable(server ./echo/Server.cc)
add_executable(client ./echo/Client.cc)
target_link_libraries(server libuvCpp uv pthread)
target_link_libraries(client libuvCpp uv pthread)

add_executable(radix ./RadixTree/radixTree.cpp)
target_link_libraries(radix libuvCpp uv pthread)



