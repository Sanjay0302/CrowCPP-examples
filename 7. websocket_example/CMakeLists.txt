cmake_minimum_required(VERSION 3.16)
project(websocket_example)

set(CMAKE_CXX_STANDARD 17)

# Find Crow
find_package(Crow REQUIRED PATHS ../libs/crow/lib/cmake/Crow)

# Create executable
add_executable(websocket_server server.cpp)

# Find required libraries
find_package(OpenSSL REQUIRED)
find_package(ZLIB REQUIRED)
find_package(Threads REQUIRED)

# Link libraries
target_link_libraries(websocket_server 
    Crow::Crow 
    OpenSSL::SSL 
    OpenSSL::Crypto 
    ZLIB::ZLIB 
    Threads::Threads
)

# Copy templates directory to build directory
file(COPY ${CMAKE_SOURCE_DIR}/templates DESTINATION ${CMAKE_BINARY_DIR})
