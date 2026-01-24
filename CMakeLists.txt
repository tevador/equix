# Copyright (c) 2020 tevador <tevador@gmail.com>
# See LICENSE for licensing information

cmake_minimum_required(VERSION 3.10)

set(EQUIX_VERSION 1)
set(EQUIX_VERSION_MINOR 1)
set(EQUIX_VERSION_PATCH 0)
set(EQUIX_VERSION_STR "${EQUIX_VERSION}.${EQUIX_VERSION_MINOR}.${EQUIX_VERSION_PATCH}")

project(equix)

add_definitions(-DHASHX_SIZE=8)

add_subdirectory("hashx")
add_subdirectory("hashwx")

set(equix_sources
src/context.c
src/equix.c
src/solver.c)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
  message(STATUS "Setting default build type: ${CMAKE_BUILD_TYPE}")
endif()

add_library(equix SHARED ${equix_sources})
set_property(TARGET equix PROPERTY POSITION_INDEPENDENT_CODE ON)
set_property(TARGET equix PROPERTY PUBLIC_HEADER include/equix.h)
include_directories(equix
  include/
  hashx/include/
  hashx/src/
  hashwx/include)
target_compile_definitions(equix PRIVATE HASHX_STATIC)
target_compile_definitions(equix PRIVATE HASHWX_STATIC)
target_compile_definitions(equix PRIVATE EQUIX_SHARED)
target_link_libraries(equix
  PRIVATE hashx_static
  PRIVATE hashwx_static)
set_target_properties(equix PROPERTIES VERSION ${EQUIX_VERSION_STR}
                                       SOVERSION ${EQUIX_VERSION})

add_library(equix_static STATIC ${equix_sources})
set_property(TARGET equix_static PROPERTY POSITION_INDEPENDENT_CODE ON)
set_target_properties(equix_static PROPERTIES OUTPUT_NAME equix)
target_compile_definitions(equix_static PRIVATE HASHX_STATIC)
target_compile_definitions(equix_static PRIVATE HASHWX_STATIC)
target_compile_definitions(equix_static PRIVATE EQUIX_STATIC)
include_directories(equix_static
  include/
  hashx/include/
  hashx/src/
  hashwx/include)
target_link_libraries(equix_static
  PRIVATE hashx_static
  PRIVATE hashwx_static)

include(GNUInstallDirs)
install(TARGETS equix equix_static
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

add_executable(equix-tests
  src/tests.c)
include_directories(equix-tests
  include/)
target_compile_definitions(equix-tests PRIVATE EQUIX_STATIC)
target_link_libraries(equix-tests
  PRIVATE equix_static)

if(NOT Threads_FOUND AND UNIX AND NOT APPLE)
  set(THREADS_PREFER_PTHREAD_FLAG ON)
  find_package(Threads)
endif()

add_executable(equix-bench
  src/bench.c
  hashx/src/hashx_thread.c
  hashx/src/hashx_time.c)
include_directories(equix-bench
  include/
  hashx/src/)
target_compile_definitions(equix-bench PRIVATE EQUIX_STATIC)
target_link_libraries(equix-bench
  PRIVATE equix_static
  PRIVATE ${CMAKE_THREAD_LIBS_INIT})
