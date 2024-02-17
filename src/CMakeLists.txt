cmake_minimum_required(VERSION 3.1...3.27)


project(
  database-cw
  VERSION 1.0
  LANGUAGES CXX)

# set(CMAKE_CXX_FLAGS "-g -Werror -Wall -Wextra -pedantic -Wno-error=unused-variable -Wno-error=unused-parameter " )
# set(CMAKE_CXX_FLAGS_ASAN "-g -fsanitize=address -fsanitize=pointer-compare -fsanitize=pointer-subtract -fsanitize=leak -fsanitize=undefined -fsanitize-address-use-after-scope"
#                    CACHE STRING "Compiler flags in asan build"
#                    FORCE)
set(CMAKE_CXX_STANDARD 17)


# set(Boost_USE_STATIC_LIBS ON)
# find_package(Boost COMPONENTS program_options REQUIRED)
# include_directories(${BOOST_INCLUDE_DIRS})


# set(SOURCES )
add_executable(database main.cpp)
# target_link_libraries(database ${Boost_LIBRARIES})
# target_include_directories(database PRIVATE "./src/")







