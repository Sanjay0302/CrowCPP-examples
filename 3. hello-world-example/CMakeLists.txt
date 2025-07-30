cmake_minimum_required(VERSION 3.10)

project(HelloWorldExample)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Aggressive compiler optimizations
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG -march=native -mtune=native -flto")
set(CMAKE_BUILD_TYPE Release)

# Additional performance flags
add_compile_options(
    -ffast-math
    -funroll-loops
    -finline-functions
    -fomit-frame-pointer
)

# Add include directories for libs
include_directories(../libs/crow/include)
include_directories(../libs/asio/asio/include)

# Find required packages
find_package(Threads REQUIRED)

# Create executable
add_executable(HelloWorldExample main.cpp)

# Link libraries
target_link_libraries(HelloWorldExample 
    Threads::Threads
)
