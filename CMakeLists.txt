cmake_minimum_required(VERSION 3.14)

# ---- project ----
set(CNTITY_VERSION 3.0.0)
project(CNtity VERSION ${CNTITY_VERSION} LANGUAGES CXX)

# ---- library ----
add_library(CNtity INTERFACE)
target_include_directories(CNtity INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(CNtity INTERFACE cxx_std_17)

# ---- examples ----
option(CNTITY_BUILD_EXAMPLES "Build CNtity example executables" OFF)

if(CNTITY_BUILD_EXAMPLES)
    file(GLOB EXAMPLE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/examples/*/main.cpp")

    foreach(EXAMPLE_FILE ${EXAMPLE_FILES})
        get_filename_component(EXAMPLE_NAME ${EXAMPLE_FILE} DIRECTORY)
        get_filename_component(EXAMPLE_NAME ${EXAMPLE_NAME} NAME)

        add_executable(${EXAMPLE_NAME}_example ${EXAMPLE_FILE})
        target_link_libraries(${EXAMPLE_NAME}_example PRIVATE CNtity)
        target_compile_features(${EXAMPLE_NAME}_example PRIVATE cxx_std_17)
    endforeach()
endif()

# ---- tests ----
option(CNTITY_BUILD_TESTS "Build the CNtity test suite" OFF)

if (CNTITY_BUILD_TESTS)
    include(CTest)
    enable_testing()

    include(FetchContent)
    FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v3.8.1
    )
    FetchContent_MakeAvailable(Catch2)

    file(GLOB TEST_FILES "${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cpp")
    
    add_executable(CNtity_tests ${TEST_FILES})
    target_include_directories(CNtity_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/tests/include)
    target_link_libraries(CNtity_tests PRIVATE CNtity Catch2::Catch2WithMain)

    if (MSVC)
        target_compile_options(CNtity_tests PRIVATE /W4 /WX)
    else()
        target_compile_options(CNtity_tests PRIVATE -Wall -Wextra -Wpedantic -Werror)
    endif()

    include(Catch)
    catch_discover_tests(CNtity_tests)
endif()
