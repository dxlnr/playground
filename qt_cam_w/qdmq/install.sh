#!/usr/bin/env bash
#
# chmod +x install.sh
# ./install_deps.sh
#
set -e
# LINUX
LINUX_PACKAGES=(clang libzmq3-dev)
# MACOS
MACOS_PACKAGES=(zeromq)
# WINDOWS
WINDOWS_PACKAGES_CHOCO=(zeromq)

OS_TYPE=""

# GET OS
_uname="$(uname | tr '[:upper:]' '[:lower:]')"

if [[ "$_uname" == *"linux"* ]]; then
    OS_TYPE="linux"
elif [[ "$_uname" == *"darwin"* ]]; then
    OS_TYPE="macos"
elif [[ "$_uname" == *"mingw"* ]] || [[ "$_uname" == *"msys"* ]] || [[ "$_uname" == *"cygwin"* ]]; then
    OS_TYPE="windows"
elif grep -qi microsoft /proc/version 2>/dev/null; then
    OS_TYPE="windows-wsl"
else
    echo "Unsupported OS: $_uname"
    exit 1
fi

install_linux() {
    sudo apt-get install -y "${LINUX_PACKAGES[@]}"
}
install_macos() {
    if ! command -v brew &>/dev/null; then
        /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
        echo 'eval "$(/opt/homebrew/bin/brew shellenv)"' >> ~/.bash_profile
        eval "$(/opt/homebrew/bin/brew shellenv)"
    fi
    brew update
    brew install "${MACOS_PACKAGES[@]}"
}
install_windows() {
    if command -v choco.exe &>/dev/null; then
        choco install -y "${WINDOWS_PACKAGES_CHOCO[@]}"
    else
        exit 3
    fi
}

case "$OS_TYPE" in
    linux)
        install_linux
        ;;
    macos)
        install_macos
        ;;
    windows|windows-wsl)
        install_windows
        ;;
    *)
        exit 4
        ;;
esac
