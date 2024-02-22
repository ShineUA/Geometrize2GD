@echo off

tasklist /fi "ImageName eq GeometryDash.exe" /fo csv 2>NUL | find /I "geometrydash.exe">NUL
if "%ERRORLEVEL%"=="0" taskkill /im GeometryDash.exe
geode run --background