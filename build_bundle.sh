#!/bin/bash

# Script to build the XPC Demo application bundle

set -e  # Exit on error

echo "Building XPC Demo Application Bundle..."
echo

# Build directory
BUILD_DIR="build"
APP_NAME="XPCDemo.app"
APP_BUNDLE="${BUILD_DIR}/${APP_NAME}"
SERVICE_NAME="com.example.DemoService.xpc"

# Clean and create build directory
echo "Creating build directory structure..."
rm -rf "${BUILD_DIR}"
mkdir -p "${BUILD_DIR}"

# Create app bundle structure
mkdir -p "${APP_BUNDLE}/Contents/MacOS"
mkdir -p "${APP_BUNDLE}/Contents/XPCServices/${SERVICE_NAME}/Contents/MacOS"

echo "Compiling service executable..."
clang -Wall -Wextra -g -fblocks -o "${APP_BUNDLE}/Contents/XPCServices/${SERVICE_NAME}/Contents/MacOS/com.example.DemoService" xpc_service.c -framework Foundation

echo "Compiling app executable..."
clang -Wall -Wextra -g -fblocks -o "${APP_BUNDLE}/Contents/MacOS/XPCDemo" xpc_app.c -framework Foundation

echo "Copying Info.plist files..."
cp AppInfo.plist "${APP_BUNDLE}/Contents/Info.plist"
cp ServiceInfo.plist "${APP_BUNDLE}/Contents/XPCServices/${SERVICE_NAME}/Contents/Info.plist"

echo
echo "Application bundle created successfully at:"
echo "  ${APP_BUNDLE}"
echo
echo "To run the demo:"
echo "  ./build/XPCDemo.app/Contents/MacOS/XPCDemo"
echo
echo "Or simply:"
echo "  make run-bundle"
