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

rem Auto-detect the actual VS install via vswhere and override VSDIR if found.
rem The hardcoded path above matches a typical developer machine, but not
rem GitHub-hosted runners (different edition/path) -- vswhere.exe is present
rem on both, so this keeps local/manual usage unchanged while fixing CI.
rem NOTE: deliberately avoids nested ( ... ) blocks here -- %ProgramFiles(x86)%
rem and paths derived from it contain literal parentheses, which confuse cmd's
rem block parser when expanded inside an if/for block body; goto/call sidesteps
rem that entirely.
set VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe
if not exist "%VSWHERE%" goto :vswhere_done
echo Found vswhere at %VSWHERE%, detecting VS install...
for /f "usebackq tokens=*" %%i in (`"%VSWHERE%" -latest -products * -requires Microsoft.Component.MSBuild -property installationPath`) do call :use_vswhere_path "%%i"
goto :vswhere_done

:use_vswhere_path
echo vswhere reports installation at: %~1
if exist "%~1\Common7\IDE\devenv.com" set VSDIR=%~1\Common7\IDE
goto :eof

:vswhere_done

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
