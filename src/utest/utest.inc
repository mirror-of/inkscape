/* Ultraminimal unit testing framework */
/* This file is in the public domain */

#ifdef __cplusplus
extern "C" {
#endif
#include <stdlib.h>
#include <stdio.h>
#include <setjmp.h>
#ifdef __cplusplus
};
#endif

jmp_buf utest__jmp_buf;
int utest__tests;
int utest__passed;
int utest__running;
const char *utest__name;

/** \brief Initializes the framework for running a series of tests.
  * \param name A descriptive label for this series of tests.
  */
void utest_start(const char *name) {
	fprintf(stderr, "Testing %s...\n", name);
	utest__name = name;
	utest__tests = utest__passed = 0;
	utest__running = 0;
}

void utest__pass(void) {
	utest__passed++;
	utest__running = 0;
	fprintf(stderr, "OK\n");
}

int utest__fail(const char *a, const char *b) {
	utest__running = 0;
	fprintf(stderr, "%s%s\n", a?a:"", b?b:"");
	longjmp(utest__jmp_buf, 0);
	return 0;
}

/** \breif Marks a C block constituting a single test.
  * \param name A descriptive name for this test.
  *
  * The block effectively becomes a try statement; if code within the
  * block triggers an assertion, control will resume at the end of the
  * block.
  */
#define UTEST_TEST(name) if (!setjmp(utest__jmp_buf)&&utest__test((name)))

/** \brief Terminates the current test if \a cond evaluates to nonzero.
  * \param cond The condition to test.
  */
#define UTEST_ASSERT(cond) UTEST_ASSERT_LABELED( #cond, (cond))

/** \brief Terminates the current tests if \a cond evaluates to nonzero,
  *        and prints a descriptive \a label instead of the condition
  *        that caused it to fail.
  * \param label The descriptive label to use.
  * \param cond The condition to test.
  */
#define UTEST_ASSERT_LABELED(label, cond) ((cond)||utest__fail("Assertion failed: ", (label) ))

int utest__test(const char *name) {
	utest__tests++;
	if (utest__running) {
		utest__pass();
	}
	fprintf(stderr, "\t%s...", name);
	fflush(stderr);
	utest__running = 1;
	return 1;
}

/** \brief Ends a series of tests, reporting test statistics.
  *
  * Test statistics are printed to stderr, then the function returns
  * nonzero iff all the tests have passed, zero otherwise.
  */
int utest_end(void) {
	if (utest__running) {
		utest__pass();
	}
	if ( utest__passed == utest__tests ) {
		fprintf(stderr, "%s: OK (all %d passed)\n",
		                utest__name, utest__tests);
		return 1;
	} else {
		fprintf(stderr, "%s: FAILED (%d/%d tests passed)\n",
		                utest__name, utest__passed, utest__tests);
		return 0;
	}
}

