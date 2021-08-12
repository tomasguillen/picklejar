cmake_minimum_required(VERSION 3.18)
project(PickleJar VERSION 0.1.0 LANGUAGES CXX)

add_library(PickleJar INTERFACE)
target_sources(PickleJar INTERFACE picklejar.hpp)
target_include_directories(PickleJar INTERFACE .)
#target_compile_features(PickleJar INTERFACE cxx_std_20)
