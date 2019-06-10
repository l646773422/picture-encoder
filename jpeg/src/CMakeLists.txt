cmake_minimum_required (VERSION 2.6)
project (picture_encoder C)
ADD_DEFINITIONS(-D_CRT_SECURE_NO_WARNINGS)
add_library(commonlib common.c common.h)
add_library(encodelib encoder.c encoder.h)
add_library(decodelib decoder.c decoder.h)

add_executable(encode_app encode_app.c common.h encoder.h)
target_link_libraries(encode_app commonlib encodelib)

add_executable(decode_app  decode_app.c common.h decoder.h)
target_link_libraries(decode_app commonlib decodelib)

add_executable(unit_test unit_test.c common.h)
target_link_libraries(unit_test commonlib encodelib decodelib)
