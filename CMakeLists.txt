cmake_minimum_required(VERSION 3.7)
project(BackwardsChess)

find_package(SDL2 REQUIRED)

add_executable(BackwardsChess "main.cpp")
target_link_libraries(BackwardsChess SDL2::SDL2 SDL2::SDL2main)
