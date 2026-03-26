#!/bin/bash

# LH Thunderforge - One-Command Installer
set -e

REPO_URL="https://github.com/Tehuti01/thunderforge-vst.git"

if [[ "$OSTYPE" == "darwin"* ]]; then
    INSTALL_DIR="$HOME/Library/Audio/Plug-Ins/VST3"
else
    INSTALL_DIR="$HOME/.vst3"
fi

echo "⚡️ LH Thunderforge - Starting Installation..."

# 1. Clone or Update Repo
if [ ! -d "thunderforge-juce" ]; then
    git clone --recursive $REPO_URL thunderforge-juce
    cd thunderforge-juce
else
    cd thunderforge-juce
    git pull
    git submodule update --init --recursive
fi

# 2. Build
echo "🏗 Building Thunderforge..."
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --target Thunderforge_VST3 -j$(sysctl -n hw.ncpu || nproc)

# 3. Install
echo "📦 Installing VST3..."
mkdir -p "$INSTALL_DIR"
cp -R "Thunderforge_artefacts/VST3/LH Thunderforge.vst3" "$INSTALL_DIR/"

echo "✅ Done! LH Thunderforge is now available in your DAW (e.g., FL Studio)."
