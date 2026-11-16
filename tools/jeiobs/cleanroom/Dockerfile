# Clean-room check of docs/jeiobs/BUILDING.md on Ubuntu 24.04.
#   docker build -t jeiobs-buildenv:24.04 tools/jeiobs/cleanroom
#   docker run --rm jeiobs-buildenv:24.04 bash /home/builder/docs-check.sh origin/main
# Clones from GitHub inside the container; nothing is mounted or published.
FROM ubuntu:24.04
ENV DEBIAN_FRONTEND=noninteractive
# Upstream .github/scripts/utils.zsh/setup_ubuntu list, minus what BUILDING.md
# says is not needed (dh-cmake, libatk*, libfdk-aac-dev) and minus Ubuntu's Qt
# (the private Qt 6.11.1 is used), plus the tools the build and linux-deps.sh use.
RUN apt-get update && apt-get install -y --no-install-recommends \
  build-essential libglib2.0-dev extra-cmake-modules lsb-release \
  libcurl4-openssl-dev \
  libavcodec-dev libavdevice-dev libavfilter-dev libavformat-dev libavutil-dev \
  libswresample-dev libswscale-dev libjansson-dev libx11-xcb-dev libgles2-mesa-dev \
  libwayland-dev libpipewire-0.3-dev libpulse-dev libx264-dev libmbedtls-dev \
  libgl1-mesa-dev uthash-dev libsimde-dev libluajit-5.1-dev python3-dev \
  libx11-dev libxcb-randr0-dev libxcb-shm0-dev libxcb-xinerama0-dev \
  libxcb-composite0-dev libxinerama-dev libxcb1-dev libxcb-xfixes0-dev \
  swig libcmocka-dev libxss-dev libglvnd-dev libxkbcommon-dev libxcomposite-dev \
  libxdamage-dev libasound2-dev libfontconfig-dev libfreetype6-dev \
  libjack-jackd2-dev libsndio-dev libspeexdsp-dev libudev-dev libv4l-dev \
  libva-dev libvlc-dev libpci-dev libdrm-dev nlohmann-json3-dev \
  libwebsocketpp-dev libasio-dev libqrcodegencpp-dev libffmpeg-nvenc-dev \
  librist-dev libsrt-openssl-dev libvpl-dev libvpl2 \
  libssl-dev cmake ninja-build git ca-certificates curl xz-utils bzip2 nasm pkg-config \
  python3-venv libegl-dev libxcb-cursor0 libxkbcommon-x11-0 xvfb xauth \
  && rm -rf /var/lib/apt/lists/*
RUN useradd -m builder
USER builder
WORKDIR /home/builder
COPY --chown=builder docs-check.sh /home/builder/docs-check.sh
