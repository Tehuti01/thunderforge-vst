# ⚡️ LH Thunderforge

**LH Thunderforge** is an ultra high-end guitar amp simulator and neural modeler built with JUCE and NAM. It features a premium "AeroTone" machined hardware aesthetic and a real-time glass visualizer.

## 🚀 One-Command Install (macOS/Linux)

To install LH Thunderforge and its dependencies, run this in your terminal:

```bash
curl -fsSL https://raw.githubusercontent.com/username/rusty-ro-vst-3/main/scripts/install.sh | bash
```

## 🛠 Features
- **Neural Amp Modeling (NAM)**: Integrated high-fidelity amp profiles.
- **AeroTone UI**: Machined metal aesthetic with real-time spectrum analysis.
- **Full Signal Chain**: Noise Gate, Tube Screamer, Compressor, EQ, Cabinet Sim, Delay, Reverb, and Chorus.

## 📦 Developer Setup

### Prerequisites
- **macOS**: Xcode Command Line Tools, CMake.
- **Linux**: `build-essential`, `libasound2-dev`, `libjack-jackd2-dev`, `libx11-dev`, `libxcomposite-dev`, `libxcursor-dev`, `libxinerama-dev`, `libxrandr-dev`, `libmesa-dev`, `libfreetype6-dev`, `libcurl4-openssl-dev`.

### Build Locally
```bash
cmake -B build
cmake --build build -j 8
```

## 🔄 Updates
To update to the latest version:
```bash
./scripts/update.sh
```

## ⚖️ License
Proprietary / LH Thunderforge Protocol.
