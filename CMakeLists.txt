cmake_minimum_required(VERSION 3.10)

project(WordleSutom LANGUAGES CXX)

set(DEBUG_MODE false)
set(SOURCES
    src/utils.cpp
    src/word_list.cpp
    src/game.cpp
    src/gameResolver.cpp
)

set(HEADERS
    src/utils.h
    src/word_list.h
    src/game.h
    src/gameResolver.h
)

if(DEBUG_MODE)
    set(CMAKE_BUILD_TYPE Debug)
else(DEBUG_MODE)
    set(CMAKE_BUILD_TYPE Release)
endif(DEBUG_MODE)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

add_executable(WordleSutom
    app.cpp
    ${SOURCES}
    ${HEADERS}
)
