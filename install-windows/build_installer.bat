rem @echo off

rem
rem Run this from 'install-windows' folder
rem

rem Prepare binaries

cd ..
make all -j5 -B WINDOWS_TARGET=-m64
if ERRORLEVEL 1 exit 1
strip wcm.exe
cd install-windows

rem Prepare SDK directory structure

md Temp
copy ..\wcm.exe Temp\wcm.exe
copy ..\src\small.ico Temp\small.ico
copy ..\LICENSE Temp\LICENSE
copy ..\CHANGELOG.GitHub Temp\CHANGELOG.GitHub
copy ..\readme_eng.txt Temp\readme_eng.txt
xcopy ..\install-files\share\wcm Temp /S /Y

"D:\Program Files (x86)\NSIS\makensis.exe" installer64.nsi

cd Temp
"C:\Program Files\7-Zip\7z.exe" a WalCommanderGitHub-0.18.0-x64.zip .
cd ..
copy Temp\WalCommanderGitHub-0.18.0-x64.zip .

erase Temp /F /S /Q

if not exist WalCommanderGitHub-0.18.0-x64.exe exit 1
