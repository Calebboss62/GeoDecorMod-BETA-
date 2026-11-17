# Changelog

## v0.2.0
- Added read-only Geometry Dash inner level string parser.
- Added initial theme analysis from color channels, RGB trigger colors, object bounds, and density.
- Added `.gmd` / `.lvl` / `.gmd2` import bridge through `hjfod.gmd-api`.
- Added portable parser/analyzer tests and an Android64 GitHub Actions workflow.
- Explicitly report that glow inference needs sprite metadata and that no original level objects are modified.
