cmake_minimum_required(VERSION 3.10)

project(socket_sample)

# Add pthread library
find_package(Threads REQUIRED)

# Add executable target
add_executable(socket_sample socket_sample.cpp)

# Link pthread library
target_link_libraries(socket_sample PRIVATE Threads::Threads)

# Enable debugging
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")


