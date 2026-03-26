#!/bin/bash

# LH Thunderforge - Update Script
set -e

echo "🔄 Updating LH Thunderforge..."

# 1. Pull Changes
git pull
git submodule update --init --recursive

# 2. Re-build
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --target Thunderforge_VST3 -j$(sysctl -n hw.ncpu || nproc)

# 3. Re-install
if [[ "$OSTYPE" == "darwin"* ]]; then
    INSTALL_DIR="$HOME/Library/Audio/Plug-Ins/VST3"
else
    INSTALL_DIR="$HOME/.vst3"
fi
echo "📦 Refreshing VST3..."

if [ -d "Thunderforge_artefacts/Release/VST3" ]; then
    cp -R "Thunderforge_artefacts/Release/VST3/LH Thunderforge.vst3" "$INSTALL_DIR/"
elif [ -d "Thunderforge_artefacts/VST3" ]; then
    cp -R "Thunderforge_artefacts/VST3/LH Thunderforge.vst3" "$INSTALL_DIR/"
fi

echo "✅ Update Complete!"
