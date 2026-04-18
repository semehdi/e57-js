cmake_minimum_required(VERSION 3.13)
project(libe57-js)
set(CMAKE_CXX_STANDARD 17)
add_subdirectory(libE57Format)
add_executable(${PROJECT_NAME} src/main.cpp src/e57.cpp)
target_link_libraries( ${PROJECT_NAME} PRIVATE E57Format )
target_include_directories( ${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/include )
target_link_options(${PROJECT_NAME} PRIVATE
    "--bind"
    "-sMODULARIZE"
    "-sEXPORT_NAME=e57"
    "-sENVIRONMENT=node"
    "-sNO_DISABLE_EXCEPTION_CATCHING"
    "-sEXPORTED_RUNTIME_METHODS=['FS']"
    "-lnodefs.js"
)
