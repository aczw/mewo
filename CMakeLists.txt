cmake_minimum_required(VERSION 4.0.0)

project(mewo VERSION 0.0.1 LANGUAGES C CXX)

add_executable(mewo src/main.cpp)

target_compile_features(mewo PRIVATE cxx_std_23)
set_target_properties(mewo PROPERTIES CXX_STANDARD_REQUIRED ON CXX_EXTENSIONS OFF)
