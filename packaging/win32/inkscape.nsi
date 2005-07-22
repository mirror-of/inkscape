; #######################################
; Inkscape NSIS installer project file
; Used as of 0.40
; #######################################

; #######################################
; DEFINES
; #######################################
!define PRODUCT_NAME "Inkscape"
!define PRODUCT_VERSION "0.41+0.42pre3"
!define PRODUCT_PUBLISHER "Inkscape Organization"
!define PRODUCT_WEB_SITE "http://www.inkscape.org"
!define PRODUCT_DIR_REGKEY "Software\Microsoft\Windows\CurrentVersion\App Paths\inkscape.exe"
!define PRODUCT_UNINST_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}"



; #######################################
; MUI   SETTINGS
; #######################################
; MUI 1.67 compatible ------
SetCompressor /SOLID lzma
!include "MUI.nsh"
!define MUI_ABORTWARNING
!define MUI_ICON "${NSISDIR}\Contrib\Graphics\Icons\modern-install.ico"
!define MUI_UNICON "${NSISDIR}\Contrib\Graphics\Icons\modern-uninstall.ico"


; Welcome page
!insertmacro MUI_PAGE_WELCOME
; License page
!define MUI_LICENSEPAGE_RADIOBUTTONS
!insertmacro MUI_PAGE_LICENSE "..\..\Copying"
Page custom CustomPageSingleuser
Page custom CustomPageMultiuser
; Directory page
!insertmacro MUI_PAGE_DIRECTORY
; Instfiles page
!insertmacro MUI_PAGE_INSTFILES
; Finish page
!define MUI_FINISHPAGE_RUN "$INSTDIR\inkscape.exe"
!insertmacro MUI_PAGE_FINISH

; Uninstaller pages
!insertmacro MUI_UNPAGE_CONFIRM
UninstPage custom un.CustomPageUninstall
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_UNPAGE_FINISH

; Language files
!insertmacro MUI_LANGUAGE "English"
;!insertmacro MUI_LANGUAGE "Catalan"
!insertmacro MUI_LANGUAGE "Czech"
!insertmacro MUI_LANGUAGE "French"
!insertmacro MUI_LANGUAGE "German"
!insertmacro MUI_LANGUAGE "Italian"
!insertmacro MUI_LANGUAGE "Polish"

ReserveFile "inkscape.nsi.singleuser"
ReserveFile "inkscape.nsi.multiuser"
ReserveFile "inkscape.nsi.uninstall"




; #######################################
; STRING   LOCALIZATION
; #######################################
; Thanks to Adib Taraben and Luca Bruno for getting this started
; Add your translation here!  :-)
; I had wanted to list the languages alphabetically, but apparently
; the first is the default.  So putting English first is just being
; practical.  It is not chauvinism or hubris, I swear!  ;-)
; default language first

; Product name
LangString lng_Caption   ${LANG_ENGLISH} "${PRODUCT_NAME} -- Open Source Scalable Vector Graphics Editor"
;LangString lng_Caption   ${LANG_CATALAN} "${PRODUCT_NAME} -- Editor de gràfics vectorials escalables de codi obert"
LangString lng_Caption   ${LANG_CZECH}   "${PRODUCT_NAME} -- Open Source Editor SVG vektorové grafiky"
LangString lng_Caption   ${LANG_FRENCH}  "${PRODUCT_NAME} -- Logiciel de dessin vectoriel SVG libre"
LangString lng_Caption   ${LANG_GERMAN}  "${PRODUCT_NAME} -- Open Source SVG-Vektorillustrator"
LangString lng_Caption   ${LANG_ITALIAN} "${PRODUCT_NAME} -- Editor di grafica vettoriale Open Source"
LangString lng_Caption   ${LANG_POLISH}  "${PRODUCT_NAME} -- Edytor Grafiki Wektorowej SVG Open Source"


; installation options
LangString lng_InstOpt   ${LANG_ENGLISH} "Installation options"
LangString lng_InstOpt   ${LANG_CZECH} "Volby instalace"
LangString lng_InstOpt   ${LANG_FRENCH} "Options d'installation"
LangString lng_InstOpt   ${LANG_GERMAN} "Installations Optionen"
LangString lng_InstOpt   ${LANG_ITALIAN} "Preferenze per l'installazione"
LangString lng_InstOpt   ${LANG_POLISH} "Opcje instalacji"

