cmake_minimum_required(VERSION 3.1)

project(dcue CXX)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
  set(HTTP_LIBS Wininet)
  set(HTTP_EXTRA_SOURCES http_wininet.cpp)
else()
  find_package(CURL 7.10 REQUIRED)
  set(HTTP_INCLUDES ${CURL_INCLUDE_DIRS})
  set(HTTP_LIBS ${CURL_LIBRARIES})
  set(HTTP_EXTRA_SOURCES http_curl.cpp)
endif()

add_library(dcue_core OBJECT
  cue.cpp
  filename_utility.cpp
  naming.cpp
  string_utility.cpp
)

add_executable(dcue
  $<TARGET_OBJECTS:dcue_core>
  discogs.cpp
  main.cpp
  ${HTTP_EXTRA_SOURCES}
)
target_include_directories(dcue PRIVATE ${HTTP_INCLUDES})
target_link_libraries(dcue ${HTTP_LIBS})

if(DEFINED ENV{DCUE_APP_KEY} AND DEFINED ENV{DCUE_APP_SECRET} AND
    DEFINED ENV{DCUE_APP_XORKEY})
  message("App keys found in environment, making an official build")
  configure_file(appkey.h.in appkey.h)
  target_compile_definitions(dcue PRIVATE DCUE_OFFICIAL_BUILD)
  target_include_directories(dcue PRIVATE ${CMAKE_BINARY_DIR})
endif()

add_executable(dcue_test
  $<TARGET_OBJECTS:dcue_core>
  test/main.cpp)
