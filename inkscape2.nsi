; #######################################
; Inkscape NSIS installer project file
; Used as of 0.38.1
; #######################################

; #######################################
; DEFINES
; #######################################
!define PRODUCT_NAME "Inkscape"
!define PRODUCT_VERSION "0.38.1"
!define PRODUCT_PUBLISHER "Inkscape Organization"
!define PRODUCT_WEB_SITE "http://www.inkscape.org"
!define PRODUCT_DIR_REGKEY "Software\Microsoft\Windows\CurrentVersion\App Paths\inkscape.exe"
!define PRODUCT_UNINST_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}"
!define PRODUCT_UNINST_ROOT_KEY "HKLM"

; #######################################
; MUI   SETTINGS
; #######################################
; MUI 1.67 compatible ------
!include "MUI.nsh"
!define MUI_ABORTWARNING
!define MUI_ICON "${NSISDIR}\Contrib\Graphics\Icons\win-install.ico"
!define MUI_UNICON "${NSISDIR}\Contrib\Graphics\Icons\win-uninstall.ico"

; Welcome page
!insertmacro MUI_PAGE_WELCOME
; License page
!define MUI_LICENSEPAGE_RADIOBUTTONS
!insertmacro MUI_PAGE_LICENSE "C:\ink\inkscape\Copying"
; Directory page
!insertmacro MUI_PAGE_DIRECTORY
; Instfiles page
!insertmacro MUI_PAGE_INSTFILES
; Finish page
!define MUI_FINISHPAGE_RUN "$INSTDIR\inkscape.exe"
!insertmacro MUI_PAGE_FINISH

; Uninstaller pages
!insertmacro MUI_UNPAGE_INSTFILES

; Language files
!insertmacro MUI_LANGUAGE "English"

; MUI end ------




Name "${PRODUCT_NAME} ${PRODUCT_VERSION}"
OutFile "Inkscape-0.38.1-1.win32.exe"
InstallDir "$PROGRAMFILES\Inkscape"
InstallDirRegKey HKLM "${PRODUCT_DIR_REGKEY}" ""
ShowInstDetails show
ShowUnInstDetails show

Section "MainSection" SEC01
  SetOutPath "$INSTDIR"
  SetOverwrite ifnewer
  File /a /r "inkscape\*.*"

  CreateDirectory "$SMPROGRAMS\Inkscape"
  CreateShortCut "$SMPROGRAMS\Inkscape\Inkscape.lnk" "$INSTDIR\inkscape.exe"
  CreateShortCut "$DESKTOP\Inkscape.lnk" "$INSTDIR\inkscape.exe"
SectionEnd

Section -AdditionalIcons
  CreateShortCut "$SMPROGRAMS\Inkscape\Uninstall Inkscape.lnk" "$INSTDIR\uninst.exe"
SectionEnd

Section -Post
  WriteUninstaller "$INSTDIR\uninst.exe"

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
SectionEnd


Function un.onUninstSuccess
  HideWindow
  MessageBox MB_ICONINFORMATION|MB_OK "$(^Name) was successfully removed from your computer."
FunctionEnd

Function un.onInit
  MessageBox MB_ICONQUESTION|MB_YESNO|MB_DEFBUTTON2 "Are you sure you want to completely remove $(^Name) and all of its components?" IDYES +2
  Abort
FunctionEnd

Section Uninstall
  Delete   "$DESKTOP\Inkscape.lnk"

  Delete "$SMPROGRAMS\Inkscape\Uninstall Inkscape.lnk"
  Delete "$SMPROGRAMS\Inkscape\Inkscape.lnk"
  RMDir  "$SMPROGRAMS\Inkscape"

  RMDir /r "$INSTDIR"

  SetAutoClose true
SectionEnd