; installation options subtitle
LangString lng_InstOpt1  ${LANG_ENGLISH} "Please make your choices for additional options"
LangString lng_InstOpt1  ${LANG_CZECH} "Prosíme vyberte z následujících možností"
LangString lng_InstOpt1  ${LANG_FRENCH} "Veuillez choisir parmi les options additionnelles"
LangString lng_InstOpt1  ${LANG_GERMAN} "Bitte wählen Sie die optionalen Installtionsparameter"
LangString lng_InstOpt1  ${LANG_ITALIAN} "Completare le opzioni aggiuntive per l'installazione"
LangString lng_InstOpt1  ${LANG_POLISH} "Wybierz dodatkowe opcje instalacji"

; installation type
LangString lng_InstType  ${LANG_ENGLISH} "Install this application for:"
LangString lng_InstType  ${LANG_CZECH} "Instalovat aplikaci pro:"
LangString lng_InstType  ${LANG_FRENCH} "Installer cette application pour :"
LangString lng_InstType  ${LANG_GERMAN}  "Installiert diese Anwendung für:"
LangString lng_InstType  ${LANG_ITALIAN} "Installare questa applicazione per:"
LangString lng_InstType  ${LANG_POLISH} "Zainstaluj aplikacjê dla:"

; multi user installation
LangString lng_AllUsers  ${LANG_ENGLISH} "Anyone who uses this computer (all users)"
LangString lng_AllUsers  ${LANG_CZECH} "Kohokoliv kdo používá tento poèítaè (Všichni uživatelé)"
LangString lng_AllUsers  ${LANG_FRENCH} "Toute personne utilisant cet ordinateur (tous les utilisateurs)"
LangString lng_AllUSers  ${LANG_GERMAN}  "Alle Benutzer dieses Computers (all users)"
LangString lng_AllUSers  ${LANG_ITALIAN}  "Chiunque usi il computer (tutti gli utenti)"
LangString lng_AllUsers  ${LANG_POLISH} "Wszystkich, którzy korzystaj¹ z tego komputera (wszyscy u¿ytkownicy)"

; single user installation
LangString lng_CurUser  ${LANG_ENGLISH} "Only for me (current user)"
LangString lng_CurUser  ${LANG_CZECH} "Pouze pro mne (aktuální uživatel)"
LangString lng_CurUser  ${LANG_FRENCH} "Moi seulement (utilisateur courant)"
LangString lng_CurUSer  ${LANG_GERMAN}  "Nur für mich (current user)"
LangString lng_CurUser  ${LANG_ITALIAN} "Solo per me (utente attuale)"
LangString lng_CurUser  ${LANG_POLISH} "Tylko dla mnie (aktualny u¿ytkownik)"

; File type association
LangString lng_UseAs ${LANG_ENGLISH} "Select ${PRODUCT_NAME} as default application for:"
LangString lng_UseAs ${LANG_CZECH} "Nastavit ${PRODUCT_NAME} jako výchozí aplikaci pro:"
LangString lng_UseAs ${LANG_FRENCH} "Sélectionner ${PRODUCT_NAME} comme application par défaut pour :"
LangString lng_UseAs    ${LANG_GERMAN}  "Wählen Sie ${PRODUCT_NAME} als Standardanwendung zum:"
LangString lng_UseAs    ${LANG_ITALIAN} "Impostare ${PRODUCT_NAME} come scelta predefinita per:"
LangString lng_UseAs ${LANG_POLISH} "Wybierz ${PRODUCT_NAME} jako domyœlny program do:"

; File type association for editing
LangString lng_Editor    ${LANG_ENGLISH} "editor for SVG files"
;LangString lng_Editor    ${LANG_CATALAN} "Voleu que l'$(^Name) sigui l'editor SVG predeterminat?"
LangString lng_Editor    ${LANG_CZECH}   "úpravu SVG souborù"
LangString lng_Editor    ${LANG_FRENCH}  "éditer des fichiers SVG"
LangString lng_Editor    ${LANG_GERMAN}  "Bearbeiten für SVG Dateien machen"
LangString lng_Editor    ${LANG_ITALIAN} "modificare file SVG"
LangString lng_Editor    ${LANG_POLISH}  "edycji plików SVG"

