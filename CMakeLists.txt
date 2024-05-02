cmake_minimum_required(VERSION 3.20)

set( CMAKE_VERBOSE_MAKEFILE TRUE )

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


cmake_policy(SET CMP0145 NEW)

# set the project name
project(C2-chessengine LANGUAGES CXX)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

include(CTest)

configure_file(C2Config.h.in C2Config.h)

# set output directories for all builds (Debug, Release, etc.)
#foreach( OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES} )
#    string( TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG )
#    set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_SOURCE_DIR}/lib ) # lib.a
#    set( CMAKE_LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_SOURCE_DIR}/lib ) # lib.so
#    set( CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_SOURCE_DIR}/bin ) # binaries
#endforeach( OUTPUTCONFIG CMAKE_CONFIGURATION_TYPES )

set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib ) # lib.a
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib ) # lib.so
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin ) # binaries

include_directories("/home/torsten/eclipse-workspace/C2-chessengine/src")

# specify compiler flags
if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++latest /W4")
    # Default debug flags are OK in windows
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2")
    # Tried /favour:{INTEL64} as well to make it faster on my own system,
    # but the build system didn't seem to understand, and It's hardly
    # applicable to everyone anyway.

else()
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -g3")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -g0")

    if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
        set(CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES /usr/include/c++/13 /usr/include/x86_64-linux-gnu/c++/13 /usr/include/c++/13/backward /usr/lib/llvm-18/lib/clang/18/include /usr/local/include /usr/include/x86_64-linux-gnu /usr/include)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20 -m64 -march=native -mpopcnt -msse2 -DNDEBUG -pedantic -pedantic-errors -Wall -Wextra -Werror -Wcast-qual -Wshadow -Wconversion -Wdouble-promotion -fmessage-length=0 -pthread -stdlib=libstdc++")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lc++ -lc++abi")
    else()
        # for gcc at the moment
        set(CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES /usr/include/c++/13 /usr/include/x86_64-linux-gnu/c++/13 /usr/include/c++/13/backward /usr/lib/gcc/x86_64-linux-gnu/13/include /usr/local/include /usr/include/x86_64-linux-gnu /usr/include)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20 -m64 -march=native -mpopcnt -msse2 -DNDEBUG -pedantic -pedantic-errors -Wall -Wextra -Werror -Wcast-qual -Wshadow -Wconversion -Wdouble-promotion -fmessage-length=0 -Wabi-tag -D_GLIBCXX_USE_CXX11_ABI=0 -pthread")
    endif()
endif()


if(UNIX AND NOT APPLE)
    # for Linux, BSD, Solaris, Minix

    # target_link_libraries(C2-chessengine pthread)
    # is  defined with -pthread compiler-flag instead

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

enable_testing()

add_subdirectory(src)
add_subdirectory(tests)

# To have a look at the CMake variables:
get_cmake_property(_variableNames VARIABLES)
list (SORT _variableNames)
foreach (_variableName ${_variableNames})
    message(STATUS "${_variableName}=${${_variableName}}")
endforeach()
