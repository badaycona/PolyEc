@echo off
if not exist work mkdir work
call "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat" >nul
cl /nologo /utf-8 /MT /std:c++20 /EHsc /W4 /Zi /Od /I core core\world.cpp core\actions.cpp core\validation.cpp core\contracts.cpp core\resources.cpp core\transport.cpp core\ledger.cpp core\production.cpp core\life.cpp core\civic.cpp core\persistence.cpp core\view.cpp tests\core_tests.cpp /Fe:work\core_tests.exe /Fo:work\ /Fd:work\tests.pdb
if errorlevel 1 exit /b 1
work\core_tests.exe
if errorlevel 1 exit /b 1
cl /nologo /utf-8 /MT /std:c++20 /EHsc /W4 /Zi /Od /I core tests\action_tests.cpp work\world.obj work\actions.obj work\validation.obj work\contracts.obj work\resources.obj work\transport.obj work\ledger.obj work\production.obj work\life.obj work\civic.obj work\persistence.obj work\view.obj /Fe:work\action_tests.exe /Fo:work\ /Fd:work\tests.pdb
if errorlevel 1 exit /b 1
work\action_tests.exe
if errorlevel 1 exit /b 1
cl /nologo /utf-8 /MT /std:c++20 /EHsc /W4 /Zi /Od /I core tests\journey_tests.cpp work\world.obj work\actions.obj work\validation.obj work\contracts.obj work\resources.obj work\transport.obj work\ledger.obj work\production.obj work\life.obj work\civic.obj work\persistence.obj work\view.obj /Fe:work\journey_tests.exe /Fo:work\ /Fd:work\tests.pdb
if errorlevel 1 exit /b 1
work\journey_tests.exe
if errorlevel 1 exit /b 1
cl /nologo /utf-8 /MT /std:c++20 /EHsc /W4 /O2 /I core core\world.cpp core\actions.cpp core\validation.cpp core\contracts.cpp core\resources.cpp core\transport.cpp core\ledger.cpp core\production.cpp core\life.cpp core\civic.cpp core\persistence.cpp core\view.cpp core\bridge.cpp /Fe:game\polyec_core.exe /Fo:work\ /Fd:work\core.pdb
