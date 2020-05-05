cmake_minimum_required (VERSION 3.5)
project (LiubuvCpp)

add_executable(async ./base/async_.cc)
target_link_libraries(async LibuvCpp uv)

add_executable(helloWorld ./HelloWorld/HelloWorld.cc)
target_link_libraries(helloWorld LibuvCpp uv pthread)

add_executable(server ./echo/Server.cc)
add_executable(client ./echo/Client.cc)
target_link_libraries(server LibuvCpp uv pthread)
target_link_libraries(client LibuvCpp uv pthread)

add_executable(httpServer_test ./Http/Server.cpp)
target_link_libraries(httpServer_test LibuvCpp HttpLib uv pthread)



