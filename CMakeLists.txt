cmake_minimum_required(VERSION 3.22)

project(resynth)

option(USE_RESYNTH_GIMP "Compile the feature-rich GIMP version of resynth." ON)

add_subdirectory(src)
add_subdirectory(apps)
