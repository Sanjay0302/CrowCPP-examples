cmake_minimum_required(VERSION 3.16)
project(jwt_example)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(OpenSSL REQUIRED)
find_package(Threads REQUIRED)

# Add include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ../libs/crow/include
)

# Add executable
add_executable(jwt_example
    main.cpp
    jwt_auth.cpp
)

# Link libraries
target_link_libraries(jwt_example 
    OpenSSL::SSL 
    OpenSSL::Crypto
    Threads::Threads
)

# Copy HTML resources to build directory
file(COPY 
    ${CMAKE_CURRENT_SOURCE_DIR}/index.html
    ${CMAKE_CURRENT_SOURCE_DIR}/login.html
    DESTINATION ${CMAKE_CURRENT_BINARY_DIR}
)

# Compiler-specific options
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(jwt_example PRIVATE 
        -Wall 
        -Wextra 
        -Wpedantic
        -O2
    )
endif()

# Set output directory
set_target_properties(jwt_example PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)

# Print build information
message(STATUS "JWT Example Configuration:")
message(STATUS "  C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  OpenSSL Version: ${OPENSSL_VERSION}")
message(STATUS "  Include Directories: ${CMAKE_CURRENT_SOURCE_DIR}, ../libs/crow/include")
