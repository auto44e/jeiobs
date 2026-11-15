#!/usr/bin/env bash
# Build/download the three dependencies Ubuntu 24.04 ships too old for the
# OBS 33.0.1 base, into a private prefix (no sudo, no system changes).
#
#   tools/jeiobs/linux-deps.sh [DEPS_DIR]       default: ../jeiobs-deps
#
# Then configure and build out of tree (see docs/jeiobs/BUILDING.md).
# Versions match what the 33.0.1 base expects: FFmpeg >= 8.0 (libobs
# find_package), MbedTLS 3.x (obs-outputs), and Qt 6.11.1 (obs-deps
# 2026-10-23, the pin in CMakePresets.json). Ubuntu 24.04 provides FFmpeg 6.1,
# MbedTLS 2.28 and Qt 6.4, which fail to configure or compile.
set -euo pipefail

DEPS="${1:-$(cd "$(dirname "$0")/../.." && pwd)/../jeiobs-deps}"
mkdir -p "$DEPS/src"
DEPS="$(cd "$DEPS" && pwd)"
SRC="$DEPS/src"
JOBS="$(nproc)"

FFMPEG_V=8.0.3
FFMPEG_SHA256=6136812ea6d4e68bdba27e33c2a94382711cdf4f8602ffef056ff792bd6f9818
MBEDTLS_V=3.6.7
MBEDTLS_SHA256=a7e8bcbec0e6f761b4af24f25677626b35f762f68eef79c08677a363212d11f6
QT_V=6.11.1

verify() { # file sha256
  echo "$2  $1" | sha256sum -c - >/dev/null || { echo "SHA256 mismatch: $1" >&2; exit 1; }
}

# --- FFmpeg (LGPL build: no --enable-gpl / --enable-nonfree) ---------------
if [ ! -f "$DEPS/ffmpeg-$FFMPEG_V/lib/pkgconfig/libavcodec.pc" ]; then
  cd "$SRC"
  [ -f "ffmpeg-$FFMPEG_V.tar.xz" ] || curl -fsSLO "https://ffmpeg.org/releases/ffmpeg-$FFMPEG_V.tar.xz"
  verify "ffmpeg-$FFMPEG_V.tar.xz" "$FFMPEG_SHA256"
  rm -rf "ffmpeg-$FFMPEG_V" && tar -xf "ffmpeg-$FFMPEG_V.tar.xz"
  cd "ffmpeg-$FFMPEG_V"
  ./configure --prefix="$DEPS/ffmpeg-$FFMPEG_V" \
    --enable-shared --disable-static --enable-pic \
    --disable-programs --disable-doc --disable-debug --disable-x86asm \
    --enable-libsrt --enable-librist --enable-vaapi
  make -j"$JOBS" && make install
fi

# --- MbedTLS ----------------------------------------------------------------
if [ ! -f "$DEPS/mbedtls-$MBEDTLS_V/lib/cmake/MbedTLS/MbedTLSConfig.cmake" ]; then
  cd "$SRC"
  T="mbedtls-$MBEDTLS_V"
  [ -f "$T.tar.bz2" ] || curl -fsSLO "https://github.com/Mbed-TLS/mbedtls/releases/download/$T/$T.tar.bz2"
  verify "$T.tar.bz2" "$MBEDTLS_SHA256"
  rm -rf "$T" && tar -xjf "$T.tar.bz2"
  cmake -S "$T" -B "$T/build" -G Ninja -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX="$DEPS/$T" -DENABLE_TESTING=OFF -DENABLE_PROGRAMS=OFF \
    -DUSE_SHARED_MBEDTLS_LIBRARY=ON -DUSE_STATIC_MBEDTLS_LIBRARY=OFF \
    -DCMAKE_POSITION_INDEPENDENT_CODE=ON
  cmake --build "$T/build" -j"$JOBS" && cmake --install "$T/build"
fi

# --- Qt (official binaries via aqtinstall, in a private venv) ---------------
if [ ! -d "$DEPS/Qt/$QT_V/gcc_64/lib/cmake/Qt6" ]; then
  [ -x "$DEPS/aqt-venv/bin/aqt" ] || { python3 -m venv "$DEPS/aqt-venv" && "$DEPS/aqt-venv/bin/pip" install -q aqtinstall; }
  "$DEPS/aqt-venv/bin/aqt" install-qt linux desktop "$QT_V" linux_gcc_64 -O "$DEPS/Qt" -m qtimageformats
fi

cat <<EOF
Dependencies ready in $DEPS
  FFMPEG=$DEPS/ffmpeg-$FFMPEG_V
  MBEDTLS=$DEPS/mbedtls-$MBEDTLS_V
  QT=$DEPS/Qt/$QT_V/gcc_64
EOF
