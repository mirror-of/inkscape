
#include <stdio.h>

#include "InkscapeScript.h"

#ifndef TRUE
#define TRUE  1
#endif
#ifndef FALSE
#define FALSE 0
#endif

static char *pythonCodeStr =
//"\n"
//"inkscape = _inkscape_py.getInkscape()\n"
"desktop  = inkscape.getDesktop()\n"
"document = desktop.getDocument()\n"
"document.hello()\n"
"\n";

int testPython()
{
    Inkscape::Extension::Script::InkscapeScript scriptEngine;
    printf("#####Executing Code#####\n");
    printf("%s\n", pythonCodeStr);
    printf("#####End#####\n");
    scriptEngine.interpretScript(pythonCodeStr, 
          Inkscape::Extension::Script::InkscapeScript::PYTHON);
    return TRUE;
}

static char *perlCodeStr =
//"\n"
//"$inkscape = inkscape_perlc::getInkscape();\n"
"print \"inkscape: '$inkscape'\n\"; \n"
"$desktop  = $inkscape->getDesktop();\n"
"$document = $desktop->getDocument();\n"
"$document->hello()\n"
//"reverse 'rekcaH lreP rehtonA tsuJ'\n"
"\n";

int testPerl()
{
    Inkscape::Extension::Script::InkscapeScript scriptEngine;
    printf("#####Executing Code#####\n");
    printf("%s\n", perlCodeStr);
    printf("#####End#####\n");
    scriptEngine.interpretScript(perlCodeStr,
         Inkscape::Extension::Script::InkscapeScript::PERL);
    return TRUE;
}



int doTest()
{
    if (!testPython())
        {
        printf("Failed Python test\n");
        return FALSE;
        }
    if (!testPerl())
        {
        printf("Failed Perl test\n");
        return FALSE;
        }
    return TRUE;
}



int main(int argc, char **argv)
{

    if (doTest())
        printf("Tests succeeded\n");
    else
        printf("Tests failed\n");
    return 0;
}






