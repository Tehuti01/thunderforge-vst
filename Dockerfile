# LH Thunderforge Linux Build Environment
FROM ubuntu:22.04

# 1. Install Dependencies
ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    pkg-config \
    libasound2-dev \
    libjack-jackd2-dev \
    libx11-dev \
    libxcomposite-dev \
    libxcursor-dev \
    libxinerama-dev \
    libxrandr-dev \
    libmesa-dev \
    libgl-dev \
    libglu1-mesa-dev \
    libfreetype6-dev \
    libcurl4-openssl-dev \
    && rm -rf /var/lib/apt/lists/*

# 2. Set Workdir
WORKDIR /thunderforge

# 3. Instruction
LABEL description="Build environment for LH Thunderforge VST3"
CMD ["/bin/bash", "scripts/update.sh"]
