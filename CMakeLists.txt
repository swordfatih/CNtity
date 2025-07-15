cmake_minimum_required(VERSION 3.14)
project(CNtity LANGUAGES CXX)

include(FetchContent)
FetchContent_Declare(
    stduuid
    GIT_REPOSITORY https://github.com/mariusbancila/stduuid.git
    GIT_TAG v1.2.3
)
FetchContent_MakeAvailable(stduuid)

add_library(CNtity INTERFACE)
target_include_directories(CNtity INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(CNtity INTERFACE cxx_std_17)
target_link_libraries(CNtity INTERFACE stduuid)

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
