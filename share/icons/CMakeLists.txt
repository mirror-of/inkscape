add_subdirectory(application)
add_subdirectory(hicolor)
add_subdirectory(multicolor)
add_subdirectory(Tango)
file(GLOB _FILES "iconshowcase.py" "*.svg")
install(FILES ${_FILES}
    DESTINATION ${INKSCAPE_SHARE_INSTALL}/icons
    COMPONENT icons)