; File type association for reading
LangString lng_Reader    ${LANG_ENGLISH} "reader for SVG files"
;LangString lng_Reader    ${LANG_CATALAN} "Voleu que l'$(^Name) sigui el lector SVG predefinit?"
LangString lng_Reader    ${LANG_CZECH}   "prohlížení SVG souborù"
LangString lng_Reader    ${LANG_FRENCH}  "lire des fichiers SVG"
LangString lng_Reader    ${LANG_GERMAN}  "Anzeigeprogramm für SVG Dateien"
LangString lng_Reader    ${LANG_ITALIAN} "leggere file SVG"
LangString lng_Reader    ${LANG_POLISH}  "otwierania plików SVG"

; Post-Removal notice
;not needed anymore, standard dialog is used
;LangString lng_Removed   ${LANG_ENGLISH} "$(^Name) was successfully removed from your computer."
;LangString lng_Removed   ${LANG_CATALAN} "L'$(^Name) s'ha suprimit correctament de l'ordinador."
;LangString lng_Removed   ${LANG_CZECH}   "$(^Name) byl úspìšnì odinstalován z vašeho poèítaèe."
;LangString lng_Removed   ${LANG_FRENCH}  "$(^Name) a été désinstallé avec succès de votre ordinateur."
;LangString lng_Removed   ${LANG_GERMAN}  "$(^Name) wurde erfolgreich von Ihrem Computer entfernt."
;LangString lng_Removed   ${LANG_ITALIAN} "$(^Name) è stato rimosso con successo dal sistema."

; Ask to remove
;not needed anymore, standard dialog is used
;LangString lng_Uninstall ${LANG_ENGLISH} "Are you sure you want to completely remove $(^Name) and all of its components?"
;LangString lng_Uninstall ${LANG_CATALAN} "Esteu segur que voleu suprimir completament $(^Name) i tots els seus components?"
;LangString lng_Uninstall ${LANG_CZECH}   "Jste si jisti, že chcete úplnì odstranit $(^Name) a všechny jeho komponenty?"
;LangString lng_Uninstall ${LANG_FRENCH}  "Etes-vous sûr de vouloir complètement désintaller $(^Name) et tous ses composants?"
;LangString lng_Uninstall ${LANG_GERMAN}  "Möchten Sie $(^Name) und alle seine Komponenten von Ihrem Rechner entfernen?"
;LangString lng_Uninstall ${LANG_ITALIAN} "Rimuovere completamente $(^Name) e tutti i suoi componenti?"

; uninstallation options
LangString lng_UInstOpt   ${LANG_ENGLISH} "Uninstallation Options"
LangString lng_UInstOpt   ${LANG_CZECH} "Volby pro Odinstalaci"
LangString lng_UInstOpt   ${LANG_FRENCH} "Options de désinstallation"
LangString lng_UInstOpt   ${LANG_GERMAN} "Deinstallations Optionen"
LangString lng_UInstOpt   ${LANG_ITALIAN} "Preferenze per la rimozione"
LangString lng_UInstOpt   ${LANG_POLISH} "Opcje usuwania programu"

; uninstallation options subtitle
LangString lng_UInstOpt1  ${LANG_ENGLISH} "Please make your choices for additional options"
LangString lng_UInstOpt1  ${LANG_CZECH} "Prosíme vyberte z následujících možností"
LangString lng_UInstOpt1  ${LANG_FRENCH} "Veuillez choisir parmi les options additionnelles"
LangString lng_UInstOpt1  ${LANG_GERMAN} "Bitte wählen Sie die optionalen Deinstalltionsparameter"
LangString lng_UInstOpt1  ${LANG_ITALIAN} "Completare le opzioni aggiuntive per la rimozione"
LangString lng_UInstOpt1  ${LANG_POLISH} "Wybierz dodatkowe opcje usuwania programu"

; Ask to purge the personal preferences
LangString lng_PurgePrefs ${LANG_ENGLISH} "Do you want to keep your personal preferences file?"
LangString lng_PurgePrefs ${LANG_CZECH} "Chcete zachovat váš osobní soubor s nastavením?"
LangString lng_PurgePrefs ${LANG_FRENCH} "Voulez-vous conserver votre fichier de préférences personnelles ?"
LangString lng_PurgePrefs ${LANG_GERMAN}  "Möchten Sie Ihre persönliche Vorgabendatei behalten?"
LangString lng_PurgePrefs ${LANG_ITALIAN} "Mantenere i file con le configurazioni personali?"
LangString lng_PurgePrefs ${LANG_POLISH} "Czy chcesz zachowaæ plik z w³asnymi preferencjami?"


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

