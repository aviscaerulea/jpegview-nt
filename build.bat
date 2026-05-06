@echo off
rem Initialize VS developer environment and run msbuild
call "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\Common7\Tools\VsDevCmd.bat"
if %ERRORLEVEL% NEQ 0 exit /b %ERRORLEVEL%
msbuild %1 /p:Configuration=%2 /p:Platform=%3 /t:Rebuild /m:8 /v:minimal /nologo
