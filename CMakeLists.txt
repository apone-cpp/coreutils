cmake_minimum_required(VERSION 3.5)
project(apone.coreutils)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_library(log_dep INTERFACE)
if(TARGET fmt)
  target_compile_definitions(log_dep INTERFACE USE_FMT)
  target_link_libraries(log_dep INTERFACE fmt)
elseif(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../fmt")
  add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../fmt fmt)
  target_compile_definitions(log_dep INTERFACE USE_FMT)
  target_link_libraries(log_dep INTERFACE fmt)
else()
endif()

set(SOURCE_FILES src/coreutils/log.cpp)
add_library(coreutils ${SOURCE_FILES})
target_compile_definitions(coreutils PUBLIC )
target_link_libraries(coreutils PUBLIC log_dep)
target_include_directories(coreutils INTERFACE src)

set_property(TARGET coreutils PROPERTY POSITION_INDEPENDENT_CODE ON)
set_property(TARGET coreutils PROPERTY CXX_STANDARD 14)
set_property(TARGET coreutils PROPERTY CXX_STANDARD_REQUIRED ON)

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../catch2")
    add_subdirectory(../catch2 catch2)
    add_executable(coreutils.test src/testmain.cpp src/coreutils/resources.test.cpp src/coreutils/path.test.cpp src/coreutils/split.test.cpp
        src/coreutils/file.test.cpp)
    target_link_libraries(coreutils.test PUBLIC coreutils catch2)
endif()
