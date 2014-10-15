@echo off

set DirectoryRef=
set DirectoryRefClose=/DirectoryRef

echo ^<DirectoryRef Id="SUBDIR_fonts"^>
dir /b ..\..\wcm\install-files\share\wcm\fonts | gen_wix_comps.bat 1 1 comp_tpl.txt install-files\share\wcm\fonts
echo ^<%DirectoryRefClose%^>

echo ^<DirectoryRef Id="SUBDIR_icon"^>
dir /b ..\..\wcm\install-files\share\wcm\icon | gen_wix_comps.bat 1 2 comp_tpl.txt install-files\share\wcm\icon
echo ^<%DirectoryRefClose%^>


echo ^<DirectoryRef Id="SUBDIR_lang"^>
dir /b ..\..\wcm\install-files\share\wcm\lang | gen_wix_comps.bat 1 3 comp_tpl.txt install-files\share\wcm\lang
echo ^<%DirectoryRefClose%^>

echo ^<DirectoryRef Id="SUBDIR_shl"^>
dir /b ..\..\wcm\install-files\share\wcm\shl | gen_wix_comps.bat 1 4 comp_tpl.txt install-files\share\wcm\shl
echo ^<%DirectoryRefClose%^>

