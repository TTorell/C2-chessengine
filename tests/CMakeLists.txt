
# add test executable
add_executable(C2-chessengine_test test.cpp)
target_include_directories(C2-chessengine_test PUBLIC
                           "${PROJECT_SRC_DIR}/src"
                           "${PROJECT_SRC_DIR}/test"
                           )

target_link_libraries(C2-chessengine_test PUBLIC C2)


add_test(NAME C2_test COMMAND C2-chessengine_test WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})