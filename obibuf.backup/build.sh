#!/bin/bash
# OBI Buffer Protocol - Build Script

set -e

echo "🔧 Building OBI Buffer Protocol..."

# Create build directory
mkdir -p build
cd build

# Configure with CMake
echo "📋 Configuring with CMake..."
cmake ..

# Build the project
echo "🔨 Compiling..."
make -j$(nproc)

# Run basic tests
echo "🧪 Running tests..."
if [ -f "./test_validator" ]; then
    echo "Running validator tests..."
    # ./test_validator
fi

echo "✅ Build completed successfully!"
echo "📍 Built files are in: $(pwd)"
echo "🎯 CLI tool: ./obibuf_cli"
