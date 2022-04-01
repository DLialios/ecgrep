cmake_minimum_required(VERSION 3.23)

project(ecgrep)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS True)

add_executable(ecgrep
    src/ecgrep.cpp
    src/matchfinder.cpp
    )

target_include_directories(ecgrep PRIVATE
    ${PROJECT_SOURCE_DIR}/include
    )

