cmake_minimum_required(VERSION 3.16)
project(user_authentication)

set(CMAKE_CXX_STANDARD 17)

# Find required packages
find_package(OpenSSL REQUIRED)
find_package(Threads REQUIRED)

# Add include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
    ../libs/crow/include
)

# Add executable
add_executable(auth_server
    src/main.cpp
    src/auth_manager.cpp
)

# Link libraries
target_link_libraries(auth_server 
    OpenSSL::SSL 
    OpenSSL::Crypto
    Threads::Threads
)

# Copy HTML resources to build directory
file(COPY 
    ${CMAKE_CURRENT_SOURCE_DIR}/resources/login.html
    ${CMAKE_CURRENT_SOURCE_DIR}/resources/register.html
    ${CMAKE_CURRENT_SOURCE_DIR}/resources/dashboard.html
    DESTINATION ${CMAKE_CURRENT_BINARY_DIR}
)
