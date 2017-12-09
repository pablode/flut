@echo off
echo ---------------------------------------------------------------------
echo - Visual Studio 2017 Project Generation
echo ---------------------------------------------------------------------
choice /C YN /M "- Generate project files?"
if errorlevel 2 exit
if not exist BUILD mkdir BUILD
cd BUILD
echo - Copying runtime dependencies..
xcopy /S /I /E /Y /Q ..\3rdparty\BUILD\lib bin\Release
cmake .. -G "Visual Studio 15 2017 Win64" -DCMAKE_BUILD_TYPE=Release
cd ..
echo - Execution finished. Open the solution with Visual Studio.
echo - WARNING: Be sure to select "Release" build type!
pause
