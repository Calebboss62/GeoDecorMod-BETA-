# Level analysis data path

1. `gmd::ImportGmdFile` loads `.gmd`, `.lvl`, or `.gmd2` and returns a temporary `GJGameLevel`.
2. `GJGameLevel::m_levelString` is read without changing the level.
3. URL-safe Base64 is decoded, then Geometry Dash's zlib-compressed payload is inflated. Already-decoded strings are accepted as well.
4. `parseInnerLevelString` treats the first semicolon-delimited segment as the special start object (`kA*` / `kS*` fields) and the following segments as object property pairs.
5. `analyzeTheme` derives color-channel use, palette samples, bounds, density, and coarse visual labels.

## Properties used

- Start object `kA6`: background texture ID.
- Start object `kA7`: ground texture ID.
- Start object `kS38`: initial color-channel definitions; each channel entry contains RGB keys 1/2/3 and channel ID key 6.
- Object keys 1/2/3: object ID, X, Y.
- Object keys 19/21/22: legacy/main/secondary color-channel IDs.
- Object keys 7/8/9: explicit RGB values on color triggers.
- Object key 96: Disable Glow flag.

The parser is deliberately read-only and enforces input, segment, and object-count limits. It rejects malformed object records rather than trying to repair them.
