#!/bin/bash

# HTML Server Build Script
echo "HTML Server with HTTPS - Build Script"
echo "====================================="

# Check if build directory exists
if [ ! -d "build" ]; then
    echo "Creating build directory..."
    mkdir build
fi

# Navigate to build directory
cd build

echo "Configuring project with CMake..."
cmake ..

if [ $? -eq 0 ]; then
    echo "Building project..."
    make -j$(nproc)
    
    if [ $? -eq 0 ]; then
        echo ""
        echo "✅ Build successful!"
        echo ""
        echo "To run the server:"
        echo "  cd build"
        echo "  ./HTMLServer"
        echo ""
        echo "Then visit:"
        echo "  HTTP (redirects to HTTPS): http://localhost:8080"
        echo "  HTTPS (secure): https://localhost:8443"
        echo ""
    else
        echo "❌ Build failed!"
        exit 1
    fi
else
    echo "❌ CMake configuration failed!"
    exit 1
fi
