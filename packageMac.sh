#!/bin/bash
# Expects Raylib to be installed using homebrew

mkdir -p build Sandbox
rm -rf build/* Sandbox/* Sandbox_MacOS.zip

cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
cp -r assets Sandbox/
cp build/sandbox Sandbox/

# Copy Mac libraries
cp /opt/homebrew/opt/raylib/lib/libraylib.550.dylib Sandbox/

tar -czf Sandbox_MacOS.zip Sandbox/*
rm -rf Sandbox/