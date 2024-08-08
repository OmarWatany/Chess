cmake_minimum_required(VERSION 3.30)

project(Chess VERSION 1.0
                  DESCRIPTION "Chess Game"
                  LANGUAGES C)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

link_directories(${CMAKE_SOURCE_DIR}/gdslib/lib)
include_directories(
    ${CMAKE_SOURCE_DIR}/raylib/include
    ${CMAKE_SOURCE_DIR}/gdslib/include
    ${CMAKE_SOURCE_DIR}/include
)

add_library(gdslib STATIC IMPORTED)
set_target_properties(gdslib PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/gdslib/lib/libgdslib.a)

link_directories(${CMAKE_SOURCE_DIR}/raylib/lib)
add_library(raylib STATIC IMPORTED)
set_target_properties(raylib PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/raylib/lib/libraylib.a)

add_executable(chess
    src/chess.c
    src/data.c
    src/moves.c
    src/networking.c
    include/chess.h
    include/arena.h
    include/data.h
)

target_compile_options(chess PRIVATE -Wall -Wextra -g)
target_link_libraries(chess gdslib raylib m)

add_executable(server
    src/server.c
    include/chess.h
    include/arena.h
    include/data.h
)

target_compile_options(server PRIVATE -Wall -Wextra -g)
target_link_libraries(server gdslib raylib m)
