add_executable(resynthcli
    resynthcli.c
)

target_link_libraries(resynthcli PUBLIC
    resynth
)
