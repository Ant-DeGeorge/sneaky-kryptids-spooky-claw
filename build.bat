@echo off
REM Sneaky Kryptid's Spooky Claw - Windows build (GBDK-2020 at C:\gbdk)
setlocal enabledelayedexpansion
REM 128 KB: the game plus the Rejected By The Machines studio card (autobanked).
REM Python is only needed if you change art or music (tools\gen_assets.py / tools\gen_sound.py).
if not defined GBDK_HOME set GBDK_HOME=C:\gbdk
set LCC=%GBDK_HOME%\bin\lcc
if not exist build mkdir build
if not exist build\obj mkdir build\obj
set FLAGS=-Wa-l -Wl-m -Wl-j -Wm-yC -Wm-ynSPOOKYCLAW -Wl-yo8 -Wm-yo8 -autobank -Isrc\studio_card
set OBJS=
for %%f in (main0 loader main sound sound_data assets0 assets1 assets2) do (
  "%LCC%" %FLAGS% -c -o build\obj\%%f.o src\%%f.c
  if errorlevel 1 goto :fail
  set OBJS=!OBJS! build\obj\%%f.o
)
for %%f in (studio_card studio_card_home studio_card_gfx studio_card_scx) do (
  "%LCC%" %FLAGS% -Wf--opt-code-speed -c -o build\obj\%%f.o src\studio_card\%%f.c
  if errorlevel 1 goto :fail
  set OBJS=!OBJS! build\obj\%%f.o
)
"%LCC%" %FLAGS% -Wm-ya1 -Wm-yt0x1B -o build\spooky_claw3.gbc %OBJS%
if errorlevel 1 goto :fail
"%LCC%" %FLAGS% -Wm-yt0x19 -o build\spooky_claw3_nosave.gbc %OBJS%
if errorlevel 1 goto :fail
echo Built build\spooky_claw3.gbc
goto :eof
:fail
echo BUILD FAILED
exit /b 1
