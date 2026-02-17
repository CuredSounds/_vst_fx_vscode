# Contributing

Thank you for contributing. This file includes guidance for dependency updates and testing.

## Updating JUCE and dependencies

This project includes a local copy of JUCE in `JUCE-master/` and/or `JUCE/` in modules. When updating JUCE:

- Prefer to pin to a tagged JUCE release or a specific commit. If you use a git submodule, update the submodule and commit the new SHA.
- Backup your current branch and ensure CI is green after updates.

Steps to update JUCE (manual copy approach):

1. Save a branch with your current project: `git checkout -b juce-update-backup`
2. Replace `JUCE-master/` with the new JUCE release files (or update submodule):
   - If using submodule: `git submodule update --init --remote --merge` and then commit the new pointer.
   - If replacing folder, copy new files and run `git status` to confirm changes.
3. Re-run CMake configure and build locally:

```sh
cmake -S . -B build
cmake --build build -- -j$(sysctl -n hw.ncpu || echo 2)
```

4. Run unit tests (see `Tests/`):

```sh
cmake -S Tests -B Tests/build
cmake --build Tests/build
cd Tests/build
ctest --output-on-failure
```

5. Manually test plugin loads in target DAWs (macOS/Windows combinations relevant to your targets).

6. If update causes API changes in your code, fix compile errors and run tests again. Commit when green.

## Pinning and compatibility

- Record the JUCE version in `CONTRIBUTING.md` or `CHANGELOG.md` when updating.
- Keep a short note about any module changes and breaking API adjustments.

Parameters

- Parameter IDs are stable and documented in `PARAMS.md`. Do not rename or remove IDs after release — changing them breaks host automation and stored presets.

## Quick notes for maintainers

- Use feature branches for larger changes.
- Keep `CHANGELOG.md` entries minimal and actionable.
- Run CI on PRs before merging.
