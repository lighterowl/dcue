cmake_minimum_required(VERSION 3.1)

project(dcue CXX)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(CURL 7.10 REQUIRED)

add_executable(dcue
  cue.cpp
  discogs.cpp
  filename_utility.cpp
  http.cpp
  main.cpp
  naming.cpp
  string_utility.cpp)
target_include_directories(dcue PRIVATE ${CURL_INCLUDE_DIRS})
target_link_libraries(dcue ${CURL_LIBRARIES})
if(DEFINED ENV{DCUE_APP_KEY} AND DEFINED ENV{DCUE_APP_SECRET})
  configure_file(appkey.h.in appkey.h)
  target_compile_definitions(dcue PRIVATE DCUE_OFFICIAL_BUILD)
  target_include_directories(dcue PRIVATE ${CMAKE_BINARY_DIR})
endif()
