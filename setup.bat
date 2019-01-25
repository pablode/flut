@echo off
echo ---------------------------------------------------------------------
echo - Visual Studio Project Generation
echo ---------------------------------------------------------------------
choice /C YN /M "- Build dependencies?"
if errorlevel 2 goto genProject
if errorlevel 1 goto build3rdParty

:build3rdParty
cd 3rdparty
call build.bat
cd ..

:genProject
choice /C YN /M "- Generate project files?"
if errorlevel 2 exit
mkdir BUILD
cd BUILD
echo - Copying runtime dependencies..
xcopy /S /I /E /Y /Q ..\3rdparty\INSTALL\bin bin\Debug
xcopy /S /I /E /Y /Q ..\3rdparty\INSTALL\lib bin\Debug
xcopy /S /I /E /Y /Q ..\3rdparty\INSTALL\bin bin\Release
xcopy /S /I /E /Y /Q ..\3rdparty\INSTALL\lib bin\Release
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_GENERATOR_PLATFORM=x64
cd ..
echo - Execution finished. Open the solution with Visual Studio.
pause
