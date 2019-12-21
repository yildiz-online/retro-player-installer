cmake_minimum_required(VERSION 2.8.7)
project(retro-installer)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DCURL_STATICLIB -static-libstdc++ -static-libgcc -std=c++0x")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DCURL_STATICLIB -static-libstdc++ -static-libgcc -std=c++0x")

set(CMAKE_CXX_STANDARD_LIBRARIES "${CMAKE_CXX_STANDARD_LIBRARIES} -static-libgcc -static-libstdc++")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-Bstatic,--whole-archive -lwinpthread -Wl,--no-whole-archive") 

include_directories("${CMAKE_SOURCE_DIR}/libarchive/win64/include")

file(
        GLOB_RECURSE
        source_files
        ${CMAKE_SOURCE_DIR} ../../c++/*.cpp
        ${CMAKE_SOURCE_DIR} ../../c++/*.c
        ${CMAKE_SOURCE_DIR} ../../c++/*.hpp
        ${CMAKE_SOURCE_DIR} ../../c++/*.h
)

add_executable(Play50hz-player ../../c++/main.cpp app.rc)


target_link_libraries(Play50hz-player ${CMAKE_SOURCE_DIR}/libarchive/win64/lib/libarchive_static.a)
target_link_libraries(Play50hz-player ${CMAKE_SOURCE_DIR}/zlib/win64/lib/libzlibstatic.a)
target_link_libraries(Play50hz-player wsock32)
target_link_libraries(Play50hz-player ws2_32)
target_link_libraries(Play50hz-player pthread)

