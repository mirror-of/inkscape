; #######################################
; spanish.nsh
; spanish language strings for inkscape installer
; windows code page: 1252
; Authors:
; Adib Taraben theAdib@yahoo.com
; Lucas Vieites aka basurero
; 27 july 2006 new languages en_CA, en_GB, fi, hr, mn, ne, rw, sq
; 11 august 2006 new languages dz bg
; 24 october 2006 new languages en_US@piglatin, th
; 3rd December 2006 new languages eu km
; 14th December 2006 new lng_DeletePrefs, lng_DeletePrefsDesc, lng_WANT_UNINSTALL_BEFORE and lng_OK_CANCEL_DESC

!insertmacro MUI_LANGUAGE "Spanish"

; Product name
LangString lng_Caption   ${LANG_SPANISH} "${PRODUCT_NAME} -- Editor de gráficos vectoriales escalables (SVG) de código abierto"

; Button text "Next >" for the license page
LangString lng_LICENSE_BUTTON   ${LANG_SPANISH} "Siguiente >"

; Bottom text for the license page
LangString lng_LICENSE_BOTTOM_TEXT   ${LANG_SPANISH} "$(^Name) se publica bajo la Licencia Pública General GNU (GPL). Esta licencia se muestra aquí solamente como información. $_CLICK"

;has been installed by different user
LangString lng_DIFFERENT_USER ${LANG_SPANISH} "Inkscape ha sido instalad por el usuario $0.$\r$\nSi continúa podría no terminar correctamente.$\r$\nPor favor, inicie sesión como $0 y vuelva a intentarlo."

; want to uninstall before install
LangString lng_WANT_UNINSTALL_BEFORE ${LANG_SPANISH} "$R1 ya ha sido instalado. $\n¿Desea eliminar la versión anterior antes de instalar $(^Name) ?"

; press OK to continue press Cancel to abort
LangString lng_OK_CANCEL_DESC ${LANG_SPANISH} "$\n$\nPulse «Aceptar» para continuar o «Cancelar» para terminar."

;you have no admin rigths
LangString lng_NO_ADMIN ${LANG_SPANISH} "No dispone de privilegios de administrador.$\r$\nLa instalación de Inkscape para todos los usuarios podría no completarse con éxito.$\r$\nDesmarque la opción «para todos los usuarios»."

;win9x is not supported
LangString lng_NOT_SUPPORTED ${LANG_SPANISH} "Inkscape no se ejecuta correctamente bajo Windows 95/98/ME!$\r$\nObtenga información más detallada en el sitio web oficial."

; Full install type
LangString lng_Full $(LANG_SPANISH) "Completa"

; Optimal install type
LangString lng_Optimal $(LANG_SPANISH) "Óptima"

; Minimal install type
LangString lng_Minimal $(LANG_SPANISH) "Mínima"

; Core install section
LangString lng_Core $(LANG_SPANISH) "${PRODUCT_NAME} Editor SVG (requerido)"

; Core install section description
LangString lng_CoreDesc $(LANG_SPANISH) "Archivos ${PRODUCT_NAME} básicos y dlls"

; GTK+ install section
LangString lng_GTKFiles $(LANG_SPANISH) "GTK+ Runtime Environment (requerido)"

; GTK+ install section description
LangString lng_GTKFilesDesc $(LANG_SPANISH) "Un conjunto de herramientas GUI, utilizado por ${PRODUCT_NAME}"

; shortcuts install section
LangString lng_Shortcuts $(LANG_SPANISH) "Acceso directo"

; shortcuts install section description
LangString lng_ShortcutsDesc $(LANG_SPANISH) "Accesos directos para iniciar ${PRODUCT_NAME}"

; All user install section
LangString lng_Alluser $(LANG_SPANISH) "para todos los usuarios"

; All user install section description
LangString lng_AlluserDesc $(LANG_SPANISH) "Instalar esta aplicación para todos los que utilizan este equipo (todos los usuarios)"

; Desktop section
LangString lng_Desktop $(LANG_SPANISH) "Escritorio"

; Desktop section description
LangString lng_DesktopDesc $(LANG_SPANISH) "Crear un acceso directo a ${PRODUCT_NAME} en el escritorio"

; Start Menu  section
LangString lng_Startmenu $(LANG_SPANISH) "Menú Inicio"

; Start Menu section description
LangString lng_StartmenuDesc $(LANG_SPANISH) "Crear una entrada en el menú Inicio para ${PRODUCT_NAME}"

; Quick launch section
LangString lng_Quicklaunch $(LANG_SPANISH) "Acceso rápido"

; Quick launch section description
LangString lng_QuicklaunchDesc $(LANG_SPANISH) "Crear un acceso directo a ${PRODUCT_NAME} en la barra de acceso rápido"

; File type association for editing
LangString lng_SVGWriter ${LANG_SPANISH} "Abrir archivos SVG con ${PRODUCT_NAME}"

; File type association for editing description
LangString lng_SVGWriterDesc ${LANG_SPANISH} "Seleccionar ${PRODUCT_NAME} como editor predeterminado para archivos"

; Context Menu
LangString lng_ContextMenu ${LANG_SPANISH} "Menú contextual"

; Context Menu description
LangString lng_ContextMenuDesc ${LANG_SPANISH} "Añadir ${PRODUCT_NAME} al menú contextual para archivos SVG"

; remove personal preferences
LangString lng_DeletePrefs ${LANG_SPANISH} "Eliminar las preferencias personales"

; remove personal preferences description
LangString lng_DeletePrefsDesc ${LANG_SPANISH} "Eliminar las preferencias personales residuales de instalaciones previas."

