cmake_minimum_required(VERSION 3.14)
project(rvec LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(rvec INTERFACE)
target_include_directories(rvec INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_executable(rvec_demo src/main.cpp)
target_link_libraries(rvec_demo PRIVATE rvec)
