cmake_minimum_required(VERSION 3.10)

get_cmake_property(_variableNames VARIABLES)
list (SORT _variableNames)
foreach (_variableName ${_variableNames})
    message(STATUS "${_variableName}=${${_variableName}}")
endforeach()

# set the project name
project(C2 VERSION 1.0)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(SOURCE_FILES
    ${PROJECT_SOURCE_DIR}/backtrace.cpp
    ${PROJECT_SOURCE_DIR}/board.cpp
    ${PROJECT_SOURCE_DIR}/C2_untittest.cpp
    ${PROJECT_SOURCE_DIR}/castling_state.cpp
    ${PROJECT_SOURCE_DIR}/chessfuncs.cpp
    ${PROJECT_SOURCE_DIR}/game.cpp
    ${PROJECT_SOURCE_DIR}/mate_in.cpp
    ${PROJECT_SOURCE_DIR}/move.cpp
    ${PROJECT_SOURCE_DIR}/movelist.cpp
    ${PROJECT_SOURCE_DIR}/piece.cpp
    ${PROJECT_SOURCE_DIR}/player.cpp
    ${PROJECT_SOURCE_DIR}/position.cpp
    ${PROJECT_SOURCE_DIR}/position_reader.cpp
    ${PROJECT_SOURCE_DIR}/square.cpp
    ${PROJECT_SOURCE_DIR}/squarelist.cpp
)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")


# add the executable
add_executable(C2 C2.cpp ${SOURCE_FILES})

configure_file(C2Config.h.in C2Config.h)

target_include_directories(C2 PUBLIC
                           "${PROJECT_BINARY_DIR}"
                           )

target_link_libraries(C2
  pthread
)
