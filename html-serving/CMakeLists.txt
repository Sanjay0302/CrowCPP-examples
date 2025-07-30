cmake_minimum_required(VERSION 3.10)

project(HTMLServer)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add include directories for libs
include_directories(../libs/crow/include)
include_directories(../libs/asio/asio/include)

# Find required packages
find_package(OpenSSL REQUIRED)
find_package(ZLIB REQUIRED)
find_package(Threads REQUIRED)

# Set Crow configuration
add_definitions(-DCROW_ENABLE_SSL)
add_definitions(-DCROW_ENABLE_COMPRESSION)

# Create executable
add_executable(HTMLServer main.cpp)

# Link libraries
target_link_libraries(HTMLServer 
    OpenSSL::SSL 
    OpenSSL::Crypto 
    ZLIB::ZLIB 
    Threads::Threads
)

# Copy SSL certificates to build directory
file(COPY ssl_certs/ DESTINATION ${CMAKE_BINARY_DIR}/)

# Copy public directory to build directory
file(COPY public/ DESTINATION ${CMAKE_BINARY_DIR}/public/)
