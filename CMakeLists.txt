cmake_minimum_required(VERSION 3.30)

project(Chess VERSION 1.0
                  DESCRIPTION "Chess Engine"
                  LANGUAGES C)

link_directories(${CMAKE_SOURCE_DIR}/gdslib/lib)
include_directories(${CMAKE_SOURCE_DIR}/raylib/include ${CMAKE_SOURCE_DIR}/gdslib/include)

add_library(gdslib STATIC IMPORTED)
set_target_properties(gdslib PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/gdslib/lib/libgdslib.a)

link_directories(${CMAKE_SOURCE_DIR}/raylib/lib)
add_library(raylib STATIC IMPORTED)
set_target_properties(raylib PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/raylib/lib/libraylib.a)

add_executable(chess
    chess.c
    chess.h
    arena.h
    data.c
    data.h
    moves.c
)

target_compile_options(chess PRIVATE -Wall -Wextra -g)
target_link_libraries(chess gdslib raylib m)
