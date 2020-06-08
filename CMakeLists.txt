# Copyright (c) 2020 tevador <tevador@gmail.com>
# See LICENSE for licensing information

cmake_minimum_required(VERSION 2.8.8)

project(equix)

add_definitions(-DHASHX_SIZE=8)

add_subdirectory("hashx")

set(equix_sources
src/context.c
src/equix.c
src/solver.c)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
  message(STATUS "Setting default build type: ${CMAKE_BUILD_TYPE}")
endif()

add_library(equix_static STATIC ${equix_sources})
set_property(TARGET equix_static PROPERTY POSITION_INDEPENDENT_CODE ON)
set_property(TARGET equix_static PROPERTY PUBLIC_HEADER include/equix.h)
set_target_properties(equix_static PROPERTIES OUTPUT_NAME equix)
target_compile_definitions(equix_static PRIVATE HASHX_STATIC)
include_directories(equix_static
  include/
  hashx/include/
  hashx/src/)
target_link_libraries(equix_static
  PRIVATE hashx_static)

include(GNUInstallDirs)
install(TARGETS equix_static
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

if(NOT Threads_FOUND AND UNIX AND NOT APPLE)
  set(THREADS_PREFER_PTHREAD_FLAG ON)
  find_package(Threads)
endif()

add_executable(equix-bench
  src/bench.c
  hashx/src/hashx_thread.c)
include_directories(equix-bench
  include/
  hashx/src/)
target_link_libraries(equix-bench
  PRIVATE equix_static
  PRIVATE ${CMAKE_THREAD_LIBS_INIT})
