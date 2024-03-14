set(SOURCE_FILES
    bitboard.cpp
    bitboard_make_move.cpp
    bitboard_movegen.cpp
    bitboard_with_utils.cpp
    chessfuncs.cpp
    cmd_line.cpp
    game.cpp
    shared_ostream.cpp
)

#include_directories(${CMAKE_SOURCE_DIR}/src)

# add the executable
add_executable(C2-chessengine C2.cpp ${SOURCE_FILES})

add_library(C2 ${SOURCE_FILES})

# To have a look at the CMake variables:
get_cmake_property(_variableNames VARIABLES)
list (SORT _variableNames)
foreach (_variableName ${_variableNames})
    message(STATUS "${_variableName}=${${_variableName}}")
endforeach()

