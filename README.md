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
* local settings persistence and first real RooK-Agent runtime binding for the normal-mode flow

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
make package
```

Useful variants:

```bash
make preview SCREEN=status
./build/rook-ui --preview status --scenario disconnect-dialog
SDL_VIDEODRIVER=dummy ./build/rook-ui --preview password --screenshot /tmp/password.bmp
SDL_VIDEODRIVER=invalid ./build/rook-ui --agent-socket ~/.config/rook-agent/agent.sock
make package-inspect
make rebuild
make clean
```

## Notes

* If no system `cmake` is available, `make tools` creates `.venv/` and installs a local CMake there.
* RmlUi is vendored into the repository as `third_party/rmlui` and initialized with `make deps`.
* SDL2 and FreeType are required system dependencies for the graphical host.
* JSON handling for the agent IPC is vendored project-locally via `third_party/nlohmann/json.hpp`.
* The default runtime path is graphical via SDL2 + RmlUi; the terminal renderer remains only as a diagnostic fallback if the graphical host cannot start.
* On the target path with `SDL_VIDEODRIVER=kmsdrm`, normal mode now tries a hardware-accelerated SDL renderer first and only falls back to software rendering if that renderer cannot be created.
* Resource lookup is resilient to starting the binary from the repository root or from `build/`.
* The public packaged launcher is `/usr/bin/rook-ui`; packaged UI resources live under `/usr/share/rook-console-ui/resources`.
* The packaged launcher also exports `LD_LIBRARY_PATH=/usr/lib/rook-console-ui` so the bundled `librmlui.so` is found at runtime.
* In packaged operation the UI resolves the agent socket from `/etc/default/rook-agent` via `ROOK_AGENT_SOCKET_PATH`; without that file it falls back to the per-user config path for `rook-agent/agent.sock`.
* `make package` builds two Debian packages with `nfpm`: `rook-console-ui` and `rook-console-integration`.
* The integration package installs the RooK system logo directly as `carbon-2021/art/systems/rook.svg` and removes earlier managed theme snippets on upgrade.
* If `nfpm` is not installed locally, the packaging targets fall back to `go run github.com/goreleaser/nfpm/v2/cmd/nfpm@v2.43.1`.

## Contribution workflow

When working on this repository, start here:

1. `docs/rook-ui-konzept.md`
2. `plans/00-plan-index.md`
3. the first plan with status `ready` or `in_progress`
4. `spec/implementation/01-rook-ui-status.md`
