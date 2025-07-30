cmake_minimum_required(VERSION 3.10)

project(CrowApp)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(Crow CONFIG REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(ZLIB REQUIRED)
find_package(Threads REQUIRED)

add_executable(CrowApp main.cpp)

target_link_libraries(CrowApp 
    Crow::Crow 
    OpenSSL::SSL 
    OpenSSL::Crypto 
    ZLIB::ZLIB 
    Threads::Threads
)
