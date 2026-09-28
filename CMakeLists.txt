cmake_minimum_required(VERSION 3.20)

project(cpp_file_indexer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(file_indexer
    src/main.cpp
    src/FileIndexer.cpp
)

target_include_directories(file_indexer PRIVATE
    ${PROJECT_SOURCE_DIR}/include
)

add_executable(file_indexer_bench
    src/benchmark.cpp
    src/FileIndexer.cpp
)

target_include_directories(file_indexer_bench PRIVATE
    ${PROJECT_SOURCE_DIR}/include
)