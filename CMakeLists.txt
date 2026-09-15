cmake_minimum_required(VERSION 3.20)
project(calification_program)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(calification_program src/main.cpp
        src/implementation/CalificationProgram.cpp
        src/headers/CalificationProgram.hpp
        src/implementation/PdfWriter.cpp
        src/headers/PdfWriter.hpp
        src/implementation/Student.cpp
        src/headers/Student.hpp
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
        src/headers/utils.hpp)