var askMultiUser
Var MultiUser
Var Editor
Var Viewer


; #######################################
;  I N S T A L L E R    S E C T I O N S
; #######################################

; Turn off old selected section
; GetWindowsVersion
;
; Based on Yazno's function, http://yazno.tripod.com/powerpimpit/
; Updated by Joost Verburg
; Updated for Windows 98 SE by Matthew Win Tibbals 5-21-03
;
; Returns on top of stack
;
; Windows Version (95, 98, ME, NT x.x, 2000, XP, 2003)
; or
; '' (Unknown Windows Version)
;
; Usage:
;   Call GetWindowsVersion
;   Pop $R0
;   ; at this point $R0 is "NT 4.0" or whatnot
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Function GetWindowsVersion
 
  Push $R0
  Push $R1
 
  ClearErrors
 
  ReadRegStr $R0 HKLM "SOFTWARE\Microsoft\Windows NT\CurrentVersion" CurrentVersion
 
  IfErrors 0 lbl_winnt
 
  ; we are not NT
  ReadRegStr $R0 HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion" VersionNumber
 
  StrCpy $R1 $R0 1
  StrCmp $R1 '4' 0 lbl_error
 
  StrCpy $R1 $R0 3
 
  StrCmp $R1 '4.0' lbl_win32_95
  StrCmp $R1 '4.9' lbl_win32_ME lbl_win32_98
 
  lbl_win32_95:
    StrCpy $R0 '95'
	StrCpy $AskMultiUser "0"
  Goto lbl_done
 
  lbl_win32_98:
    StrCpy $R0 '98'
	StrCpy $AskMultiUser "0"
  Goto lbl_done
  lbl_win32_ME:
    StrCpy $R0 'ME'
	StrCpy $AskMultiUser "0"
  Goto lbl_done
 
  lbl_winnt:
 
  StrCpy $R1 $R0 1
 
  StrCmp $R1 '3' lbl_winnt_x
  StrCmp $R1 '4' lbl_winnt_x
 
  StrCpy $R1 $R0 3
 
  StrCmp $R1 '5.0' lbl_winnt_2000
  StrCmp $R1 '5.1' lbl_winnt_XP
  StrCmp $R1 '5.2' lbl_winnt_2003 lbl_error
 
  lbl_winnt_x:
    StrCpy $R0 "NT $R0" 6
  Goto lbl_done
 
  lbl_winnt_2000:
    Strcpy $R0 '2000'
  Goto lbl_done
 
  lbl_winnt_XP:
    Strcpy $R0 'XP'
  Goto lbl_done
 
  lbl_winnt_2003:
    Strcpy $R0 '2003'
  Goto lbl_done
 
  lbl_error:
    Strcpy $R0 ''
  lbl_done:
 
  Pop $R1
  Exch $R0
 
FunctionEnd
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;; 



Function .onInit
  ;Extract InstallOptions INI files
  !insertmacro MUI_INSTALLOPTIONS_EXTRACT "inkscape.nsi.singleuser"
  !insertmacro MUI_INSTALLOPTIONS_EXTRACT "inkscape.nsi.multiuser"
  
  StrCpy $AskMultiUser "1"
  StrCpy $MultiUser "1"
  Call GetWindowsVersion
  Pop $R0
  DetailPrint "detected operating system $R0"
FunctionEnd


Function CustomPageMultiUser
  !insertmacro MUI_HEADER_TEXT "$(lng_InstOpt)" "$(lng_InstOpt1)"
  !insertmacro MUI_INSTALLOPTIONS_WRITE "inkscape.nsi.multiuser" "Field 1" "Text" "$(lng_InstType)"
  !insertmacro MUI_INSTALLOPTIONS_WRITE "inkscape.nsi.multiuser" "Field 2" "Text" "$(lng_AllUsers)"
  !insertmacro MUI_INSTALLOPTIONS_WRITE "inkscape.nsi.multiuser" "Field 3" "Text" "$(lng_CurUser)"
  !insertmacro MUI_INSTALLOPTIONS_WRITE "inkscape.nsi.multiuser" "Field 4" "Text" "$(lng_UseAs)"
  !insertmacro MUI_INSTALLOPTIONS_WRITE "inkscape.nsi.multiuser" "Field 5" "Text" "$(lng_Editor)"
  !insertmacro MUI_INSTALLOPTIONS_WRITE "inkscape.nsi.multiuser" "Field 6" "Text" "$(lng_Reader)"

  StrCmp $AskMultiUser "1" 0 +5
    !insertmacro MUI_INSTALLOPTIONS_DISPLAY "inkscape.nsi.multiuser"
    !insertmacro MUI_INSTALLOPTIONS_READ $MultiUser "inkscape.nsi.multiuser" "Field 2" "State"
    !insertmacro MUI_INSTALLOPTIONS_READ $Editor "inkscape.nsi.multiuser" "Field 5" "State"
    !insertmacro MUI_INSTALLOPTIONS_READ $Viewer "inkscape.nsi.multiuser" "Field 6" "State"

