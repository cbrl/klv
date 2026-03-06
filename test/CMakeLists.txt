cmake_minimum_required(VERSION 3.14)

project(klvTests LANGUAGES CXX)

include(../cmake/project-is-top-level.cmake)
include(../cmake/folders.cmake)

# ---- Dependencies ----

if(PROJECT_IS_TOP_LEVEL)
  find_package(klv REQUIRED)
  enable_testing()
endif()

find_package(Catch2 3 REQUIRED)
include(Catch)

# ---- Tests ----

add_executable(
    klv_test
    source/bytes_test.cpp
    source/codec_test.cpp
    source/misb_test.cpp
    source/packet_test.cpp
    source/set_test.cpp
)
target_link_libraries(
    klv_test PRIVATE
    klv::klv
    Catch2::Catch2WithMain
)
target_compile_features(klv_test PRIVATE cxx_std_23)

catch_discover_tests(klv_test)

# ---- End-of-file commands ----

add_folders(Test)
