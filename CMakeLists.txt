cmake_minimum_required(VERSION 3.18)
project(PickleJar VERSION 0.1.0 LANGUAGES CXX)


option(ENABLE_THIRDPARTY_OPTIONAL "whether or not to enable type_safe library that allows for no move for read_vector functions" ON)


add_library(PickleJar INTERFACE)
target_sources(PickleJar INTERFACE include/picklejar.hpp)
target_include_directories(PickleJar INTERFACE include)

if(${ENABLE_THIRDPARTY_OPTIONAL})
  add_subdirectory(thirdparty/type_safe)
  target_link_libraries(PickleJar INTERFACE type_safe)
else()
  target_compile_definitions(PickleJar INTERFACE DISABLE_TYPESAFE_OPTIONAL=1)
endif()


