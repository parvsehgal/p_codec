cmake_minimum_required(VERSION 3.10)
project(p_codec)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SOURCES
    main.cpp
    implementations/chroma.cpp
    implementations/dct.cpp
)

# Create executable
add_executable(p_codec ${SOURCES})

# Tell it where headers are
target_include_directories(p_codec PRIVATE headers)
