cmake_minimum_required(VERSION 3.24)
project(SO2_Projekt_1)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# raylib pobierany automatycznie przy pierwszej konfiguracji
include(FetchContent)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_GAMES OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
        raylib
        GIT_REPOSITORY https://github.com/raysan5/raylib.git
        GIT_TAG 5.5
        GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(raylib)

find_package(Threads REQUIRED)

add_executable(SO2_Projekt_1 main.cpp
        Wersja1.cpp
        Wersja1.h
        Wersja2.cpp
        Wersja2.h
        Wersja3.cpp
        Wersja3.h
        TableView.cpp
        TableView.h
)

target_link_libraries(SO2_Projekt_1 PRIVATE raylib Threads::Threads)

# MinGW: biblioteki GCC i C++ wbudowane w .exe (pelne -static psuje linkowanie z winpthread)
if (MINGW)
    target_link_options(SO2_Projekt_1 PRIVATE -static-libgcc -static-libstdc++)
endif ()