FunctionEnd

Function CustomPageSingleUser
  !insertmacro MUI_HEADER_TEXT "$(lng_InstOpt)" "$(lng_InstOpt1)"
  !insertmacro MUI_INSTALLOPTIONS_WRITE "inkscape.nsi.singleuser" "Field 1" "Text" "$(lng_UseAs)"
  !insertmacro MUI_INSTALLOPTIONS_WRITE "inkscape.nsi.singleuser" "Field 2" "Text" "$(lng_Editor)"
  !insertmacro MUI_INSTALLOPTIONS_WRITE "inkscape.nsi.singleuser" "Field 3" "Text" "$(lng_Reader)"

  StrCmp $askMultiUser "0" 0 +4 
    !insertmacro MUI_INSTALLOPTIONS_DISPLAY "inkscape.nsi.singleuser"
    !insertmacro MUI_INSTALLOPTIONS_READ $Editor "inkscape.nsi.singleuser" "Field 2" "State"
    !insertmacro MUI_INSTALLOPTIONS_READ $Viewer "inkscape.nsi.singleuser" "Field 3" "State"

FunctionEnd



Section Install

  StrCmp $MultiUser "1" "" SingleUser
    DetailPrint "admin mode, registry root will be HKLM"
    SetShellVarContext all
    Goto endSingleUser

  SingleUser:
    DetailPrint "single user mode, registry root will be HKCU"
    SetShellVarContext current
  endSingleUser:		

  ; check for writing registry
  ClearErrors
  WriteRegStr SHCTX "${PRODUCT_DIR_REGKEY}" "" "$INSTDIR\inkscape.exe"  
  IfErrors 0 +4
    DetailPrint "fatal: failed to write to ${PRODUCT_DIR_REGKEY}"
    DetailPrint "aborting installation"
	Abort
  WriteRegStr SHCTX "${PRODUCT_DIR_REGKEY}" "MultiUser" "$MultiUser"  
  WriteRegStr SHCTX "${PRODUCT_DIR_REGKEY}" "askMultiUser" "$askMultiUser"  
    

  SetOutPath "$INSTDIR"
  SetOverwrite ifnewer
  File /a /r "..\..\inkscape\*.*"
  WriteUninstaller "$INSTDIR\uninst.exe"

  ; start menu entries
  CreateDirectory "$SMPROGRAMS\Inkscape"
  CreateShortCut "$SMPROGRAMS\Inkscape\Inkscape.lnk" "$INSTDIR\inkscape.exe"
  CreateShortCut "$SMPROGRAMS\Inkscape\Uninstall Inkscape.lnk" "$INSTDIR\uninst.exe"
  CreateShortCut "$DESKTOP\Inkscape.lnk" "$INSTDIR\inkscape.exe"

  ; uninstall settings
  WriteRegExpandStr SHCTX "${PRODUCT_UNINST_KEY}" "UninstallString" '"$INSTDIR\uninst.exe"'
  WriteRegExpandStr SHCTX "${PRODUCT_UNINST_KEY}" "InstallLocation" "$INSTDIR"
  WriteRegStr SHCTX "${PRODUCT_UNINST_KEY}" "DisplayName" "${PRODUCT_NAME} ${PRODUCT_VERSION}"
  WriteRegStr SHCTX "${PRODUCT_UNINST_KEY}" "DisplayIcon" "$INSTDIR\Inkscape.exe,0"
  WriteRegStr SHCTX "${PRODUCT_UNINST_KEY}" "DisplayVersion" "${PRODUCT_VERSION}"
  WriteRegDWORD SHCTX "${PRODUCT_UNINST_KEY}" "NoModify" "1"
  WriteRegDWORD SHCTX "${PRODUCT_UNINST_KEY}" "NoRepair" "1"
  
  
  ; create file assoziations, test before if needed
  DetailPrint "creating file assoziations"
  ReadRegStr $0 HKCR ".svg" ""
  StrCmp $0 "" 0 +3
    WriteRegStr HKCR ".svg" "" "svgfile"
    WriteRegStr HKCR "svgfile" "" "Scalable Vector Graphics file"
  ReadRegStr $0 HKCR ".svgz" ""
  StrCmp $0 "" 0 +3
    WriteRegStr HKCR ".svgz" "" "svgfile"
    WriteRegStr HKCR "svgfile" "" "Scalable Vector Graphics file"
  
  StrCmp $Editor "1" "" NoEditor
    DetailPrint "creating default editor"
    ClearErrors
	ReadRegStr $0 HKCR ".svg" ""
    WriteRegStr HKCR "$0\shell\edit\command" "" '$INSTDIR\Inkscape.exe "%1"'
    ReadRegStr $0 HKCR ".svgz" ""
    WriteRegStr HKCR "$0\shell\edit\command" "" '$INSTDIR\Inkscape.exe "%1"'
	IfErrors 0 +2
	  DetailPrint "Uups! Problems creating default editor"
  NoEditor:

  StrCmp $Viewer "1" "" NoReader
    DetailPrint "creating default reader"
    ClearErrors
	ReadRegStr $0 HKCR ".svg" ""
	WriteRegStr HKCR "$0\shell\open\command" "" '$INSTDIR\Inkscape.exe "%1"'
    ReadRegStr $0 HKCR ".svgz" ""
	WriteRegStr HKCR "$0\shell\open\command" "" '$INSTDIR\Inkscape.exe "%1"'
	IfErrors 0 +2
	  DetailPrint "Uups! Problems creating default reader"
  NoReader:

  SetAutoClose false

