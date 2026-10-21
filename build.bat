@echo off
REM Sneaky Kryptid's Spooky Claw - Windows build (GBDK-2020 at C:\gbdk)
REM Python is only needed if you change art or music (tools\gen_assets.py / tools\gen_sound.py).
if not defined GBDK_HOME set GBDK_HOME=C:\gbdk
if not exist build mkdir build
set SRC=src\main0.c src\loader.c src\main.c src\sound.c src\sound_data.c src\assets0.c src\assets1.c src\assets2.c
set FLAGS=-Wm-yC -Wm-yn"SPOOKYCLAW" -Wm-yo4
"%GBDK_HOME%\bin\lcc" %FLAGS% -Wm-ya1 -Wm-yt0x1B -o build\spooky_claw3.gbc %SRC%
if errorlevel 1 goto :fail
"%GBDK_HOME%\bin\lcc" %FLAGS% -Wm-yt0x19 -o build\spooky_claw3_nosave.gbc %SRC%
if errorlevel 1 goto :fail
echo Built build\spooky_claw3.gbc
goto :eof
:fail
echo BUILD FAILED
exit /b 1
