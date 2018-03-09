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
