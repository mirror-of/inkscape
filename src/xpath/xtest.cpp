#include <stdio.h>

#include "xpath.h"

int main(int argc, char **argv)
{

    XPath *xpath = XPath::create();

    xpath->parse("/svg/xyz/a123");

    delete xpath;

    return 0;
}

