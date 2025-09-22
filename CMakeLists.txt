cmake_minimum_required(VERSION 4.0)
project(lab02_tp)

set(CMAKE_CXX_STANDARD 20)

add_executable(lab02_tp src/main.cpp
        src/implementation/Deduction.cpp
        src/headers/Deduction.hpp
        src/implementation/Criterion.cpp
        src/headers/Criterion.hpp
        src/implementation/Rubric.cpp
        src/headers/Rubric.hpp
        src/implementation/Observation.cpp
        src/headers/Observation.hpp
        src/implementation/Item.cpp
        src/headers/Item.hpp
        src/headers/utils.hpp
        src/implementation/read_functions.cpp
        src/headers/read_functions.hpp)
