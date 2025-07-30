cmake_minimum_required(VERSION 3.10)

project(StrictHTTPSServer)

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
add_executable(StrictHTTPSServer main.cpp)

# Link libraries
target_link_libraries(StrictHTTPSServer 
    OpenSSL::SSL 
    OpenSSL::Crypto 
    ZLIB::ZLIB 
    Threads::Threads
)

# Copy SSL certificates to build directory
file(COPY server.crt server.key DESTINATION ${CMAKE_BINARY_DIR}/)
