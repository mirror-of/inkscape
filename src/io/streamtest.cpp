

#include <stdio.h>

#include "InkscapeStream.h"
#include "GzipStream.h"
#include "StringStream.h"
#include "UriStream.h"
#include "XsltStream.h"

bool testUriStream()
{
   Inkscape::URI inUri("streamtest");
   Inkscape::IO::UriInputStream  ins(inUri);
   Inkscape::URI outUri("streamtest.copy");
   Inkscape::IO::UriOutputStream outs(outUri);
   
   pipeStream(ins, outs);
   
   ins.close();
   outs.close();

   return true;
}

bool testWriter()
{
   Inkscape::IO::StdOutputStream outs;
   Inkscape::IO::OutputStreamWriter writer(outs);
   
   writer << "Hello, world!  " << 123.45 << " times\n";

   writer.printf("There are %f quick brown foxes in %d states\n", 123.45, 88);
   return true;
}

bool testXslt()
{
   printf("######### XSLT Sheet ############\n");
   Inkscape::URI xsltSheetUri("doc2html.xsl");
   Inkscape::IO::UriInputStream  xsltSheetIns(xsltSheetUri);
   Inkscape::IO::XsltStyleSheet stylesheet(xsltSheetIns);
   xsltSheetIns.close();
   
   Inkscape::URI sourceUri("crystalegg.xml");
   Inkscape::IO::UriInputStream  xmlIns(sourceUri);

   printf("######### XSLT Input ############\n");
   Inkscape::URI destUri("test.html");
   Inkscape::IO::UriOutputStream xmlOuts(destUri);
   
   Inkscape::IO::XsltInputStream  xsltIns(xmlIns, stylesheet);
   pipeStream(xsltIns, xmlOuts);
   xsltIns.close();
   xmlOuts.close();


   printf("######### XSLT Output ############\n");

   Inkscape::IO::UriInputStream  xmlIns2(sourceUri);

   Inkscape::URI destUri2("test2.html");
   Inkscape::IO::UriOutputStream xmlOuts2(destUri2);
   
   Inkscape::IO::XsltOutputStream  xsltOuts(xmlOuts2, stylesheet);
   pipeStream(xmlIns2, xsltOuts);
   xmlIns2.close();
   xsltOuts.close();

   return true;
}

bool testGzip()
{

   printf("######### Gzip Output ############\n");
   Inkscape::URI gzUri("test.gz");
   Inkscape::URI sourceUri("crystalegg.xml");
   Inkscape::IO::UriInputStream  sourceIns(sourceUri);
   Inkscape::IO::UriOutputStream  gzOuts(gzUri);

   Inkscape::IO::GzipOutputStream gzipOuts(gzOuts);
   pipeStream(sourceIns, gzipOuts);
   sourceIns.close();
   gzipOuts.close();

   printf("######### Gzip Input ############\n");

   Inkscape::IO::UriInputStream  gzIns(gzUri);
   Inkscape::URI destUri("crystalegg2.xml");
   Inkscape::IO::UriOutputStream destOuts(destUri);

   Inkscape::IO::GzipInputStream gzipIns(gzIns);
   pipeStream(gzipIns, destOuts);
   gzipIns.close();
   destOuts.close();

   return true;
}

bool doTest()
{
   /*
   if (!testUriStream())
       {
       return false;
       }
   if (!testWriter())
       {
       return false;
       }
   if (!testXslt())
       {
       return false;
       }
   */
   if (!testGzip())
       {
       return false;
       }
   return true;
}


int main(int argc, char **argv)
{
    if (!doTest())
        {
        printf("#### Test failed\n");
        }
    else
        {
        printf("##### Test succeeded\n");
        }
    return 0;
}
