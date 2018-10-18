cmake_minimum_required(VERSION 3.12)
project(Networking_Project C)

set(CMAKE_C_STANDARD 99)

add_executable(Networking_Project receiver.c receiver.h packet_implem.c packet_interface.h network.c network.h utils.c utils.h sorted_queue.c sorted_queue.h)