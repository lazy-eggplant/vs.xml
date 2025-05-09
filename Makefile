# Default prefix; override with command line, e.g. "make install PREFIX=/usr"
PREFIX ?= /usr/local

# Build directory for Meson
BUILD_DIR ?= build

# Meson command, use muon if possible
MESON ?= $(shell command -v muon >/dev/null 2>&1 && echo muon || echo meson)

BUILDTYPE ?= release

TOOLCHAIN ?= platforms/gcc.ini

.PHONY: all configure build install dist test clean docs

# Run all steps by default.
all: configure build install docs

# Configure step.
configure:
	@echo "Configuring project with $(MESON) (build type: $(BUILDTYPE))..."
	$(MESON) setup $(BUILD_DIR) -Db_lto=true -Db_thinlto_cache=true -Db_lto_mode=thin --reconfigure --prefix=$(PREFIX) --buildtype=$(BUILDTYPE) --native-file $(TOOLCHAIN) 

# Build step depends on configuration.
build: configure
	@echo "Building project with $(MESON)..."
	$(MESON) compile -C $(BUILD_DIR)

# Install step depends on building.
install: build
	@echo "Installing project with $(MESON)..."
	$(MESON) install -C $(BUILD_DIR)

test: build
	$(MESON) test -C $(BUILD_DIR)

# Dist target: Create a distribution tarball.
dist: install
	@echo "Creating tarball of the project via $(MESON)..."
	$(MESON) dist -C $(BUILD_DIR)

docs:
	doxygen ./dist/configs/Doxyfile
	mkdocs build -f ./dist/configs/mkdocs.yml

# Clean up the build directory.
clean:
	@echo "Cleaning up build directory..."
	rm -rf $(BUILD_DIR)