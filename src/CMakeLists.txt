cmake_minimum_required(VERSION 3.5)
project(LibuvCpp)

aux_source_directory(. SRC)

add_library(LibuvCpp ${SRC})
