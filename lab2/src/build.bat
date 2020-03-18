@echo off

set common_compiler_flags=-nologo

mkdir ..\build
pushd ..\build

cl -nologo -Zi ..\src\win32_main.cpp
cl -nologo -Zi ..\src\win32_child.cpp

popd