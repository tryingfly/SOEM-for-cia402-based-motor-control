cmake_minimum_required(VERSION 3.15)

project(hello-soem)

find_package(Threads REQUIRED)

include_directories("/usr/local/include/soem")


add_executable(helloSOEM hello_soem.cc)
target_link_libraries(helloSOEM soem Threads::Threads)