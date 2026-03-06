cmake_minimum_required(VERSION 3.14)

project(klvExamples CXX)

include(../cmake/project-is-top-level.cmake)
include(../cmake/folders.cmake)

if(PROJECT_IS_TOP_LEVEL)
  find_package(klv REQUIRED)
endif()

add_custom_target(run-examples)

# add_example(<name> [args...]): build <name>.cpp and add a run_<name> target that runs it with the given arguments in
# this binary directory
function(add_example NAME)
  add_executable("${NAME}" "${NAME}.cpp")
  target_link_libraries("${NAME}" PRIVATE klv::klv)
  target_compile_features("${NAME}" PRIVATE cxx_std_23)
  add_custom_target(
      "run_${NAME}"
      COMMAND "${NAME}" ${ARGN}
      WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
      VERBATIM
  )
  add_dependencies("run_${NAME}" "${NAME}")
  add_dependencies(run-examples "run_${NAME}")
endfunction()

# create_klv_data writes example_output.klv, which the other two examples read
add_example(create_klv_data)
add_example(parse_klv_file example_output.klv)
add_example(print_metadata example_output.klv)
add_dependencies(run_parse_klv_file run_create_klv_data)
add_dependencies(run_print_metadata run_create_klv_data)

add_folders(Example)
