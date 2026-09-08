# BigCalc GUI

A high-precision calculator built for working with secp256k1 keys: a fixed-point decimal/hex big-integer calculator, elliptic-curve point arithmetic, a brute-force `Start + k·G = Target` step counter, and Bitcoin WIF/address derivation — all in one small desktop app. Ships as a native Qt6 GUI on Linux (`main.cpp`) and a native Win32 GUI on Windows (`main_win32.cpp`), sharing the same dependency-free calculator/crypto core.

*[Magyar leírás → README.hu.md](README.hu.md)*

![BigCalc GUI screenshot](docs/screenshot.png)

## Features

- **Fixed-point big-integer calculator** — arbitrary precision decimal and hexadecimal arithmetic (`+ - * /`), configurable decimal places, mixed hex/decimal expressions like `0xFF * 1.5`.
- **Curve mode ("G-mode")** — the same input box also accepts secp256k1 points (`02…`/`03…` compressed, `04…` uncompressed, or `G` for the generator). Supports point `+`, point `-` (`P1 + (-P2)`), scalar multiplication (`k*P`), and scalar division (`P/k = (k⁻¹ mod n)·P`).
- **Range-Count** — `TargetPoint / StartPoint` walks `Start + G, +G, +G, …` until it lands on `Target` (or hits a configurable step limit), reporting the exact step count. A real (if brute-force) discrete-log-by-walk — useful for checking small, known offsets between two points, not for searching an unbounded keyspace.
- **Bitcoin key derivation** — every scalar or curve result is automatically expanded into its compressed pubkey, P2PKH address, and WIF (when a private key is available). SHA-256, RIPEMD-160, and Base58Check are implemented from scratch, no external crypto library.
- **History panel** — every calculation is logged with its expression, result, WIF, pubkey, and address; click to select, double-click or `Ctrl+C` to copy a cell, right-click for a row/cell copy menu.
- **Hungarian / English / German UI** — switch languages live from the dropdown in the top-right corner; the choice is remembered next to the executable.
- **Three visual themes (Linux/Qt6 build)** — switch live from the "Appearance" dropdown next to the language switcher; the choice is remembered next to the executable, in `bigcalc_theme.ini`:
  - **Terminal / Crypto Dark** — phosphor-green monospace terminal look (JetBrains Mono everywhere).
  - **Modern Light Dashboard** — clean white cards, indigo accent (Manrope + IBM Plex Mono).
  - **Dark Fintech / Exchange** — premium dark navy with a gold accent (Space Grotesk + IBM Plex Mono).
- **Minimal dependencies** — the calculator/crypto core has zero external dependencies; the GUI layer uses Qt6 Widgets on Linux or plain Win32 API on Windows.

## Build

### Linux / Ubuntu (Qt6)

Requires the Qt6 Widgets development package:

```bash
sudo apt install qt6-base-dev cmake build-essential
./build.sh
```

produces `build/BigCalc`. `build.sh` just wraps `cmake` + `cmake --build` (see `CMakeLists.txt`).

### Windows (Win32)

Requires [MSYS2](https://www.msys2.org/) with the `mingw-w64-x86_64-gcc` toolchain (`g++`) installed at `C:\msys64\mingw64`.

```bat
build.bat
```

produces `BigCalc.exe`. The build is a single `g++` invocation (see `build.bat`) — no CMake, no project files needed.

### Running the self-tests

Three small console programs exercise the calculator core and the Bitcoin key derivation against known-good vectors (including the well-known `k=1` → `1BgGZ9tcN4rm9KBzDn7KprQz87SZ26SAMH` test vector):

```bat
g++ -std=c++17 -O2 -I. test_calc.cpp -o test_calc.exe
g++ -std=c++17 -O2 -I. test_btc.cpp crypto/BitcoinCrypto.cpp crypto/SecpAffine.cpp -o test_btc.exe
g++ -std=c++17 -O2 -I. test_btc_vectors.cpp crypto/BitcoinCrypto.cpp crypto/SecpAffine.cpp -o test_btc_vectors.exe
```

## Usage

Type an expression into the **Full expression** box, or fill the **Left / Operator / Right** fields, then hit **Calculate**:

| Input example | What it does |
|---|---|
| `0xFF * 1.5` | Fixed-point calculator, `382.500` |
| `5 * G` | Scalar multiplication → the pubkey `5·G` (and its WIF/address, since `5` is a valid private key) |
| `<pubkey> / <start-pubkey>` | Range-Count: how many `+G` steps from Start to reach the target pubkey |
| `<pubkey> / 3` | `(3⁻¹ mod n) · pubkey` |

The **Decimal** and **Hexadecimal** result panels show the raw numeric or point result; the **History** table always tries to also derive the WIF / compressed pubkey / P2PKH address, when the result is a valid scalar or point.

## Project layout

```
main.cpp                 Qt6 Widgets GUI (Linux), window/layout, event handling, theming
main_win32.cpp           Win32 GUI (Windows), window/layout, event loop
Lang.hpp                 HU/EN/DE translation table for every user-facing string
Theme.hpp                visual theme enum + bigcalc_theme.ini load/save (Linux/Qt6 build)
Calculator.hpp           fixed-point decimal/hex big-integer calculator
ModeEngine.hpp           input classification + the "smart" DEC/HEX/G-mode dispatcher
BigInt.hpp               arbitrary-precision signed integer
crypto/
  SecpAffine.*            secp256k1 field/curve arithmetic (affine), Range-Count
  BitcoinCrypto.*         SHA-256, RIPEMD-160, Base58Check, WIF/address derivation
fonts/                   bundled OFL-licensed webfonts for the 3 Qt6 themes (see fonts/*/OFL.txt)
CMakeLists.txt, build.sh Linux (Qt6) build
build.bat                Windows (Win32) build
test_calc.cpp, test_btc.cpp, test_btc_vectors.cpp   self-tests (platform-independent)
```

## Security note

This tool computes real Bitcoin private keys, WIFs, and addresses locally, in memory, and never sends anything over the network. That said: **prefer building from source over running a downloaded binary** if you intend to use it with a private key that guards real funds — that's good practice for any tool that touches key material, not a claim that this one is untrustworthy.

## License

[MIT](LICENSE)
