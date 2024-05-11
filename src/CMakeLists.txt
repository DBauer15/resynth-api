add_library(resynth
    resynth.c
)

set_target_properties(resynth PROPERTIES 
    C_STANDARD 11
    C_STANDARD_REQUIRED ON
)

target_link_libraries(resynth m)

target_include_directories(resynth PUBLIC
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)
