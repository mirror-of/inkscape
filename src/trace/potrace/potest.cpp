#include <stdio.h>

#include "inkscape-potrace.h"


int main(int argc, char **argv)
{
    void *res;
    res = Inkscape::Potrace::Potrace::convertImageToPath();
    return 0;
}
