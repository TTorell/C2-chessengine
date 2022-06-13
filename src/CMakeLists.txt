cmake_minimum_required(VERSION 3.16)

set( CMAKE_VERBOSE_MAKEFILE TRUE )

# set the project name
project(C2 VERSION 1.0)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(SOURCE_FILES
    ${PROJECT_SOURCE_DIR}/bitboard.cpp
    ${PROJECT_SOURCE_DIR}/bitboard_make_move.cpp
    ${PROJECT_SOURCE_DIR}/bitboard_movegen.cpp
    ${PROJECT_SOURCE_DIR}/bitboard_with_utils.cpp
    ${PROJECT_SOURCE_DIR}/chessfuncs.cpp
    ${PROJECT_SOURCE_DIR}/cmd_line.cpp
    ${PROJECT_SOURCE_DIR}/game.cpp
    ${PROJECT_SOURCE_DIR}/movelog.cpp
    ${PROJECT_SOURCE_DIR}/shared_ostream.cpp
)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++latest /W4")
    # Default debug flags are OK in windows 
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2")
    # Tried /favour:{INTEL64} as well to make it faster on my own system,
    # but the build system didn't seem to understand, and It's hardly
    # applicable to everyone anyway.

else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++2a -m64 -march=native -mpopcnt -msse2 -DNDEBUG -pedantic -pedantic-errors -Wall -Wextra -Werror -Wcast-qual -Wshadow -fmessage-length=0 -Wabi-tag -D_GLIBCXX_USE_CXX11_ABI=0 -pthread")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -g3")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -g0")

    if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
    else()
        # nothing special for gcc at the moment
    endif()
endif()

# add the executable
add_executable(C2 C2.cpp ${SOURCE_FILES})

if(UNIX AND NOT APPLE)
    # for Linux, BSD, Solaris, Minix
    target_link_libraries(C2 pthread)
    
    # Don't know if the --stack flag has any effect on linkers in LINUX.
    # In Ubuntu it seems like you get approximately 8 megabytes as default.
    # You can change stacksize in the bash-shell from cmd-line with "ulimit -s <size>"
    # Tried, in vain, the following just to test anyhow:
    # set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--stack,10000") didn't work
    # or set_target_properties(<target> PROPERTIES LINK_FLAGS -Wl,--stack,10000") didn't work
else()
    # In windows, I think we only get 1 Mb of stack default,
    # so I'm trying to specify 10 Mb:
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /STACK:10000")
endif()

configure_file(C2Config.h.in C2Config.h)

target_include_directories(C2 PUBLIC
                           "${PROJECT_BINARY_DIR}"
                           )

# To have a look at the CMake variables:
get_cmake_property(_variableNames VARIABLES)
list (SORT _variableNames)
foreach (_variableName ${_variableNames})
    message(STATUS "${_variableName}=${${_variableName}}")
endforeach()

