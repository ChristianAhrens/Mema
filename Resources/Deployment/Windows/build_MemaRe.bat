@echo off

set VSVER=%1
if "%VSVER%"=="" set VSVER=VS2026

set WORKSPACE=..\..\..
set JUCEDIR=%WORKSPACE%\submodules\JUCE

if "%VSVER%"=="VS2022" (
    set VSDIR=C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE
    set PROJUCERVSDIR=%JUCEDIR%\extras\Projucer\Builds\VisualStudio2022
    set BUILDSLN=%WORKSPACE%\MemaRe\Builds\VisualStudio2022\Mema.sln
) else (
    set VSDIR=C:\Program Files\Microsoft Visual Studio\18\Community\Common7\IDE
    set PROJUCERVSDIR=%JUCEDIR%\extras\Projucer\Builds\VisualStudio2026
    set BUILDSLN=%WORKSPACE%\MemaRe\Builds\VisualStudio2026\Mema.sln
)

echo.
echo Using variables:
echo VSVER         = %VSVER%
echo JUCEDIR       = %JUCEDIR%
echo VSDIR         = %VSDIR%
echo PROJUCERVSDIR = %PROJUCERVSDIR%
echo WORKSPACE     = %WORKSPACE%
echo BUILDSLN      = %BUILDSLN%
echo.

echo Building Projucer binary
"%VSDIR%\devenv.com" %PROJUCERVSDIR%\Projucer.sln /build Release
echo.

echo Exporting Projucer project
"%PROJUCERVSDIR%\x64\Release\App\Projucer.exe" --resave %WORKSPACE%\MemaRe\MemaRe.jucer --fix-missing-dependencies
echo.

echo Build release
"%VSDIR%\devenv.com" %BUILDSLN% /build Release
echo.
