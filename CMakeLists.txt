cmake_minimum_required(VERSION 3.23)
project(luaproc VERSION 1.0.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Release)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/cmake/dist)

set(LUAPROC_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/app.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/lua.cpp
)

link_directories(
    ${PROJECT_SOURCE_DIR}/external/lua/lib
    ${PROJECT_SOURCE_DIR}/external/raylib/lib
)

add_executable(${CMAKE_PROJECT_NAME} ${LUAPROC_FILES})
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE stdc++exp raylib winmm lua54)
target_include_directories(${CMAKE_PROJECT_NAME}
    PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/external/lua/include
    ${PROJECT_SOURCE_DIR}/external/raylib/include
    ${PROJECT_SOURCE_DIR}/external/sol/include
)