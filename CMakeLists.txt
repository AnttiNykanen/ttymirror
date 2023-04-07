cmake_minimum_required(VERSION 3.11..3.25)

project(
  ttymirror
  LANGUAGES C
  VERSION 0.1
)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -pedantic")

add_subdirectory(src)
