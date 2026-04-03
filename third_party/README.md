# Third-party dependencies

## RmlUi

RmlUi is embedded in this repository as the Git submodule `third_party/rmlui`.

Pinned upstream source:

* Upstream: `https://github.com/mikke89/RmlUi.git`
* Pinned release: `6.2`
* Pinned commit: `2230d1a6e8e0848ed87a5761e2a5160b2a175ba4`

Why it is vendored here:

* the target environment cannot rely on an `apt`-installable RmlUi development package
* the UI graphics work in later plans needs a project-local, version-pinned source of truth

Build integration:

* top-level `CMakeLists.txt` consumes `third_party/rmlui` via `add_subdirectory(...)`
* `make deps` initializes the submodule for fresh checkouts

License and updates:

* the upstream license files live inside `third_party/rmlui`
* update the dependency by moving the submodule to a reviewed upstream release commit and adjusting this file accordingly
