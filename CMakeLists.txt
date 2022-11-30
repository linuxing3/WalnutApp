cmake_minimum_required(VERSION 3.10)

# 1. ezvcpkg
# Include ezvcpkg.cmake so we can use ezvcpkg function.
include(${CMAKE_SOURCE_DIR}/cmake/ezvcpkg.cmake)

# 2. xrepo
# Include xrepo.cmake so we can use xrepo_package function.
# include(${CMAKE_BINARY_DIR}/xrepo.cmake)
# xrepo_package("zlib")
# xrepo_target_packages(winapt zlib)

ezvcpkg_fetch(PACKAGES imgui UPDATE_TOOLCHAIN)

project(samplelib)

include_directories(src)
add_library(samplelib src/pch.cpp)

find_package(imgui CONFIG REQUIRED)
target_link_libraries(samplelib PRIVATE imgui::imgui)
