cmake_minimum_required(VERSION 3.10.1)
project(packaging)

set(src "src")
set(foo_src "src/foo.cpp")
add_library(foo SHARED ${foo_src})
