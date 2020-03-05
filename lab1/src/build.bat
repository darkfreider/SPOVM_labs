@echo off

set common_compiler_flags=-nologo -Zi

mkdir ..\build
pushd ..\build

cl %common_compiler_flags% -DGENERATE_PARENT_APP /Foparent.obj /Feparent ..\src\win32_main.cpp
cl %common_compiler_flags% -DGENERATE_CHILD_APP /Fochild.obj /Fechild ..\src\win32_main.cpp

popd