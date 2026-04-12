cmake_minimum_required(VERSION 3.13)
project(libe57-js)
set(CMAKE_CXX_STANDARD 17)
add_subdirectory(libE57Format)
add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries( ${PROJECT_NAME} PRIVATE E57Format )
target_link_options(${PROJECT_NAME} PRIVATE
    "--bind"
    "-sMODULARIZE"
    "-sEXPORT_NAME=e57"
)
