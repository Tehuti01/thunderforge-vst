#!/bin/bash

# ⚡️ AeroTone Thunderforge 300x Elite - Elite Installer
# (C) 2026 Lukas Hansen Audio Protocol
set -e

REPO_URL="https://github.com/Tehuti01/thunderforge-vst.git"
TARGET_DIR="$HOME/Downloads/thunderforge-vst"

echo "🎸 Starting AeroTone Thunderforge 300x Elite Deployment..."

# 1. Clone/Navigate to Downloads
if [ ! -d "$TARGET_DIR" ]; then
    echo "🛸 Cloning project to $TARGET_DIR..."
    git clone "$REPO_URL" "$TARGET_DIR"
else
    echo "🔄 Updating existing project in $TARGET_DIR..."
    cd "$TARGET_DIR" && git pull
fi

cd "$TARGET_DIR"

# 2. Sync Submodules (Heavy Module Integration)
echo "📦 Syncing JUCE and NAM Core..."
git submodule update --init --recursive

# 3. Build Elite Edition
echo "🏗 Building Thunderforge 300x Elite (MacOS)..."
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --target Thunderforge_Standalone -j$(sysctl -n hw.ncpu)
cmake --build . --target Thunderforge_VST3 -j$(sysctl -n hw.ncpu)

# 4. Install VST3
INSTALL_DIR="$HOME/Library/Audio/Plug-Ins/VST3"
mkdir -p "$INSTALL_DIR"
echo "📦 Installing VST3 to $INSTALL_DIR..."
cp -R "Thunderforge_artefacts/VST3/LH Thunderforge.vst3" "$INSTALL_DIR/"

echo "✅ Deployment Complete!"

# 5. Immediate Startup
echo "🚀 Launching AeroTone Thunderforge 300x Elite..."
open "Thunderforge_artefacts/Standalone/LH Thunderforge.app"
