#include "../utest/utest.h"
#include <cmath>
using std::ceil;

/* mental disclaims all responsibility for this evil idea for testing
   static functions.  The main disadvantages are that we retain the
   #define's and `using' directives of the included file. */
#include "Path.cpp"

int main(int argc, char *argv[]) {
	utest_start("roundup_div");

	UTEST_TEST("roundup_div") {
		for(unsigned denom = -5u; denom != 6u; ++denom) {
			if(!denom) {
				continue;
			}
			for(unsigned numer = -5u; numer != 6u; ++numer) {
				unsigned q = roundup_div(numer, denom);
				UTEST_ASSERT( q == ceil( double(numer) / denom ) );
			}
		}
	}

	return ( utest_end()
		 ? EXIT_SUCCESS
		 : EXIT_FAILURE );
}
