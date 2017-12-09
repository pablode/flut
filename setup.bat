@echo off
echo ---------------------------------------------------------------------
echo - Windows CMake-Gen Script for Dummies
echo ---------------------------------------------------------------------
if not exist BUILD mkdir BUILD
cd BUILD
echo - Copying runtime dependencies..
xcopy /S /I /E /Y /Q ..\3rdparty\BUILD\lib bin
xcopy /S /I /E /Y /Q ..\3rdparty\BUILD\lib bin\Release

echo - Choose your IDE:
echo (1) Eclipse CDT mit MinGW
echo (2) Visual Studio 2017
echo ----------------------
choice /n /c "12" /M ":"

if errorlevel 1 set CMAKEGEN="Eclipse CDT4 - Unix Makefiles"
if errorlevel 2 (
    set CMAKEGEN="Visual Studio 15 2017 Win64"
    set VSGEN=1
)
cmake .. -G %CMAKEGEN% -DCMAKE_BUILD_TYPE=Release
cd ..
echo - Open the generated project files with your IDE.
if %VSGEN% == 1 (
    echo - NOTE: Be sure to select "Release" build type!
)
pause
