#!/usr/bin/env bash

set -euo pipefail

if ! command -v apt-get >/dev/null 2>&1; then
    echo "This installer supports Ubuntu and Debian." >&2
    exit 1
fi

if (( EUID == 0 )); then
    APT=(apt-get)
elif command -v sudo >/dev/null 2>&1; then
    APT=(sudo apt-get)
else
    echo "Run this script as root or install sudo." >&2
    exit 1
fi

"${APT[@]}" update
"${APT[@]}" install --yes \
    binutils \
    build-essential \
    cmake \
    git \
    libfontconfig1-dev \
    libgl1-mesa-dev \
    libglew-dev \
    libglu1-mesa-dev \
    libgtk-3-dev \
    libsqlite3-dev \
    libx11-dev \
    libxcursor-dev \
    libxi-dev \
    libxinerama-dev \
    libxrandr-dev \
    libxxf86vm-dev \
    perl \
    pkg-config \
    python3
