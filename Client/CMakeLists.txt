cmake_minimum_required(VERSION 3.31)

# Name of executable
project(Client)

# C
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

#### Debug ####

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(-g -O0 -fno-omit-frame-pointer)
    add_compile_options(
        -fsanitize=address,undefined
        -fsanitize-recover=address
        -fno-omit-frame-pointer
        -g
        -O0
    )
    add_link_options(
        -fsanitize=address,undefined
    )
endif()

#### Dev ####

if (CMAKE_BUILD_TYPE STREQUAL "Dev")
    add_compile_options(-g -O2 -fno-omit-frame-pointer -DNDEBUG)
endif()

#### Release ####

if (CMAKE_BUILD_TYPE STREQUAL "Release")
    add_compile_options(-g -O3 -ffast-math -DNDEBUG)
endif()

#### Dependencies ####

include(FetchContent)
FetchContent_Declare(
    enet
    GIT_REPOSITORY https://github.com/zpl-c/enet.git
    GIT_TAG master
)
FetchContent_MakeAvailable(enet)

add_executable(${CMAKE_PROJECT_NAME}
    src/main.c
)

# ncurses
set(CURSES_NEED_WIDE TRUE) 
set(CURSES_NEED_NCURSES TRUE)
find_package(Curses REQUIRED)
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE ${CURSES_LIBRARIES})
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${CURSES_INCLUDE_DIRS})

# pthread
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE Threads::Threads)

# Enet
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${enet_SOURCE_DIR}/include)
