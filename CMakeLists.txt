cmake_minimum_required(VERSION 3.13)

project(dcue CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
  add_compile_definitions(SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_TRACE)
endif()

if(${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU" OR
    ${CMAKE_CXX_COMPILER_ID} MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Werror)
elseif(MSVC)
  add_compile_options(/W3 /WX)
endif()

# work around stupid gcc 9 and 10 in release mode
if(${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU" AND
   ${CMAKE_CXX_COMPILER_VERSION} VERSION_LESS 11.0.0)
  add_compile_options(-Wno-maybe-uninitialized)
endif()

add_subdirectory(spdlog)
include_directories(nlohmann-json/single_include)

if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
  set(HTTP_LIBS Wininet)
  set(HTTP_EXTRA_SOURCES http_wininet.cpp)
else()
  find_package(CURL 7.10 REQUIRED)
  set(HTTP_INCLUDES ${CURL_INCLUDE_DIRS})
  set(HTTP_LIBS ${CURL_LIBRARIES})
  set(HTTP_EXTRA_SOURCES http_curl.cpp)
endif()

if(COVERAGE)
  if(${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
    add_compile_options(--coverage)
    add_link_options(--coverage)
  elseif(${CMAKE_CXX_COMPILER_ID} MATCHES "Clang")
    add_compile_options(-fprofile-instr-generate -fcoverage-mapping)
    add_link_options(-fprofile-instr-generate)
  else()
    message(FATAL_ERROR "Coverage not supported for compiler ${CMAKE_CXX_COMPILER_ID}")
  endif()
endif()

add_library(dcue_core OBJECT
  cue.cpp
  filename_utility.cpp
  naming.cpp
  string_utility.cpp
)
target_link_libraries(dcue_core PUBLIC spdlog::spdlog)

add_executable(dcue
  $<TARGET_OBJECTS:dcue_core>
  discogs.cpp
  main.cpp
  ${HTTP_EXTRA_SOURCES}
)
target_include_directories(dcue PRIVATE ${HTTP_INCLUDES} ${CMAKE_SOURCE_DIR})
target_link_libraries(dcue PRIVATE ${HTTP_LIBS} dcue_core)

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
target_link_libraries(dcue_test PRIVATE dcue_core)
