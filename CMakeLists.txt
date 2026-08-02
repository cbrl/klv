cmake_minimum_required(VERSION 3.14)

include(cmake/prelude.cmake)

project(
    klv
    VERSION 1.0.0
    DESCRIPTION "Header-only C++23 library that decodes and encodes KLV (SMPTE 336) metadata and MISB local sets"
    LANGUAGES NONE
)

include(cmake/project-is-top-level.cmake)
include(cmake/variables.cmake)

# ---- Declare library ----

add_library(klv_klv INTERFACE)
add_library(klv::klv ALIAS klv_klv)

set_property(
    TARGET klv_klv PROPERTY
    EXPORT_NAME klv
)

target_include_directories(
    klv_klv ${warning_guard}
    INTERFACE
    "\$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
)

# std::expected and std::generator
target_compile_features(klv_klv INTERFACE cxx_std_23)

# The nested MISB sets instantiate many templates. With MSVC, a source file that uses ST 0601 needs more object
# file sections than the default limit.
target_compile_options(klv_klv INTERFACE "$<$<CXX_COMPILER_ID:MSVC>:/bigobj>")

# ---- Install rules ----

if(NOT CMAKE_SKIP_INSTALL_RULES)
  include(cmake/install-rules.cmake)
endif()

# ---- Examples ----

if(PROJECT_IS_TOP_LEVEL)
  option(BUILD_EXAMPLES "Build examples tree." "${klv_DEVELOPER_MODE}")
  if(BUILD_EXAMPLES)
    add_subdirectory(example)
  endif()
endif()

# ---- Developer mode ----

if(NOT klv_DEVELOPER_MODE)
  return()
elseif(NOT PROJECT_IS_TOP_LEVEL)
  message(
      AUTHOR_WARNING
      "Developer mode is intended for developers of klv"
  )
endif()

include(cmake/dev-mode.cmake)
