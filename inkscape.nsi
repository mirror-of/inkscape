;-------------------------------------------------

Name "Inkscape"
Caption "Inkscape -- Open Source Scalable Vector Graphics Editor"
Icon "inkscape32x16col.ico"
OutFile "inkscape-0.37-2.win32.exe"

; The default installation directory
InstallDir "c:\Inkscape"
LicenseData "COPYING"

;-------------------------------------------------

; Pages

Page license
Page directory
Page instfiles
UninstPage uninstConfirm
UninstPage instfiles

;-------------------------------------------------

Section ""

  SetOutPath $INSTDIR
  File /a /r "inkscape\*.*"

  MessageBox MB_YESNO|MB_ICONQUESTION "Do you want Inkscape to be the default SVG editor?" IDNO NoEditor
  WriteRegStr HKCR ".svg" "" "svgfile"
  WriteRegStr HKCR "svgfile" "" "Scalable Vector Graphics file"
  WriteRegStr HKCR "svgfile\shell\edit\command" "" '$INSTDIR\Inkscape.exe "%1"'
  NoEditor:

  MessageBox MB_YESNO|MB_ICONQUESTION "Do you want Inkscape to be the default SVG reader?" IDNO NoReader
  WriteRegStr HKCR ".svg" "" "svgfile"
  WriteRegStr HKCR "svgfile" "" "Scalable Vector Graphics file"
  WriteRegStr HKCR "svgfile\shell\open\command" "" '$INSTDIR\Inkscape.exe "%1"'
  NoReader:

  WriteUninstaller "inkscape-uninstall.exe"


SectionEnd

;-------------------------------------------------

Section "Uninstall"

  MessageBox MB_YESNO|MB_ICONQUESTION "Would you like to uninstall Inkscape?" IDNO NoDelete
    RMDir /r "$INSTDIR" ; skipped if no
  NoDelete:
  
  IfFileExists "$INSTDIR" 0 NoErrorMsg
    MessageBox MB_OK "Note: $INSTDIR could not be removed!" IDOK 0 ; skipped if file doesn't exist
  NoErrorMsg:

SectionEnd
