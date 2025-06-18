cmake_minimum_required(VERSION 3.31)
project(MessagingApp)

set(CMAKE_CXX_STANDARD 20)

add_executable(server server.cpp)
add_executable(client client.cpp)

# Her hedef için ayrı ayrı ws2_32 kütüphanesini linkle
target_link_libraries(server PRIVATE ws2_32)
target_link_libraries(client PRIVATE ws2_32)
