cmake_minimum_required(VERSION 3.1)

project(dcue CXX)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(WIN32)
  set(CURL_PATH "" CACHE PATH "Path to libcurl build files")
  link_directories("${CURL_PATH}/lib")
  set(CURL_INCLUDE_DIRS "${CURL_PATH}/include")
  set(CURL_LIBRARIES libcurl)
else()
  find_package(CURL 7.10 REQUIRED)
endif()

include_directories(${CURL_INCLUDE_DIRS})
add_executable(dcue
  cue.cpp
  dcue.cpp
  discogs.cpp
  filename_utility.cpp
  http.cpp
  json_parser.cpp
  main.cpp
  naming.cpp
  string_utility.cpp)
target_link_libraries(dcue ${CURL_LIBRARIES})
