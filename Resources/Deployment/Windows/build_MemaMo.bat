@echo off

set VSDIR=C:\Program Files\Microsoft Visual Studio\18\Community\Common7\IDE
set WORKSPACE=..\..\..
set JUCEDIR=%WORKSPACE%\submodules\JUCE
set PROJUCERVSDIR=%JUCEDIR%\extras\Projucer\Builds\VisualStudio2026
echo.

echo Using variables:
echo JUCEDIR = %JUCEDIR%
echo VSDIR = %VSDIR%
echo PROJUCERVSDIR = %PROJUCERVSDIR%
echo WORKSPACE = %WORKSPACE%
echo.

echo Building Projucer binary
"%VSDIR%\devenv.com" %PROJUCERVSDIR%\Projucer.sln /build Release
echo.

echo Exporting Projucer project
"%PROJUCERVSDIR%\x64\Release\App\Projucer.exe" --resave %WORKSPACE%\MemaMo\MemaMo.jucer --fix-missing-dependencies
echo.

echo Build release
"%VSDIR%\devenv.com" %WORKSPACE%\MemaMo\Builds\VisualStudio2026\Mema.sln /build Release
echo.
