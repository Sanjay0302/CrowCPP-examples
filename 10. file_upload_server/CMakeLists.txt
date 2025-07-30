cmake_minimum_required(VERSION 3.15)
project(MyCrowProject)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(Threads REQUIRED)

# Add Crow include directory
include_directories(${CMAKE_SOURCE_DIR}/../libs/crow/include)

# Add ASIO include directory (standalone)
include_directories(${CMAKE_SOURCE_DIR}/../libs/asio/asio/include)

# Define ASIO_STANDALONE to use ASIO without Boost
add_definitions(-DASIO_STANDALONE)

# Optional: Disable ASIO's deprecated features
add_definitions(-DASIO_NO_DEPRECATED)

# Create executable
add_executable(file_upload_project main.cpp)

# Link threads
target_link_libraries(file_upload_project Threads::Threads)

if(UNIX)
    target_link_libraries(file_upload_project pthread)
endif()

