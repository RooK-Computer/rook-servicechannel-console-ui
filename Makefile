VENV := .venv
VENV_BIN := $(VENV)/bin
CMAKE_LOCAL := $(VENV_BIN)/cmake
CMAKE := $(shell if command -v cmake >/dev/null 2>&1; then command -v cmake; elif [ -x "$(CMAKE_LOCAL)" ]; then printf '%s' "$(CMAKE_LOCAL)"; fi)
BUILD_DIR := build
BINARY := $(BUILD_DIR)/rook-ui
SCREEN ?= welcome
DIST_DIR := dist
VERSION ?= 0.1.0
ARCH ?= $(shell dpkg --print-architecture)
NFPM_GO_PACKAGE := github.com/goreleaser/nfpm/v2/cmd/nfpm@v2.43.1

.PHONY: help tools deps configure build rebuild run preview package package-ui package-integration package-inspect clean distclean

help:
	@echo "Available targets:"
	@echo "  make tools                 Create .venv and install local cmake if needed"
	@echo "  make deps                  Initialize project-local vendored dependencies"
	@echo "  make configure             Configure the CMake build"
	@echo "  make build                 Build the project"
	@echo "  make rebuild               Reconfigure and rebuild"
	@echo "  make run                   Run the app in normal mode"
	@echo "  make preview SCREEN=name   Run preview mode for a specific screen"
	@echo "  make package               Build both Debian packages with nfpm"
	@echo "  make package-ui            Build the rook-console-ui Debian package"
	@echo "  make package-integration   Build the rook-console-integration Debian package"
	@echo "  make package-inspect       Show metadata and contents of built Debian packages"
	@echo "  make clean                 Remove build artifacts"
	@echo "  make distclean             Remove build artifacts and local tool venv"

tools:
	@if command -v cmake >/dev/null 2>&1; then \
		echo "Using system cmake: $$(command -v cmake)"; \
	else \
		echo "Installing local cmake into $(VENV)"; \
		python3 -m venv $(VENV); \
		$(VENV_BIN)/pip install cmake; \
	fi

deps:
	@git submodule update --init --recursive third_party/rmlui

configure:
	@if [ -z "$(CMAKE)" ]; then \
		echo "cmake not found. Run 'make tools' first."; \
		exit 1; \
	fi
	@if [ ! -f third_party/rmlui/CMakeLists.txt ]; then \
		echo "Vendored RmlUi missing. Run 'make deps' first."; \
		exit 1; \
	fi
	@$(CMAKE) -S . -B $(BUILD_DIR)

build: configure
	@$(CMAKE) --build $(BUILD_DIR)

rebuild: clean build

run: build
	@./$(BINARY)

preview: build
	@./$(BINARY) --preview $(SCREEN)

package: package-ui package-integration

package-ui: build
	@mkdir -p $(BUILD_DIR) $(DIST_DIR)
	@sed -e 's|@VERSION@|$(VERSION)|g' -e 's|@ARCH@|$(ARCH)|g' packaging/nfpm/rook-console-ui.yaml.in > $(BUILD_DIR)/rook-console-ui.nfpm.yaml
	@NFPM_CMD="$$(if command -v nfpm >/dev/null 2>&1; then command -v nfpm; else printf '%s' 'go run $(NFPM_GO_PACKAGE)'; fi)"; \
	$$NFPM_CMD package --config $(BUILD_DIR)/rook-console-ui.nfpm.yaml --target $(DIST_DIR)/rook-console-ui_$(VERSION)_$(ARCH).deb

package-integration: build
	@mkdir -p $(BUILD_DIR) $(DIST_DIR)
	@sed -e 's|@VERSION@|$(VERSION)|g' packaging/nfpm/rook-console-integration.yaml.in > $(BUILD_DIR)/rook-console-integration.nfpm.yaml
	@NFPM_CMD="$$(if command -v nfpm >/dev/null 2>&1; then command -v nfpm; else printf '%s' 'go run $(NFPM_GO_PACKAGE)'; fi)"; \
	$$NFPM_CMD package --config $(BUILD_DIR)/rook-console-integration.nfpm.yaml --target $(DIST_DIR)/rook-console-integration_$(VERSION)_all.deb

package-inspect:
	@for package in $(DIST_DIR)/*.deb; do \
		[ -f "$$package" ] || continue; \
		echo "== $$package =="; \
		dpkg-deb -I "$$package"; \
		echo "-- contents --"; \
		dpkg-deb -c "$$package"; \
		echo; \
	done

clean:
	@rm -rf $(BUILD_DIR)

distclean: clean
	@rm -rf $(VENV)
