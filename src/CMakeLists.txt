if (USE_RESYNTH_GIMP)
    set(RESYNTH_SOURCES 
        ./resynth_gimp/resynth.c
        ./resynth_gimp/imageSynth.c
        ./resynth_gimp/engine.c
        ./resynth_gimp/glibProxy.c
        ./resynth_gimp/engineParams.c
        ./resynth_gimp/imageFormat.c
        ./resynth_gimp/progress.c
    )
else()
    set(RESYNTH_SOURCES 
        ./resynth_c/resynth.c 
    )
endif(USE_RESYNTH_GIMP)

add_library(resynth 
    ${RESYNTH_SOURCES}
)

if (USE_RESYNTH_GIMP)
    target_compile_definitions(resynth PUBLIC SYNTH_LIB_ALONE)
endif(USE_RESYNTH_GIMP)

set_target_properties(resynth PROPERTIES 
    C_STANDARD 11
    C_STANDARD_REQUIRED ON
)

if (NOT WIN32)
target_link_libraries(resynth m)
endif ()

target_include_directories(resynth PUBLIC
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)
