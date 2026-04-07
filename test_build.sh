#!/bin/bash
cmake -B build -DCMAKE_CXX_FLAGS="$(pkg-config --cflags webkit2gtk-4.1 gtk+-3.0)"
cmake --build build --target Thunderforge_Standalone -j 8
