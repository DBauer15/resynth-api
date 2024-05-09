cmake_minimum_required(VERSION 3.22)

project(resynth)

add_executable(resynth
    resynth.c
)

set_target_properties(resynth PROPERTIES 
    C_STANDARD 11
    C_STANDARD_REQUIRED ON
)

target_link_libraries(resynth m)
