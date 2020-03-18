@echo off

set common_compiler_flags=-nologo -Zi

mkdir ..\build
pushd ..\build

cl %common_compiler_flags% -Zi ..\src\win32_main.cpp
cl %common_compiler_flags% -Zi ..\src\win32_child.cpp

popd