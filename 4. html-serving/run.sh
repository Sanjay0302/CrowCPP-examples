#!/bin/bash

# HTML Server Run Script
echo "HTML Server with HTTPS - Run Script"
echo "==================================="

# Check if build directory exists
if [ ! -d "build" ]; then
    echo "❌ Build directory not found. Please run ./build.sh first"
    exit 1
fi

# Check if executable exists
if [ ! -f "build/HTMLServer" ]; then
    echo "❌ HTMLServer executable not found. Please run ./build.sh first"
    exit 1
fi

echo "Starting HTML Server with HTTPS..."
echo ""
echo "Server will start on:"
echo "  HTTP (redirects to HTTPS): http://localhost:8080"
echo "  HTTPS (secure): https://localhost:8443"
echo ""
echo "Press Ctrl+C to stop the server"
echo "================================="
echo ""

# Navigate to build directory and run
cd build
./HTMLServer
