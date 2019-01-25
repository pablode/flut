mkdir temp
cd temp
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_GENERATOR_PLATFORM=x64
cmake --build .
cd ..
rmdir /s /q temp
pause