SectionEnd


Function un.CustomPageUninstall
  !insertmacro MUI_HEADER_TEXT "$(lng_UInstOpt)" "$(lng_UInstOpt1)"
  !insertmacro MUI_INSTALLOPTIONS_WRITE "inkscape.nsi.uninstall" "Field 1" "Text" "$APPDATA\Inkscape\preferences.xml"
  !insertmacro MUI_INSTALLOPTIONS_WRITE "inkscape.nsi.uninstall" "Field 2" "Text" "$(lng_PurgePrefs)"

  !insertmacro MUI_INSTALLOPTIONS_DISPLAY "inkscape.nsi.uninstall"
  !insertmacro MUI_INSTALLOPTIONS_READ $MultiUser "inkscape.nsi.uninstall" "Field 2" "State"
  DetailPrint "keepfiles = $MultiUser" 
  ;MessageBox MB_OK "adminmode = $MultiUser MultiUserOS = $askMultiUser" 

FunctionEnd



Function un.onInit
  StrCpy $askMultiUser "1"
  StrCpy $MultiUser "1"
 
  ; Test if this was a multiuser installation
  ReadRegStr $0 HKLM "${PRODUCT_DIR_REGKEY}" ""
  StrCmp $0  "$INSTDIR\inkscape.exe" 0 +4  
    ReadRegStr $MultiUser HKLM "${PRODUCT_DIR_REGKEY}" "MultiUser"
    ReadRegStr $askMultiUser HKLM "${PRODUCT_DIR_REGKEY}" "askMultiUser"
	Goto +3
  ReadRegStr $MultiUser HKCU "${PRODUCT_DIR_REGKEY}" "MultiUser"
  ReadRegStr $askMultiUser HKCU "${PRODUCT_DIR_REGKEY}" "askMultiUser"
	  
  
 !insertmacro MUI_INSTALLOPTIONS_EXTRACT "inkscape.nsi.uninstall"

 ;check whether Multi user installation ?
 SetShellVarContext all
 StrCmp $MultiUser "0" 0 +2 
 SetShellVarContext current
 ;MessageBox MB_OK "adminmode = $MultiUser MultiUserOS = $askMultiUser" 
   
FunctionEnd

