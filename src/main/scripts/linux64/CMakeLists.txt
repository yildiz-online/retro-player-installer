cmake_minimum_required(VERSION 2.8.7)
project(retro-installer)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DCURL_STATICLIB -static-libstdc++ -static-libgcc -std=c++0x")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DCURL_STATICLIB -static-libstdc++ -static-libgcc -std=c++0x")

set(CMAKE_CXX_STANDARD_LIBRARIES "${CMAKE_CXX_STANDARD_LIBRARIES} -static-libgcc -static-libstdc++")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-Bstatic,--whole-archive -Wl,--no-whole-archive") 

include_directories("${CMAKE_SOURCE_DIR}/libarchive/linux64/include")

file(
        GLOB_RECURSE
        source_files
        ${CMAKE_SOURCE_DIR} ../../c++/*.cpp
        ${CMAKE_SOURCE_DIR} ../../c++/*.c
        ${CMAKE_SOURCE_DIR} ../../c++/*.hpp
        ${CMAKE_SOURCE_DIR} ../../c++/*.h
)

add_executable(Pxl ../../c++/main.cpp)


target_link_libraries(Pxl ${CMAKE_SOURCE_DIR}/libarchive/linux64/lib/libarchive.a)
target_link_libraries(Pxl ${CMAKE_SOURCE_DIR}/zlib/linux64/lib/libz.a)
