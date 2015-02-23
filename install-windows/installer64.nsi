SetCompressor /SOLID /FINAL lzma

Name "Wal Commander GitHub Edition 0.19.0 (64-bit)"

; The file to write
OutFile "WalCommanderGitHub-0.18.0-x64.exe"

; The default installation directory
InstallDir "D:\Program Files\WalCommander"

; Request application privileges for Windows Vista
RequestExecutionLevel admin

Icon ..\src\small.ico

LicenseText "License Agreement"
LicenseData ..\LICENSE

!include "MUI2.nsh"
!define MUI_ABORTWARNING
!define MUI_ICON "..\src\small.ico"
!define MUI_HEADERIMAGE
!define MUI_HEADERIMAGE_RIGHT
!define MUI_HEADER_TRANSPARENT_TEXT
!define MUI_WELCOMEFINISHPAGE_BITMAP_NOSTRETCH

; Welcome page
!insertmacro MUI_PAGE_WELCOME

; License page
!define MUI_LICENSEPAGE_CHECKBOX
!insertmacro MUI_PAGE_LICENSE "..\LICENSE"

; Directory page
!insertmacro MUI_PAGE_DIRECTORY

; Instfiles page
!insertmacro MUI_PAGE_INSTFILES

Var StartMenuFolder
!define MUI_STARTMENUPAGE_DEFAULTFOLDER "Wal Commander"
!define MUI_STARTMENUPAGE_REGISTRY_VALUENAME "Start Menu Folder"
!insertmacro MUI_PAGE_STARTMENU Application $StartMenuFolder

; Finish page
!define MUI_FINISHPAGE_SHOWREADME "$INSTDIR\readme_eng.txt"
!define MUI_FINISHPAGE_NOREBOOTSUPPORT
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_LANGUAGE "English"

;--------------------------------

; Pages

;Page license 
;Page directory
;Page instfiles

UninstPage uninstConfirm
UninstPage instfiles

;--------------------------------

; The stuff to install
Section "" ;No components page, name is not important
	SectionIn RO

  ; Set output path to the installation directory.
  SetOutPath $INSTDIR
  
  ; Put file there
  File /r Temp\*.*

  ; Write the uninstall keys for Windows
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\WalCommander" "DisplayName" "Wal Commander GitHub Edition"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\WalCommander" "UninstallString" '"$INSTDIR\uninstall.exe"'
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\WalCommander" "NoModify" 1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\WalCommander" "NoRepair" 1
  WriteUninstaller "uninstall.exe"

   ; include for some of the windows messages defines
   !include "winmessages.nsh"
   ; HKLM (all users) vs HKCU (current user) defines
   !define env_hklm 'HKLM "SYSTEM\CurrentControlSet\Control\Session Manager\Environment"'
   !define env_hkcu 'HKCU "Environment"'
SectionEnd ; end the section

Section "Start Menu Shortcuts"
  !insertmacro MUI_STARTMENU_WRITE_BEGIN Application
    
	;Create shortcuts
	CreateDirectory "$SMPROGRAMS\$StartMenuFolder"
	SetOutPath $INSTDIR\Tools\ProjectWizard
	CreateShortCut "$SMPROGRAMS\$StartMenuFolder\Wal Commander.lnk" "$INSTDIR\wcm.exe" "" "$INSTDIR\small.ico" 0
	SetOutPath $INSTDIR
	CreateShortCut "$SMPROGRAMS\$StartMenuFolder\Uninstall.lnk" "$INSTDIR\uninstall.exe" "" "$INSTDIR\uninstall.exe" 0
	CreateShortCut "$SMPROGRAMS\$StartMenuFolder\readme_eng.txt"    "$INSTDIR\readme_eng.txt"
  
   !insertmacro MUI_STARTMENU_WRITE_END

	CreateShortCut "$DESKTOP\Wal Commander.lnk" "$INSTDIR\" "" "$INSTDIR\small.ico"

;  CreateShortCut "$DESKTOP\Asteroids.lnk" "$INSTDIR\Launcher2.exe"
SectionEnd

;--------------------------------

; Uninstaller

Function un.onInit
	MessageBox MB_ICONQUESTION|MB_YESNO|MB_DEFBUTTON2 "Are you sure you want to completely remove Wal Commander GitHub Edition and all of its components?" IDYES +2
	Abort
FunctionEnd

Section "Uninstall"
  ; Remove registry keys
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\WalCommander"
  DeleteRegKey HKLM SOFTWARE\WalCommander


  ; Remove directories used
  RMDir /r "$INSTDIR"

  ; Remove shortcuts, if any
  Delete "$DESKTOP\Wal Commander.lnk"

  !insertmacro MUI_STARTMENU_GETFOLDER Application $StartMenuFolder
  Delete "$SMPROGRAMS\$StartMenuFolder\*.*"
  RMDir "$SMPROGRAMS\$StartMenuFolder"
SectionEnd

