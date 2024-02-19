cmake_minimum_required(VERSION 3.1...3.27)


project(
  database-cw
  VERSION 1.0
  LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -lpqxx -lpq")
set(Boost_USE_MULTITHREADED ON)


find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(Boost COMPONENTS system REQUIRED)
find_package(CURL)
find_package(libpqxx REQUIRED)
find_package(PostgreSQL REQUIRED)
include_directories(/usr/local/include ${OPENSSL_INCLUDE_DIR} ${Boost_INCLUDE_DIR}) 
if (CURL_FOUND)
    include_directories(${CURL_INCLUDE_DIRS})
    add_definitions(-DHAVE_CURL)
endif()
set(SOURCES 
  main.cpp
  main.hpp
  util.hpp)
add_executable(database ${SOURCES})
target_include_directories(database PRIVATE ".")
target_link_libraries(database /usr/local/lib/libTgBot.a ${CMAKE_THREAD_LIBS_INIT} ${OPENSSL_LIBRARIES} ${Boost_LIBRARIES} ${CURL_LIBRARIES} libpqxx::pqxx_shared PostgreSQL::PostgreSQL)
