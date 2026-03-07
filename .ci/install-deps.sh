#!/usr/bin/env bash

# Install build dependencies for CI
# Usage: .ci/install-deps.sh [sdl2|headless|format|analysis]

set -euo pipefail

source "$(dirname "$0")/common.sh"

MODE="${1:-sdl2}"

# Setup LLVM 20 APT repository (shared by format and analysis modes)
setup_llvm_repo() {
	LLVM_KEYRING=/usr/share/keyrings/llvm-archive-keyring.gpg
	LLVM_KEY_FP="6084F3CF814B57C1CF12EFD515CF4D18AF4F7421"
	TMPKEY=$(mktemp)
	download_to_file https://apt.llvm.org/llvm-snapshot.gpg.key "$TMPKEY"
	ACTUAL_FP=$(gpg --with-fingerprint --with-colons "$TMPKEY" 2>/dev/null | grep fpr | head -1 | cut -d: -f10)
	if [ "$ACTUAL_FP" != "$LLVM_KEY_FP" ]; then
		print_error "LLVM key fingerprint mismatch!"
		print_error "Expected: $LLVM_KEY_FP"
		print_error "Got: $ACTUAL_FP"
		rm -f "$TMPKEY"
		exit 1
	fi
	sudo gpg --dearmor -o "$LLVM_KEYRING" <"$TMPKEY"
	rm -f "$TMPKEY"

	CODENAME=$(lsb_release -cs)
	echo "deb [signed-by=${LLVM_KEYRING}] https://apt.llvm.org/${CODENAME}/ llvm-toolchain-${CODENAME}-20 main" | sudo tee /etc/apt/sources.list.d/llvm-20.list
	sudo apt-get update -q=2
}

case "$MODE" in
sdl2)
	if [ "$OS_TYPE" = "Linux" ]; then
		sudo apt-get update -q=2
		sudo apt-get install -y -q=2 --no-install-recommends libsdl2-dev python3
	else
		brew install sdl2 python3
	fi
	;;
headless)
	if [ "$OS_TYPE" = "Linux" ]; then
		sudo apt-get update -q=2
		sudo apt-get install -y -q=2 --no-install-recommends python3
	else
		brew install python3
	fi
	;;
format)
	if [ "$OS_TYPE" != "Linux" ]; then
		print_error "Formatting tools only supported on Linux"
		exit 1
	fi
	sudo apt-get update -q=2
	sudo apt-get install -y -q=2 --no-install-recommends shfmt python3-pip gnupg ca-certificates lsb-release
	setup_llvm_repo
	sudo apt-get install -y -q=2 --no-install-recommends clang-format-20

	# Install Python formatter (version-pinned for reproducibility)
	pip3 install --break-system-packages --only-binary=:all: black==25.1.0
	;;
analysis)
	if [ "$OS_TYPE" != "Linux" ]; then
		print_error "Static analysis only supported on Linux"
		exit 1
	fi
	sudo apt-get update -q=2
	sudo apt-get install -y -q=2 --no-install-recommends gnupg ca-certificates lsb-release libsdl2-dev
	setup_llvm_repo
	sudo apt-get install -y -q=2 --no-install-recommends clang-20 clang-tools-20
	;;
*)
	print_error "Unknown mode: $MODE"
	echo "Usage: $0 [sdl2|headless|format|analysis]"
	exit 1
	;;
esac

print_success "Dependencies installed for mode: $MODE"
