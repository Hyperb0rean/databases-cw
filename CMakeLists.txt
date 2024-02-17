cmake_minimum_required(VERSION 3.1...3.27)


project(
  database-cw
  VERSION 1.0
  LANGUAGES CXX)

set(CMAKE_CXX_FLAGS "-g -Werror -Wall -Wextra -pedantic -Wno-error=unused-variable -Wno-error=unused-parameter " )
set(CMAKE_CXX_FLAGS_ASAN "-g -fsanitize=address -fsanitize=pointer-compare -fsanitize=pointer-subtract -fsanitize=leak -fsanitize=undefined -fsanitize-address-use-after-scope"
                   CACHE STRING "Compiler flags in asan build"
                   FORCE)
set(CMAKE_CXX_STANDARD 23)


set(SOURCES )
add_executable(database main.cpp ${SOURCES})
# target_include_directories(database PRIVATE "./src/")





