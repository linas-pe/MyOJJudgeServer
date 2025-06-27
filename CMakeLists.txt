cmake_minimum_required(VERSION 3.18)
project(MyOJJudgeServer VERSION 0.0.1)

INCLUDE (cmake/pen.cmake)

pen_package_check("pen_utils")
pen_package_check("pen_socket")
pen_package_check("pen_http")
pen_package_check("pen_json")

add_subdirectory(source)

configure_file (
  "${PROJECT_SOURCE_DIR}/pen_config.h.in"
  "${PROJECT_SOURCE_DIR}/include/pen_config.h"
)
