cmake_minimum_required(VERSION 3.16)
project(sonars)

include(/home/tenderbook/Current_task/modules/src/codegen/generate_settings.cmake)

# Default to C++14
if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
endif()

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()


find_library(LIBRT rt)
if(NOT LIBRT)
    message(FATAL_ERROR "librt library not found")
endif()

find_library(PTHREAD pthread)
if(NOT PTHREAD)
    message(FATAL_ERROR "libpthread library not found")
endif()

find_library(MODBUS modbus)
if(NOT MODBUS)
    message(FATAL_ERROR "libmodbus library not found")
endif()

add_subdirectory(/home/tenderbook/rapidyaml ryml)

add_executable( sonars
                sonars.cpp
                /home/tenderbook/Current_task/modules/src/common/debout.cpp
                /home/tenderbook/Current_task/modules/src/common/debout.h
                /home/tenderbook/Current_task/modules/src/common/loggen.cpp
                /home/tenderbook/Current_task/modules/src/common/loggen.hpp
)

target_include_directories( sonars PRIVATE
                /home/tenderbook/Current_task/modules/src
                /home/tenderbook/Current_task/3rdparty/src/modbus/src
                /home/tenderbook/Current_task/3rdparty/src/modbus
                /home/tenderbook/Current_task/modules/src/common
                /home/tenderbook/rapidyaml/src
)




target_link_libraries(sonars  ${LIBRT} ${PTHREAD} ${MODBUS} rt ryml)


