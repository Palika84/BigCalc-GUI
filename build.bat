@echo off
setlocal
set "MINGW=C:\msys64\mingw64\bin"
if not exist "%MINGW%\g++.exe" (
  echo Hiba: nem talalhato a g++: %MINGW%\g++.exe
  exit /b 1
)
set "PATH=%MINGW%;%PATH%"

g++ -std=c++17 -O2 -municode -mwindows ^
  -static -static-libgcc -static-libstdc++ ^
  -I. ^
  main.cpp ^
  crypto/BitcoinCrypto.cpp ^
  crypto/SecpAffine.cpp ^
  -o BigCalc.exe ^
  -lcomctl32 -luser32 -lgdi32 -lkernel32

if errorlevel 1 (
  echo Forditas sikertelen.
  exit /b 1
)

echo OK: BigCalc.exe kesz.
endlocal
