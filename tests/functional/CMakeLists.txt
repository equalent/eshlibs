set(functests_sources
    main.c
    condparser.c
)

add_executable(functests ${functests_sources})

target_include_directories(functests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../..)