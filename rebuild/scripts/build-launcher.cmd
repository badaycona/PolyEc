@echo off
if not exist work mkdir work
if not exist outputs\PolyEc mkdir outputs\PolyEc
call "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat" >nul
cl /nologo /utf-8 /MT /std:c++20 /EHsc /W4 /O2 core\launcher_win.cpp /Fe:outputs\PolyEc\PolyEc.exe /Fo:work\ /link /SUBSYSTEM:WINDOWS user32.lib
