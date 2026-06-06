cmake_minimum_required(VERSION 3.13)
project(libe57-js)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_compile_options(-pthread)

add_subdirectory(libE57Format)

file(GLOB SOURCES src/cpp/*.cpp)

add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(${PROJECT_NAME} PRIVATE E57Format)

set(EM_LINK_OPTIONS
    "--bind"
    "-sMODULARIZE"
    "-sEXPORT_ES6=1"
    "-sEXPORT_NAME=e57"
    "-sENVIRONMENT=node,worker"
    "-sEXPORTED_FUNCTIONS=['_malloc','_free']"
    "-sEXPORTED_RUNTIME_METHODS=['FS','HEAPU8']"
    "-lnodefs.js"
    "-sFORCE_FILESYSTEM=1"
    "-sALLOW_MEMORY_GROWTH"
    "-pthread"
    "-sPTHREAD_POOL_SIZE=8"
)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    list(APPEND EM_LINK_OPTIONS "-sNO_DISABLE_EXCEPTION_CATCHING")
endif()

target_link_options(${PROJECT_NAME} PRIVATE ${EM_LINK_OPTIONS})
