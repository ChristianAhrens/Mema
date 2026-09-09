@echo off

set INNOSETUPDIR=C:\Program Files (x86)\Inno Setup 6
set WORKSPACE=..\..\..

set VSVER=%1
if "%VSVER%"=="" set VSVER=VS2026

if "%VSVER%"=="VS2022" (
    set ISSFILE=create_MemaRe-Installer_VS2022.iss
) else (
    set ISSFILE=create_MemaRe-installer.iss
)

echo.
echo Using variables:
echo INNOSETUPDIR = %INNOSETUPDIR%
echo WORKSPACE    = %WORKSPACE%
echo VSVER        = %VSVER%
echo ISSFILE      = %ISSFILE%
echo.

echo Build project
CALL build_MemaRe.bat %VSVER%

echo Build installer
"%INNOSETUPDIR%\ISCC.exe" %ISSFILE%
echo.

echo Move setup executable to workspace root
move Output\MemaReSetup*.exe %WORKSPACE%
