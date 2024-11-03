cmake --preset win64-default || exit /b 10
cmake --build --preset win64-default || exit /b 11

cd build\bin\Release
functests.exe || exit /b 10
cd ..\..\..