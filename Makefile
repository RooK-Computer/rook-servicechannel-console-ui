VENV := .venv
VENV_BIN := $(VENV)/bin
CMAKE_LOCAL := $(VENV_BIN)/cmake
CMAKE := $(shell if command -v cmake >/dev/null 2>&1; then command -v cmake; elif [ -x "$(CMAKE_LOCAL)" ]; then printf '%s' "$(CMAKE_LOCAL)"; fi)
BUILD_DIR := build
BINARY := $(BUILD_DIR)/rook-ui
SCREEN ?= welcome

.PHONY: help tools deps configure build rebuild run preview clean distclean

help:
	@echo "Available targets:"
	@echo "  make tools                 Create .venv and install local cmake if needed"
	@echo "  make deps                  Initialize project-local vendored dependencies"
	@echo "  make configure             Configure the CMake build"
	@echo "  make build                 Build the project"
	@echo "  make rebuild               Reconfigure and rebuild"
	@echo "  make run                   Run the app in normal mode"
	@echo "  make preview SCREEN=name   Run preview mode for a specific screen"
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

clean:
	@rm -rf $(BUILD_DIR)

distclean: clean
	@rm -rf $(VENV)
