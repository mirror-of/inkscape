#include <setjmp.h>

extern void utest_start(char const *name);
extern int utest_end(void);


/** \brief Marks a C block constituting a single test.
  * \param name A descriptive name for this test.
  *
  * The block effectively becomes a try statement; if code within the
  * block triggers an assertion, control will resume at the end of the
  * block.
  */
#define UTEST_TEST(_name)	\
	if (!setjmp (utest__jmp_buf)	\
	    && utest__test ((_name)))


/** \brief Terminates the current test if \a cond evaluates to nonzero.
  * \param cond The condition to test.
  */
#define UTEST_ASSERT(_cond) \
	UTEST_NAMED_ASSERT( #_cond, (_cond))

/** \brief Terminates the current tests if \a _cond evaluates to nonzero,
  *        and prints a descriptive \a _name instead of the condition
  *        that caused it to fail.
  * \param _name The descriptive label to use.
  * \param _cond The condition to test.
  */
#define UTEST_NAMED_ASSERT(_name, _cond)	\
	((_cond) || utest__fail ("Assertion `", (_name), "' failed"))


/* Internal things referenced by macros. */
extern jmp_buf utest__jmp_buf;
extern int utest__test(char const *name);
extern int utest__fail(const char *s0, const char *s1, const char *s2);
