cmake_minimum_required (VERSION 2.6)
project (picture_encoder C)
ADD_DEFINITIONS(-D_CRT_SECURE_NO_WARNINGS)
add_library(jpeglib encoder.c encoder.h)
add_executable(encode_app encode_app.c)
target_link_libraries(encode_app jpeglib)

add_executable(unit_test unit_test.c)
target_link_libraries(unit_test jpeglib)
