
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

void utest_start(const char *name) {
	fprintf(stderr, "Testing %s...\n", name);
	utest__name = name;
	utest__tests = utest__passed = 0;
	utest__running = 0;
}

#define utest_assert_labeled(label, cond) ((cond)||utest__fail("Assertion failed: ", (label) ))
#define utest_assert(cond) utest_assert_labeled( #cond, (cond))

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

#define utest_test(name) (!setjmp(utest__jmp_buf)&&utest__test((name)))

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

