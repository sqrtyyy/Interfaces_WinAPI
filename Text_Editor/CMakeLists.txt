cmake_minimum_required(VERSION 3.17)
project(TextReader C)

set(CMAKE_C_STANDARD 99)

add_executable(TextReader WIN32 main.c Model.h Model.c Viewer.h Viewer.c Menu.h WindowData.h)

project(LoadFileLibrary C)
add_library(LoadFileLibrary SHARED LoadFile.h LoadFile.c)
add_compile_definitions(LoadFileLibrary BUILD_DLL)
