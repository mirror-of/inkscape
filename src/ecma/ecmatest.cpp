/*#########################################################################
## $Id$
#########################################################################*/

/**
 *
 * Test driver for EcmaBinding
 *
 */

#include <stdio.h>
#include "EcmaBinding.h"

#ifndef TRUE
#define TRUE  1
#endif
#ifndef FALSE
#define FALSE 0
#endif


int testme(void)
{
    Inkscape::EcmaBinding *ecma = new Inkscape::EcmaBinding(NULL);


    delete ecma;

    return TRUE;
}




int main(int argc, char **argv)
{
    if (testme())
        {
	printf("Ecma tests succeeded\n");
	}
    else
        {
	printf("Ecma tests failed\n");
	}

    return 0;
}


/*#########################################################################
##  E N D    O F    F I L E
#########################################################################*/
