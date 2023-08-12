cmake_minimum_required (VERSION 3.20 FATAL_ERROR)

project (asx LANGUAGES CXX)


set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_library(${PROJECT_NAME} STATIC "./source/asx.cpp")
target_include_directories(${PROJECT_NAME} PUBLIC include PRIVATE source)
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)
target_link_libraries(${PROJECT_NAME} PUBLIC jclib)

if(NOT WIN32)
    target_compile_options(${PROJECT_NAME} PUBLIC -stdlib=libc++ -v)
endif()

target_compile_definitions(${PROJECT_NAME} PUBLIC ASX_PROJECT_SOURCE_ROOT="${ASX_PROJECT_SOURCE_ROOT}")

# Add cmake subdirs
ADD_CMAKE_SUBDIRS_HERE()
