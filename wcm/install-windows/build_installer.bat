rem @echo off

rem
rem Run this from 'git.master' folder
rem


rem Prepare x86 binaries

cd ..
make all -j4 -B
if ERRORLEVEL 1 exit 1
strip wcm.exe
cd install-windows

rem Prepare SDK directory structure

md Temp
copy ..\wcm.exe Temp\wcm.exe
copy ..\small.ico Temp\small.ico
copy ..\LICENSE Temp\LICENSE
copy ..\readme_eng.txt Temp\readme_eng.txt
xcopy ..\install-files\share\wcm Temp /S /Y

"D:\Program Files (x86)\NSIS\makensis.exe" installer32.nsi

cd Temp
"C:\Program Files\7-Zip\7z.exe" a WalCommanderGitHub-0.17.0-x86.zip .
cd ..
copy Temp\WalCommanderGitHub-0.17.0-x86.zip .

erase Temp /F /S /Q

if not exist WalCommanderGitHub-0.17.0-x86.exe exit 1

rem Prepare x64 binaries

cd ..
make all -j4 -B WINDOWS_TARGET=-m64
if ERRORLEVEL 1 exit 1
strip wcm.exe
cd install-windows

rem Prepare SDK directory structure

md Temp64
copy ..\wcm.exe Temp64\wcm.exe
copy ..\small.ico Temp64\small.ico
copy ..\LICENSE Temp64\LICENSE
copy ..\readme_eng.txt Temp64\readme_eng.txt
xcopy ..\install-files\share\wcm Temp64 /S /Y

"D:\Program Files (x86)\NSIS\makensis.exe" installer64.nsi

cd Temp64
"C:\Program Files\7-Zip\7z.exe" a WalCommanderGitHub-0.17.0-x64.zip .
cd ..
copy Temp64\WalCommanderGitHub-0.17.0-x64.zip .

erase Temp64 /F /S /Q

if not exist WalCommanderGitHub-0.17.0-x64.exe exit 1
