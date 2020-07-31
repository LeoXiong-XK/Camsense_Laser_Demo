mkdir build
cd build

mkdir win32_vs2012

cd win32_vs2012
cmake -G "Visual Studio 14 2015" ..\..

cd ..\..

mkdir bin

pause