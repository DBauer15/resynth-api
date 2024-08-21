cmake -B build . \
    -D CMAKE_BUILD_TYPE=Release \
    -D USE_RESYNTH_GIMP=ON

cmake --build build -j
