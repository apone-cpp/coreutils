cmake_minimum_required(VERSION 3.5)
project(apone.coreutils)

set(SOURCE_FILES src/coreutils/log.cpp)

add_subdirectory(../fmt fmt)

add_library(coreutils ${SOURCE_FILES})
target_link_libraries(coreutils PUBLIC fmt)
target_include_directories(coreutils INTERFACE src)
set_property(TARGET coreutils PROPERTY POSITION_INDEPENDENT_CODE ON)
set_property(TARGET coreutils PROPERTY CXX_STANDARD 14)
set_property(TARGET coreutils PROPERTY CXX_STANDARD_REQUIRED ON)
if(EXISTS "${CMAKE_SOURCE_DIR}/../catch2")
    add_subdirectory(../catch2 catch2)
    add_executable(coreutils.test src/testmain.cpp src/coreutils/path.test.cpp src/coreutils/split.test.cpp
        src/coreutils/file.test.cpp)
    target_link_libraries(coreutils.test PUBLIC coreutils catch2)
endif()