; Additional files section
LangString lng_Addfiles $(LANG_SPANISH) "Archivos adicionales"

; Additional files section description
LangString lng_AddfilesDesc $(LANG_SPANISH) "Archivos adicionales"

; Examples section
LangString lng_Examples $(LANG_SPANISH) "Ejemplos"

; Examples section description
LangString lng_ExamplesDesc $(LANG_SPANISH) "Ejemplos de uso de ${PRODUCT_NAME}"

; Tutorials section
LangString lng_Tutorials $(LANG_SPANISH) "Tutoriales"

; Tutorials section description
LangString lng_TutorialsDesc $(LANG_SPANISH) "Tutoriales del uso de ${PRODUCT_NAME}"


; Languages section
LangString lng_Languages $(LANG_SPANISH) "Traducciones"

; Languages section dscription
LangString lng_LanguagesDesc $(LANG_SPANISH) "Instalar varias traducciones para ${PRODUCT_NAME}"

LangString lng_am $(LANG_SPANISH) "am  Amhárico"
LangString lng_az $(LANG_SPANISH) "az  Azerbaiyaní"
LangString lng_be $(LANG_SPANISH) "be  Bieloruso"
LangString lng_bg $(LANG_SPANISH) "bg  Búlgaro"
LangString lng_ca $(LANG_SPANISH) "ca  Catalán"
LangString lng_cs $(LANG_SPANISH) "cs  Checo"
LangString lng_da $(LANG_SPANISH) "da  Danés"
LangString lng_de $(LANG_SPANISH) "de  Alemám"
LangString lng_dz $(LANG_SPANISH) "dz  Dzongkha"
LangString lng_el $(LANG_SPANISH) "el  Griego"
LangString lng_en $(LANG_SPANISH) "en  Inglés"
LangString lng_en_CA $(LANG_SPANISH) "en_CA Inglés canadiense"
LangString lng_en_GB $(LANG_SPANISH) "en_GB Inglés británico"
LangString lng_en_US@piglatin $(LANG_SPANISH) "en_US@piglatin Pig Latin"
LangString lng_es $(LANG_SPANISH) "es  Español"
LangString lng_es_MX $(LANG_SPANISH) "es_MX  Español Mexicano"
LangString lng_et $(LANG_SPANISH) "et  Estonio"
LangString lng_eu $(LANG_SPANISH) "eu  Euskera/Vasco"
LangString lng_fi $(LANG_SPANISH) "fi  Finlandés"
LangString lng_fr $(LANG_SPANISH) "fr  Francés"
LangString lng_ga $(LANG_SPANISH) "ga  Irlandés"
LangString lng_gl $(LANG_SPANISH) "gl  Gallego"
LangString lng_hr $(LANG_SPANISH) "hr  Croata"
LangString lng_hu $(LANG_SPANISH) "hu  Húngaro"
LangString lng_it $(LANG_SPANISH) "it  Italiano"
LangString lng_ja $(LANG_SPANISH) "ja  Japonés"
LangString lng_km $(LANG_SPANISH) "km  Khmer"
LangString lng_ko $(LANG_SPANISH) "ko  Coreano"
LangString lng_lt $(LANG_SPANISH) "lt  Lituano"
LangString lng_mk $(LANG_SPANISH) "mk  Macedonio"
LangString lng_mn $(LANG_SPANISH) "mn  Mongol"
LangString lng_ne $(LANG_SPANISH) "ne  Nepalí"
LangString lng_nb $(LANG_SPANISH) "nb  Noruego Bokmål"
LangString lng_nl $(LANG_SPANISH) "nl  Neerlandés"
LangString lng_nn $(LANG_SPANISH) "nn  Noruego Nynorsk"
LangString lng_pa $(LANG_SPANISH) "pa  Punjabi"
LangString lng_pl $(LANG_SPANISH) "po  Polaco"
LangString lng_pt $(LANG_SPANISH) "pt  Portugués"
LangString lng_pt_BR $(LANG_SPANISH) "pt_BR Portugués brasileño"
LangString lng_ru $(LANG_SPANISH) "ru  Ruso"
LangString lng_rw $(LANG_SPANISH) "rw  Kinyarwanda"
LangString lng_sk $(LANG_SPANISH) "sk  Eslovaco"
LangString lng_sl $(LANG_SPANISH) "sl  Esloveno"
LangString lng_sq $(LANG_SPANISH) "sq  Albano"
LangString lng_sr $(LANG_SPANISH) "sr  Serbio"
LangString lng_sr@Latn $(LANG_SPANISH) "sr@Latn  Serbio en caracteres latinos"
LangString lng_sv $(LANG_SPANISH) "sv  Sueco"
LangString lng_th $(LANG_SPANISH) "th  Tailandés"
LangString lng_tr $(LANG_SPANISH) "tr  Turco"
LangString lng_uk $(LANG_SPANISH) "uk  Ucraniano"
LangString lng_vi $(LANG_SPANISH) "vi  Vietnamita"
LangString lng_zh_CN $(LANG_SPANISH) "zh_CH  Chino simplificado"
LangString lng_zh_TW $(LANG_SPANISH) "zh_TW  Chino (Taiwan)"




; uninstallation options
LangString lng_UInstOpt   ${LANG_SPANISH} "Opciones de desinstalación"

; uninstallation options subtitle
LangString lng_UInstOpt1  ${LANG_SPANISH} "Elija sus opciones adicionales"

; Ask to purge the personal preferences
LangString lng_PurgePrefs ${LANG_SPANISH} "Guardar las preferencias personales"

 	  	 
