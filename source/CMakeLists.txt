set(LOCAL_FILES
    main.c
    server.c
    server.h
)

add_executable(myoj-judge-server ${LOCAL_FILES})

install(TARGETS myoj-judge-server)
