# Bundled fonts

These weights were instanced from the variable-font releases in the
[google/fonts](https://github.com/google/fonts) repository (`ofl/` — SIL Open
Font License 1.1). Each subfolder carries its own `OFL.txt`.

- **JetBrainsMono** — used by the *Terminal / Crypto Dark* theme.
- **Manrope** — UI font for the *Modern Light Dashboard* theme.
- **IBMPlexMono** — monospace result/table font for the *Light Dashboard* and
  *Dark Fintech* themes.
- **SpaceGrotesk** — UI font for the *Dark Fintech / Exchange* theme.

Loaded at startup via `QFontDatabase::addApplicationFont` in `main.cpp`
(Linux/Qt6 build only) — no system font installation required.
