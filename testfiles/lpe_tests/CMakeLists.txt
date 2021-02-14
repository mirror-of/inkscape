# SPDX-License-Identifier: GPL-2.0-or-later

#add your test here (do not put the .svg extension)
set(LPE_TESTS
    # -- Generic tests --
    bend
    ${LPE_TESTS_64bit}
)


foreach(lpe_test ${LPE_TESTS})
    set(testname "lpe_${lpe_test}")
    add_test(NAME ${testname}
             COMMAND bash ${CMAKE_CURRENT_SOURCE_DIR}/test.sh ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/inkscape ${CMAKE_CURRENT_SOURCE_DIR}/${lpe_test}
             WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/testfiles/lpe_tests)
    set_tests_properties(${testname} PROPERTIES ENVIRONMENT "${INKSCAPE_TEST_PROFILE_DIR_ENV}/${testname};${CMAKE_CTEST_ENV}")
endforeach()

