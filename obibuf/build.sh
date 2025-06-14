#!/bin/bash
# OBI Buffer Protocol - Build Script

set -e

echo "🔧 Building OBI Buffer Protocol..."

# Create build directory
mkdir -p build
cd build

# Configure with CMake
echo "📋 Configuring..."
cmake ..

# Build
echo "🔨 Compiling..."
make -j$(nproc)

# Run tests
echo "🧪 Running tests..."
make test

echo "✅ Build completed successfully!"
echo "📍 Built files in: $(pwd)"
echo "🎯 CLI: ./obibuf_cli"
