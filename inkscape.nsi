; #######################################
; Inkscape NSIS installer project file
; Used as of 0.40
; #######################################

; #######################################
; DEFINES
; #######################################
!define PRODUCT_NAME "Inkscape"
!define PRODUCT_VERSION "0.41+cvs"
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
!define MUI_ICON "${NSISDIR}\Contrib\Graphics\Icons\modern-install.ico"
!define MUI_UNICON "${NSISDIR}\Contrib\Graphics\Icons\modern-uninstall.ico"

; Welcome page
!insertmacro MUI_PAGE_WELCOME
; License page
!define MUI_LICENSEPAGE_RADIOBUTTONS
!insertmacro MUI_PAGE_LICENSE "Copying"
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
!insertmacro MUI_LANGUAGE "Catalan"
!insertmacro MUI_LANGUAGE "German"
!insertmacro MUI_LANGUAGE "Italian"



; #######################################
; STRING   LOCALIZATION
; #######################################
; Thanks to Adib Taraben and Luca Bruno for getting this started
; Add your translation here!  :-)

; Product name
LangString lng_Caption ${LANG_CATALAN}   "${PRODUCT_NAME} -- Editor de gràfics vectorials escalables de codi obert"
LangString lng_Caption ${LANG_CZECH}     "${PRODUCT_NAME} -- Open Source Editor SVG vektorové grafiky"
LangString lng_Caption ${LANG_ENGLISH}   "${PRODUCT_NAME} -- Open Source Scalable Vector Graphics Editor"
LangString lng_Caption ${LANG_FRENCH}    "${PRODUCT_NAME} -- Logiciel de dessin vectoriel libre SVG"
LangString lng_Caption ${LANG_GERMAN}    "${PRODUCT_NAME} -- Open Source SVG-Vektorillustrator"
LangString lng_Caption ${LANG_ITALIAN}   "${PRODUCT_NAME} -- Editor di grafica vettoriale Open Source"

; File type association for editing
LangString lng_Editor ${LANG_CATALAN}    "Voleu que l'$(^Name) sigui l'editor SVG predeterminat?"
LangString lng_Editor ${LANG_CZECH}      "Chcete nastavit $(^Name) jako výchozí editor SVG souborù?"
LangString lng_Editor ${LANG_ENGLISH}    "Do you want $(^Name) to be the default SVG editor?"
LangString lng_Editor ${LANG_FRENCH}     "Voulez-vous qu' $(^Name) devienne l'éditeur SVG par défaut?"
LangString lng_Editor ${LANG_GERMAN}     "Wollen Sie $(^Name) zu Ihrem standardmäßigem Bearbeitungsprogramm für SVG machen?"
LangString lng_Editor ${LANG_ITALIAN}    "Impostare $(^Name) come editor SVG predefinito?"

; File type association for reading
LangString lng_Reader ${LANG_CATALAN}    "Voleu que l'$(^Name) sigui el lector SVG predefinit?"
LangString lng_Editor ${LANG_CZECH}      "Chcete nastavit $(^Name) jako výchozí prohlí¾eè SVG souborù?"
LangString lng_Reader ${LANG_ENGLISH}    "Do you want $(^Name) to be the default SVG reader?"
LangString lng_Reader ${LANG_FRENCH}     "Voulez-vous qu' $(^Name) devienne la visionneuse SVG par défaut?"
LangString lng_Reader ${LANG_GERMAN}     "Wollen Sie $(^Name) zu Ihrem standardmäßigem Anzeigeprogramm für SVG machen?"
LangString lng_Reader ${LANG_ITALIAN}    "Impostare $(^Name) come lettore SVG predefinito?"

; Post-Removal notice
LangString lng_Removed ${LANG_CATALAN}   "L'$(^Name) s'ha suprimit correctament de l'ordinador."
LangString lng_Removed ${LANG_CZECH}     "$(^Name) byl úspì¹nì odinstalován z va¹eho poèítaèe."
LangString lng_Removed ${LANG_ENGLISH}   "$(^Name) was successfully removed from your computer."
LangString lng_Removed ${LANG_FRENCH}    "$(^Name) a été desinstallé avec succès de votre ordinateur."
LangString lng_Removed ${LANG_GERMAN}    "$(^Name) wurde erfolgreich von Ihrem Computer entfernt."
LangString lng_Removed ${LANG_ITALIAN}   "$(^Name) è stato rimosso con successo dal sistema."

; Ask to remove
LangString lng_Uninstall ${LANG_CATALAN} "Esteu segur que voleu suprimir completament $(^Name) i tots els seus components?"
LangString lng_Uninstall ${LANG_CZECH}   "Jste si jisti, ¾e chcete úplnì odstranit $(^Name) a v¹echny jeho komponenty?"
LangString lng_Uninstall ${LANG_ENGLISH} "Are you sure you want to completely remove $(^Name) and all of its components?"
LangString lng_Uninstall ${LANG_FRENCH}  "Etes-vous-sûr de vouloir complètement desintaller $(^Name) et tous ses composants?"
LangString lng_Uninstall ${LANG_GERMAN}  "Möchten Sie $(^Name) und alle seine Komponenten von Ihrem Rechner entfernen?"
LangString lng_Uninstall ${LANG_ITALIAN} "Rimuovere completamente $(^Name) e tutti i suoi componenti?"




; #######################################
; SETTINGS
; #######################################

Name              "${PRODUCT_NAME} ${PRODUCT_VERSION}"
Caption           $(lng_Caption)
OutFile           "Inkscape-${PRODUCT_VERSION}-1.win32.exe"
InstallDir        "$PROGRAMFILES\Inkscape"
InstallDirRegKey  HKLM "${PRODUCT_DIR_REGKEY}" ""
ShowInstDetails   show
ShowUnInstDetails show




; #######################################
;  I N S T A L L E R    S E C T I O N S
; #######################################


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

  MessageBox MB_YESNO|MB_ICONQUESTION $(lng_Editor) IDNO NoEditor
  WriteRegStr HKCR ".svg" "" "svgfile"
  WriteRegStr HKCR ".svgz" "" "svgfile"
  WriteRegStr HKCR "svgfile" "" "Scalable Vector Graphics file"
  WriteRegStr HKCR "svgfile\shell\edit\command" "" '$INSTDIR\Inkscape.exe "%1"'
  NoEditor:

  MessageBox MB_YESNO|MB_ICONQUESTION $(lng_Reader) IDNO NoReader
  WriteRegStr HKCR ".svg" "" "svgfile"
  WriteRegStr HKCR ".svgz" "" "svgfile"
  WriteRegStr HKCR "svgfile" "" "Scalable Vector Graphics file"
  WriteRegStr HKCR "svgfile\shell\open\command" "" '$INSTDIR\Inkscape.exe "%1"'
  NoReader:
SectionEnd



Function un.onUninstSuccess
  HideWindow
  MessageBox MB_ICONINFORMATION|MB_OK $(lng_Removed)
FunctionEnd



Function un.onInit
  MessageBox MB_ICONQUESTION|MB_YESNO|MB_DEFBUTTON2 $(lng_Uninstall) IDYES +2
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



