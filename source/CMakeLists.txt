set(LOCAL_FILES
    main.c
    server.c
    server.h
    sys_info.c
    sys_info.h
)

add_executable(myoj-judge-server ${LOCAL_FILES})

install(TARGETS myoj-judge-server)
