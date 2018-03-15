# - Basic CMake setup
# Check version meets ou requirements
# Declare project, which will configure compiler for us
cmake_minimum_required(VERSION 3.3)
project(ImageSegmentation)
find_package(Falaise REQUIRED)

# find the headers
include_directories(.)

# Build a dynamic library from our sources
add_library(ImageSegmentation SHARED imagesegmentation_library.h 
			      imagesegmentation_clustering_module.h
			      imagesegmentation_library.cpp
			      imagesegmentation_clustering_module.cpp)
target_link_libraries(ImageSegmentation PUBLIC Falaise::FalaiseModule)

# Build the testing code, tell CTest about it
enable_testing()

# main catch library compile only once
add_library(Catch testing/catch-main.cpp)
target_include_directories(Catch INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/testing)

# add test executable
include_directories(testing)
add_executable(trial testing/trial.cpp)
target_link_libraries(trial Catch ImageSegmentation PUBLIC Falaise::FalaiseModule)

# CTest comand
add_test(NAME trial
  COMMAND trial
)
