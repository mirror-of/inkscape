# -----------------------------------------------------------------------------
#
# -----------------------------------------------------------------------------
add_subdirectory(${GMOCK_DIR} ${CMAKE_BINARY_DIR}/gmock)

include_directories(SYSTEM ${GMOCK_DIR}/gtest/include
                           ${GMOCK_DIR}/include
                           ${CMAKE_SOURCE_DIR}/gtest/gtest/include)

set_source_files_properties(
		${CMAKE_BINARY_DIR}/src/inkscape-version.cpp
		PROPERTIES GENERATED TRUE)

include_directories(${CMAKE_BINARY_DIR}/src)

add_executable(unittest
	unittest.cpp
	doc-per-case-test.cpp
	src/attributes-test.cpp
	src/color-profile-test.cpp
	src/dir-util-test.cpp
)

target_link_libraries(unittest
	gmock_main
	inkscape_base)

add_dependencies(unittest inkscape_version)


add_test(BaseTest ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/unittest)
add_dependencies(check unittest)
