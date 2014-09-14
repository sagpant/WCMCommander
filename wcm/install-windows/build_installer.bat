rem @echo off

rem
rem Run this from 'git.master' folder
rem


rem Prepare binaries

cd ..
make all -j4 -B
if ERRORLEVEL 1 exit 1

cd install-windows

rem Prepare SDK directory structure

md Temp
copy ..\wcm.exe Temp\wcm.exe
copy ..\small.ico Temp\small.ico
copy ..\LICENSE Temp\LICENSE
copy ..\readme_eng.txt Temp\readme_eng.txt
xcopy ..\install-files\share\wcm Temp /S /Y

"D:\Program Files (x86)\NSIS\makensis.exe" installer.nsi

erase Temp /F /S /Q

if not exist WalCommanderGitHub-0.16.2.exe exit 1
