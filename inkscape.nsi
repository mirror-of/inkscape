;-------------------------------------------------

Name "Inkscape"
Caption "Inkscape Installer"
Icon "inkscape32x16col.ico"
OutFile "InkscapeInstaller.exe"

; The default installation directory
InstallDir "c:\Inkscape"

;-------------------------------------------------

; Pages

Page directory
Page instfiles

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
    RMDir "$INSTDIR" ; skipped if no
  NoDelete:
  
  IfFileExists "$INSTDIR" 0 NoErrorMsg
    MessageBox MB_OK "Note: $INSTDIR could not be removed!" IDOK 0 ; skipped if file doesn't exist
  NoErrorMsg:

SectionEnd
