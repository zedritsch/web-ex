@echo off

if not exist bin mkdir bin

pushd bin

cl -Fe..\web-ex.exe -I..\lib ..\src\**

popd