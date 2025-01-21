#!/usr/bin/env bash
set -e

SUDO=""
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

if [[ ! $(id -u) -eq 0 ]]; then
  if [[ -z $(which sudo) ]]; then
    echo "Please run as root"
    exit 1
  fi
  SUDO="sudo"
fi

function install_ubuntu_common() {
  $SUDO apt-get update
  $SUDO apt-get install -y --no-install-recommends \
    build-essential \
    clang \
    qt6-base-dev \
    qt6-tools-dev \
    mesa-utils \
    libgl1-mesa-dev \ 
    python3-dev \
}

install_ubuntu_common
