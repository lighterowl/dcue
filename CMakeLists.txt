cmake_minimum_required(VERSION 3.1)

project(dcue CXX)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(CURL 7.10 REQUIRED)

include_directories(${CURL_INCLUDE_DIRS})
add_executable(dcue
  cue.cpp
  discogs.cpp
  filename_utility.cpp
  http.cpp
  main.cpp
  naming.cpp
  string_utility.cpp)
target_link_libraries(dcue ${CURL_LIBRARIES})
