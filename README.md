# RooK Console UI

This repository contains the **local console UI** for the RooK service channel project.

It is the user-facing application that runs on the target device, guides the user through the setup flow, and shows the active service PIN once a support session is available.

## Current status

The project is under active development.

Already in place:

* local UI concept and implementation plans
* C++ / CMake project bootstrap
* graphical SDL2/RmlUi host for preview and normal mode
* screen flow foundation with navigation, focus handling, and shared screen models
* product-oriented welcome, setup, waiting, error, and status screens as real modules under `screens/`
* preview mode and normal mode using the same screen registry and screen implementations

The next implementation steps are tracked in:

* `plans/00-plan-index.md`
* `spec/implementation/01-rook-ui-status.md`

## Repository structure

* `docs/` - local UI concept
* `plans/` - sequenced implementation plans for this repository
* `app/` - application shell, navigation, command line, and runtime glue
* `render/` - rendering and preview output infrastructure
* `screens/` - screen models and screen-specific logic
* `components/` - reusable interaction and UI component models
* `theme/` - theme tokens and typography
* `resources/` - text, layouts, styles, fonts, and other UI assets
* `spec/` - upstream specification submodule

## Build and run

The repository ships with a `Makefile` that wraps the current CMake-based workflow.

Run `make deps` once after a fresh checkout (or clone the repository recursively) so the vendored RmlUi submodule is present locally.

### Common commands

```bash
make deps
make tools
make configure
make build
make run
make preview SCREEN=welcome
```

Useful variants:

```bash
make preview SCREEN=status
make rebuild
make clean
```

## Notes

* If no system `cmake` is available, `make tools` creates `.venv/` and installs a local CMake there.
* RmlUi is vendored into the repository as `third_party/rmlui` and initialized with `make deps`.
* SDL2 and FreeType are required system dependencies for the graphical host.
* The default runtime path is graphical via SDL2 + RmlUi; the terminal renderer remains only as a diagnostic fallback if the graphical host cannot start.
* Resource lookup is resilient to starting the binary from the repository root or from `build/`.

## Contribution workflow

When working on this repository, start here:

1. `docs/rook-ui-konzept.md`
2. `plans/00-plan-index.md`
3. the first plan with status `ready` or `in_progress`
4. `spec/implementation/01-rook-ui-status.md`
