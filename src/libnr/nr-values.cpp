#define __NR_VALUES_C__

#include "nr-values.h"
#include <math.h>
#include "nr-macros.h"


/*
The following predefined objects are for reference
and comparison.
*/
NRMatrix NR_MATRIX_IDENTITY =
       {{1.0, 0.0, 0.0, 1.0, 0.0, 0.0}};
NRRect   NR_RECT_EMPTY =
       {NR_HUGE, NR_HUGE, -NR_HUGE, -NR_HUGE};
NRRectL  NR_RECT_L_EMPTY =
       {NR_HUGE_L, NR_HUGE_L, -NR_HUGE_L, -NR_HUGE_L};
NRRectL  NR_RECT_S_EMPTY =
       {NR_HUGE_S, NR_HUGE_S, -NR_HUGE_S, -NR_HUGE_S};

