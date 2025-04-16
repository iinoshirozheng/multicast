cmake_minimum_required(VERSION 3.10)
project(stream_buffer VERSION 1.0.0 LANGUAGES CXX)

# Use C++11 standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set default build type to Release
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build" FORCE)
endif()

# Enable compiler warnings
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
elseif(MSVC)
  add_compile_options(/W4)
endif()

# Option for enabling code coverage
option(ENABLE_COVERAGE "Enable code coverage reporting" OFF)
if(ENABLE_COVERAGE AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  add_compile_options(--coverage -O0 -g)
  add_link_options(--coverage)
endif()

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/stream_buffer/include)

# Find required packages
find_package(Threads REQUIRED)

# Source files
file(GLOB_RECURSE SOURCES 
  "stream_buffer/src/*.cpp"
)

# Header files
file(GLOB_RECURSE HEADERS 
  "stream_buffer/include/*.h"
)

# Main executable
add_executable(stream_buffer ${SOURCES} ${HEADERS} stream_buffer/main.cpp)
target_link_libraries(stream_buffer PRIVATE Threads::Threads)

# Install targets
install(TARGETS stream_buffer
        RUNTIME DESTINATION bin)

# Testing
enable_testing()

# Add test subdirectory if it exists
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests/CMakeLists.txt")
  add_subdirectory(tests)
endif()

# Add documentation if Doxygen is available
find_package(Doxygen)
if(DOXYGEN_FOUND)
  set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/docs/Doxyfile.in)
  set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

  configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)

  add_custom_target(docs
    COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Generating API documentation with Doxygen"
    VERBATIM)
endif() 