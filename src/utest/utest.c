/* Ultra-minimal unit testing framework */
/* This file is in the public domain */

#include "utest.h"
#ifdef __cplusplus
extern "C" {
#endif
#include <stdlib.h>
#include <stdio.h>
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
void
utest_start(char const *name)
{
	fprintf (stderr, "Testing %s...\n", name);
	utest__name = name;
	utest__tests = utest__passed = 0;
	utest__running = 0;
}

void utest__pass(void) {
	utest__passed++;
	utest__running = 0;
	fprintf(stderr, "OK\n");
}


/** \brief Write \a s0, \a s1, \a s2, and exit the current block of tests.
 *
 *  In the current implementation, any of \a s0, \a s1, \a s2 may be NULL, considered equivalent to
 *  empty string; but don't rely on that unless you also change this documentation string.  (No
 *  callers use this functionality at the time of writing.)
 *
 *  No newline needed in the \a s arguments.
 */
int
utest__fail(const char *s0, const char *s1, const char *s2)
{
	utest__running = 0;
	fprintf (stderr, "%s%s%s\n",
		 (s0 ? s0 : ""),
		 (s1 ? s1 : ""),
		 (s2 ? s2 : ""));
	longjmp (utest__jmp_buf, 0);
	return 0;
}


int
utest__test(char const *name)
{
	utest__tests++;
	if (utest__running) {
		utest__pass();
	}
	fprintf (stderr, "\t%s...", name);
	fflush (stderr);
	utest__running = 1;
	return 1;
}

/** \brief Ends a series of tests, reporting test statistics.
  *
  * Test statistics are printed to stderr, then the function returns
  * nonzero iff all the tests have passed, zero otherwise.
  */
int
utest_end(void)
{
	if (utest__running) {
		utest__pass();
	}
	if (utest__passed == utest__tests) {
		fprintf (stderr, "%s: OK (all %d passed)\n",
				 utest__name, utest__tests);
		return 1;
	} else {
		fprintf (stderr, "%s: FAILED (%d/%d tests passed)\n",
				 utest__name, utest__passed, utest__tests);
		return 0;
	}
}


/*
  Local Variables:
  mode:c
  c-file-style:"linux"
  fill-column:99
  End:
  vim: set filetype=c :
*/
