@echo off
rem Initialize VS developer environment and run msbuild
rem -products * also finds Build Tools instances, but those may lack ATL, so require it explicitly
for /f "usebackq tokens=*" %%i in (`"%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" -latest -products * -requires Microsoft.VisualStudio.Component.VC.ATL -property installationPath`) do set VS_PATH=%%i
call "%VS_PATH%\Common7\Tools\VsDevCmd.bat"
if %ERRORLEVEL% NEQ 0 exit /b %ERRORLEVEL%
if "%MSBUILD_TARGET%"=="" set MSBUILD_TARGET=Rebuild
msbuild %1 /p:Configuration=%2 /p:Platform=%3 /t:%MSBUILD_TARGET% /m:8 /v:minimal /nologo
