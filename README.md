# GeoDecor Studio — v0.2.0 analysis core

Target: Geometry Dash **2.2081** on Android64, using Geode **v5.11.2**.

This version adds the first concrete analysis engine. It imports `.gmd`, `.lvl`, and `.gmd2` through the cross-platform `hjfod.gmd-api` dependency, decodes the level string, parses its start settings and object records, and builds a read-only theme profile.

## What is implemented

- Bounded parser for the decoded Geometry Dash inner level string.
- Start settings: background/ground texture IDs and initial color-channel table (`kS38`).
- Object sampling: object count, bounds, main/secondary channel use, explicit RGB trigger colors, and objects marked as glow-disabled.
- Theme profile: available palette colors, visual-style/atmosphere estimate, and object density.
- Theme mixing for the extracted profile data.
- GMD-API bridge for `.gmd`, `.lvl`, and `.gmd2` imports.
- Portable C++ tests and an Android64 GitHub Actions build.

## Safety design

The parser and analyzer only read source data. They do not edit, delete, reorder, or rewrite any gameplay/layout object. No decoration object is inserted in this milestone. This avoids guessing object IDs before the sprite catalog is verified on Android.

## Put this version into the GitHub repository

1. Download and extract this ZIP on the computer.
2. Open `Calebboss62/GeoDecorMod-BETA-` on GitHub and choose **Add file → Upload files**.
3. Upload the extracted project contents, not just this ZIP. Keep the `.github/workflows/android-smoke-test.yml` path intact.
4. Commit the files to the `main` branch.

## Build

The GitHub workflow runs portable tests first, then builds an Android64 `.geode` package. A successful workflow means the source compiled and the artifact was packaged; it does **not** prove in-game behavior. Download the `GeoDecorStudio-Android64` artifact from the Actions run after a green build.

## Local portable tests

```sh
cmake -S tests -B build-tests -DCMAKE_BUILD_TYPE=Release
cmake --build build-tests --parallel
ctest --test-dir build-tests --output-on-failure
```

The Geode mod build itself requires the Geode SDK/CLI or the supplied GitHub Actions workflow.

## Known limits in v0.2.0

- The `kS38` initial color-channel table is parsed; legacy pre-2.0 color fields and runtime animated color changes are not fully reconstructed yet.
- Glow cannot be reliably inferred from the level string alone. Sprite metadata and the Android resource atlas are needed; the profile explicitly reports this limitation.
- Theme labels are heuristics, not AI image recognition.
- No preview renderer, decoration insertion, template editor, JSON template import/export UI, or undo/redo UI is wired yet. Those come after safe object-ID/resource mapping.
