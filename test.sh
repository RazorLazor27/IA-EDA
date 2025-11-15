+#!/bin/bash

# ============================
#  Build script for C++ + OpenCV
# ============================

SRC_DIR="tests"
BUILD_DIR="build"
OUTPUT="programa"

CXX="g++"
CXXFLAGS="-std=c++17 -Wall"
OPENCV_FLAGS="-I/usr/include/opencv4 \
              -lopencv_core \
              -lopencv_imgproc \
              -lopencv_highgui \
              -lopencv_imgcodecs"

# Build mode: release or debug
if [ "$1" == "debug" ]; then
    echo "🔧 Building in DEBUG mode..."
    CXXFLAGS="$CXXFLAGS -g -O0"
else
    echo "🚀 Building in RELEASE mode..."
    CXXFLAGS="$CXXFLAGS -O2"
fi

# Ensure build directory exists
mkdir -p $BUILD_DIR

echo "📦 Compiling *.cpp in $SRC_DIR ..."
$CXX $CXXFLAGS $SRC_DIR/*.cpp $OPENCV_FLAGS -o $BUILD_DIR/$OUTPUT

if [ $? -eq 0 ]; then
    echo "✅ Build successful!"
    echo "👉 Ejecuta:   ./$BUILD_DIR/$OUTPUT"
else
    echo "❌ Build failed!"
fi
