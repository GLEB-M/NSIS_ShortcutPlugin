@echo off

REM Specify the path for MinGW compiler

set PATH=C:\mingw-w64\i686-7.2.0-win32-dwarf-rt_v5-rev1\mingw32\bin

g++ Shortcut.cpp -e_DllMain@12 -lkernel32 -luser32 -lOle32 -lUuid -lmingwex -nostdlib -shared -O3 -Wl,--kill-at -o Shortcut.dll

pause
