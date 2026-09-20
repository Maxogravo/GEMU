# ======================================================================
#  GEMU Makefile
#
#  Targets:
#    make             Full build : install dependencies + compile
#    make build       Alias for `make`
#    make deps        Install dependencies only (raylib + compiler)
#    make compile     Compile only (for when deps are already installed)
#    make test        Alias for `make compile`
#    make clean       Remove the built executable
#    make help        Show this help
#
#  Auto-detects the OS and, on Linux, the distro's package manager.
#  You can also force a platform: make linux|mac|windows
# ======================================================================

CXX    := g++
SRC    := src/GEMU.cpp

.PHONY: all build deps compile test clean help linux mac windows

# ----------------------------------------------------------------------
#  OS detection
# ----------------------------------------------------------------------
UNAME_S := $(shell uname -s)

ifeq ($(OS),Windows_NT)
	PLATFORM := windows
else ifneq ($(findstring MINGW,$(UNAME_S)),)
	PLATFORM := windows
else ifneq ($(findstring MSYS,$(UNAME_S)),)
	PLATFORM := windows
else ifneq ($(findstring CYGWIN,$(UNAME_S)),)
	PLATFORM := windows
else ifeq ($(UNAME_S),Darwin)
	PLATFORM := mac
else ifeq ($(UNAME_S),Linux)
	PLATFORM := linux
else
	$(error Unsupported platform detected from `uname -s`: $(UNAME_S))
endif

# Linux distro / package manager detection
ifeq ($(PLATFORM),linux)
	PMGR := $(shell command -v apt-get >/dev/null 2>&1 && echo apt || \
		(command -v dnf >/dev/null 2>&1 && echo dnf || \
		(command -v pacman >/dev/null 2>&1 && echo pacman || \
		(command -v zypper >/dev/null 2>&1 && echo zypper || \
		(command -v apk >/dev/null 2>&1 && echo apk || echo unknown)))))
endif

# ----------------------------------------------------------------------
#  Binary name
# ----------------------------------------------------------------------
ifeq ($(PLATFORM),windows)
	BIN := gemu.exe
else
	BIN := gemu
endif

# ----------------------------------------------------------------------
#  Compile command (kept from the original Makefile)
# ----------------------------------------------------------------------
ifeq ($(PLATFORM),linux)
	COMPILE_CMD := $(CXX) $(SRC) -o $(BIN) -lraylib -lGL -lm -lpthread -ldl
else ifeq ($(PLATFORM),mac)
	ifeq ($(shell command -v pkg-config >/dev/null 2>&1 && echo yes),yes)
		COMPILE_CMD := $(CXX) $(SRC) -o $(BIN) $(shell pkg-config --cflags --libs raylib)
	else
		COMPILE_CMD := $(CXX) $(SRC) -o $(BIN) -lraylib -framework IOKit -framework Cocoa -framework OpenGL
	endif
else
	COMPILE_CMD := $(CXX) $(SRC) -o $(BIN) -lraylib -lwinmm -lgdi32
endif

# ----------------------------------------------------------------------
#  Dependency install command (per OS / distro)
# ----------------------------------------------------------------------
ifeq ($(PLATFORM),mac)
	INSTALL_CMD := @if ! command -v brew >/dev/null 2>&1; then /bin/bash -c "$$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"; fi; brew install raylib
else ifeq ($(PLATFORM),windows)
	INSTALL_CMD := @if command -v pacman >/dev/null 2>&1; then pacman -S --needed --noconfirm mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-raylib; else echo "No MSYS2/MinGW pacman found — install raylib for your toolchain (e.g. vcpkg install raylib) then run 'make compile'."; fi
else
	ifeq ($(PMGR),apt)
		# Debian / Ubuntu / Mint
		INSTALL_CMD := @sudo apt-get update && sudo apt-get install -y libraylib-dev g++ pkg-config
	else ifeq ($(PMGR),dnf)
		# Fedora / RHEL
		INSTALL_CMD := @sudo dnf install -y raylib-devel gcc-c++ pkg-config
	else ifeq ($(PMGR),pacman)
		# Arch / Manjaro
		INSTALL_CMD := @sudo pacman -S --needed --noconfirm raylib gcc
	else ifeq ($(PMGR),zypper)
		# openSUSE / SUSE
		INSTALL_CMD := @sudo zypper install -y raylib-devel gcc-c++ pkg-config
	else ifeq ($(PMGR),apk)
		# Alpine
		INSTALL_CMD := @sudo apk add --no-cache raylib-dev g++ pkgconf
	else
		INSTALL_CMD := @echo "Could not detect your Linux package manager — install raylib and a C++ compiler manually, then run 'make compile'."
	endif
endif

# ----------------------------------------------------------------------
#  Targets
# ----------------------------------------------------------------------
help:
	@echo "GEMU Makefile — detected platform: $(PLATFORM)"
	@echo "  make            full build (install deps + compile)"
	@echo "  make build      alias for make"
	@echo "  make deps       install dependencies only"
	@echo "  make compile    compile only (deps already installed)"
	@echo "  make test       alias for make compile"
	@echo "  make clean      remove the built executable"

all: deps compile
build: all
test: compile

deps:
	$(INSTALL_CMD)

compile:
	@echo "Compiling for $(PLATFORM)..."
	$(COMPILE_CMD)
	@echo "Done: ./$(BIN)"

clean:
	rm -rf gemu gemu.exe
	clear

# Force a specific platform (skips auto-detection)
linux:
	make PLATFORM=linux all

mac:
	make PLATFORM=mac all

windows:
	make PLATFORM=windows all