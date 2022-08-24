cmake_minimum_required(VERSION 3.20)

project(pioasm CXX C)

add_executable(${PROJECT_NAME} src/main.cpp src/pio_assembler.cpp src/pio_disassembler.cpp src/parser.cpp src/lexer.cpp src/c_sdk_output.cpp src/c_output.cpp)
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/inc)