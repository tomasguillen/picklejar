cmake_minimum_required(VERSION 3.18)
project(PickleJarTests VERSION 0.1.0 LANGUAGES CXX)

#START CREATE CMAKE_BUILD_TYPE=Asan Profile
get_property(isMultiConfig GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)

if(isMultiConfig)
    if(NOT "Asan" IN_LIST CMAKE_CONFIGURATION_TYPES)
        list(APPEND CMAKE_CONFIGURATION_TYPES Asan)
    endif()
    if(NOT "Gcov" IN_LIST CMAKE_CONFIGURATION_TYPES)
        list(APPEND CMAKE_CONFIGURATION_TYPES Gcov)
    endif()
else()
    set(allowedBuildTypes Asan Gcov Debug Release RelWithDebInfo MinSizeRel)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "${allowedBuildTypes}")

    if(CMAKE_BUILD_TYPE AND NOT CMAKE_BUILD_TYPE IN_LIST allowedBuildTypes)
        message(FATAL_ERROR "Invalid build type: ${CMAKE_BUILD_TYPE}")
    endif()
endif()

set(CMAKE_C_FLAGS_ASAN
    "${CMAKE_C_FLAGS_DEBUG} -fsanitize=address -fno-omit-frame-pointer" CACHE STRING
    "Flags used by the C compiler for Asan build type or configuration." FORCE)

set(CMAKE_CXX_FLAGS_ASAN
    "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=address -fno-omit-frame-pointer" CACHE STRING
    "Flags used by the C++ compiler for Asan build type or configuration." FORCE)

set(CMAKE_EXE_LINKER_FLAGS_ASAN
    "${CMAKE_SHARED_LINKER_FLAGS_DEBUG} -fsanitize=address" CACHE STRING
    "Linker flags to be used to create executables for Asan build type." FORCE)

set(CMAKE_SHARED_LINKER_FLAGS_ASAN
    "${CMAKE_SHARED_LINKER_FLAGS_DEBUG} -fsanitize=address" CACHE STRING
    "Linker lags to be used to create shared libraries for Asan build type." FORCE)


set(CMAKE_C_FLAGS_GCOV
    "${CMAKE_C_FLAGS_DEBUG} -fprofile-arcs -ftest-coverage" CACHE STRING
    "Flags used by the C compiler for GCOV build type or configuration." FORCE)

set(CMAKE_CXX_FLAGS_GCOV
    "${CMAKE_CXX_FLAGS_DEBUG} -fprofile-arcs -ftest-coverage" CACHE STRING
    "Flags used by the C++ compiler for GCOV build type or configuration." FORCE)
set(CMAKE_EXE_LINKER_FLAGS_ASAN
    "${CMAKE_SHARED_LINKER_FLAGS_DEBUG} -fprofile-arcs -ftest-coverage" CACHE STRING
    "Linker flags to be used to create executables for Asan build type." FORCE)

set(CMAKE_SHARED_LINKER_FLAGS_ASAN
    "${CMAKE_SHARED_LINKER_FLAGS_DEBUG} -fprofile-arcs -ftest-coverage" CACHE STRING
    "Linker lags to be used to create shared libraries for Asan build type." FORCE)

  
add_subdirectory(ut)
add_subdirectory(hexer)
add_subdirectory(picklejar)


list(APPEND EXTRA_WARNING_FLAGS
  # disable gcc warning about copying memory directly onto strings address, you may try to compile without this if you are using trivial types like ints or basic structs it should work without disabling this warning
  -Wno-class-memaccess
  -Wno-unused
  # -fconcepts-ts use this if you need to compile with c++ version less than c++ 20
)

add_executable(PickleJarTests picklejartests.cpp)
target_link_libraries(PickleJarTests PRIVATE PickleJar Hexer ut)
target_compile_features(PickleJarTests PRIVATE cxx_std_20)
target_compile_options(PickleJarTests PUBLIC ${EXTRA_WARNING_FLAGS})


add_executable(PickleJarOptionalExperimentTests optional_return_tests.cpp)
target_link_libraries(PickleJarOptionalExperimentTests PRIVATE PickleJar Hexer ut)
target_compile_features(PickleJarOptionalExperimentTests PRIVATE cxx_std_20)
target_compile_options(PickleJarOptionalExperimentTests PUBLIC ${EXTRA_WARNING_FLAGS})

add_executable(versioning_example_2_with_tests versioning_example_2_with_tests.cpp)
target_compile_features(versioning_example_2_with_tests PRIVATE cxx_std_20)
target_compile_options(versioning_example_2_with_tests PUBLIC ${EXTRA_WARNING_FLAGS})
target_link_libraries(versioning_example_2_with_tests PRIVATE PickleJar Hexer ut)
