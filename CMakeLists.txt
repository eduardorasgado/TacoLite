cmake_minimum_required(VERSION 3.10)
project(SQLiteInteraction)

set(CMAKE_CXX_STANDARD 17)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()
#set_directory_properties(PROPERTIES COMPILE_DEFINITIONS_DEBUG "_DEBUG")

add_executable(SQLiteInteraction main.cpp Handle.h SQLite.h)
target_link_libraries(SQLiteInteraction ${CONAN_LIBS})