Section Uninstall

  ; remove personal settings
  StrCmp $MultiUser "0" 0 endPurge  ; multiuser assigned in dialog
    DetailPrint "purge personal settings"
    Delete "$APPDATA\Inkscape\preferences.xml"
  endPurge:

  Delete "$APPDATA\Inkscape\extension-errors.log"
  RMDir "$APPDATA\Inkscape"


  ; Remove file assoziations
  DetailPrint "removing file assoziations"
  ClearErrors
  ReadRegStr $0 HKCR ".svg" ""
  DetailPrint ".svg assoziated as $0"
  IfErrors endUninstSVGEdit  
    ReadRegStr $1 HKCR "$0\shell\edit\command" ""
	IfErrors 0 +2  
      DetailPrint "svg editor is $1"
    StrCmp $1 '$INSTDIR\Inkscape.exe "%1"' 0 +3
      DetailPrint "removing default .svg editor"
      DeleteRegKey HKCR "$0\shell\edit\command"
    DeleteRegKey /ifempty HKCR "$0\shell\edit"
    DeleteRegKey /ifempty HKCR "$0\shell"
    DeleteRegKey /ifempty HKCR "$0"
  endUninstSVGEdit:
  
  ClearErrors
  ReadRegStr $2 HKCR ".svgz" ""
  DetailPrint ".svgz assoziated as $2"
  IfErrors endUninstSVGZEdit  
    ReadRegStr $3 HKCR "$2\shell\edit\command" ""
    IfErrors 0 +2  
      DetailPrint "svgz editor is $1"
    StrCmp $3 '$INSTDIR\Inkscape.exe "%1"' 0 +3
      DetailPrint "removing default .svgz editor"
      DeleteRegKey HKCR "$2\shell\edit\command"
    DeleteRegKey /ifempty HKCR "$2\shell\edit"
    DeleteRegKey /ifempty HKCR "$2\shell"
    DeleteRegKey /ifempty HKCR "$2"
  endUninstSVGZEdit:
  
  ClearErrors
  ReadRegStr $0 HKCR ".svg" ""
  IfErrors endUninstSVGView
    ReadRegStr $1 HKCR "$0\shell\open\command" ""
    IfErrors 0 +2  
      DetailPrint "svg viewer is $1"
    StrCmp $1 '$INSTDIR\Inkscape.exe "%1"' 0 +3
      DetailPrint "removing default .svg viewer"
      DeleteRegKey HKCR "$0\shell\open\command"
    DeleteRegKey /ifempty HKCR "$0\shell\open"
    DeleteRegKey /ifempty HKCR "$0\shell"
    DeleteRegKey /ifempty HKCR "$0"
  endUninstSVGView:
  
  ClearErrors
  ReadRegStr $2 HKCR ".svgz" ""
  IfErrors endUninstSVGZView
    ReadRegStr $3 HKCR "$2\shell\open\command" ""
    IfErrors 0 +2  
      DetailPrint "svgz viewer is $1"
    StrCmp $3 '$INSTDIR\Inkscape.exe "%1"' 0 +3
      DetailPrint "removing default .svgz viewer"
      DeleteRegKey HKCR "$2\shell\open\command"
    DeleteRegKey /ifempty HKCR "$2\shell\open"
    DeleteRegKey /ifempty HKCR "$2\shell"
    DeleteRegKey /ifempty HKCR "$2"
  endUninstSVGZView:
  
  ReadRegStr $1 HKCR "$0" ""
  StrCmp $1 "" 0 +3
    DetailPrint "removing filetype .svg $0"
    DeleteRegKey HKCR ".svg"
  
  ReadRegStr $3 HKCR "$2" ""
  StrCmp $3 "" 0 +3
    DetailPrint "removing filetype .svgz $2"
    DeleteRegKey HKCR ".svgz"
  
    
  DetailPrint "removing product regkey"
  DeleteRegKey SHCTX "${PRODUCT_DIR_REGKEY}"
  DetailPrint "removing uninstall info"
  DeleteRegKey SHCTX "${PRODUCT_UNINST_KEY}"

  DetailPrint "removing shortcuts"
  Delete "$DESKTOP\Inkscape.lnk"
  Delete "$SMPROGRAMS\Inkscape\Uninstall Inkscape.lnk"
  Delete "$SMPROGRAMS\Inkscape\Inkscape.lnk"
  RMDir  "$SMPROGRAMS\Inkscape"

  DetailPrint "removing uninstall info"
  RMDir /r "$INSTDIR"

  SetAutoClose false

SectionEnd


