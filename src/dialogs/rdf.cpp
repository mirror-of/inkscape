/**
 * \brief  RDF manipulation functions
 *
 * Authors:
 *   Kees Cook <kees@outflux.net>
 *
 * Copyright (C) Kees Cook 2004
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <config.h>

#include <string.h>
#include <glib.h>
#include "rdf.h"

/*
 
<rdf:RDF xmlns="http://web.resource.org/cc/"
    xmlns:dc="http://purl.org/dc/elements/1.1/"
    xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#">
<Work rdf:about="">
   <dc:title>title of work</dc:title>
   <dc:date>year</dc:date>
   <dc:description>description of work</dc:description>
   <dc:creator><Agent>
      <dc:title>creator</dc:title>
   </Agent></dc:creator>
   <dc:rights><Agent>
      <dc:title>holder</dc:title>
   </Agent></dc:rights>
   <dc:type rdf:resource="http://purl.org/dc/dcmitype/StillImage" />
   <dc:source rdf:resource="source"/>
   <license rdf:resource="http://creativecommons.org/licenses/by/2.0/" 
/>
</Work>

  <rdf:RDF xmlns="http://web.resource.org/cc/"
      xmlns:dc="http://purl.org/dc/elements/1.1/"
      xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#">
  <Work rdf:about="">
     <dc:title>SVG Road Signs</dc:title>
     <dc:rights><Agent>
        <dc:title>John Cliff</dc:title>
     </Agent></dc:rights>
     <dc:type rdf:resource="http://purl.org/dc/dcmitype/StillImage" />
     <license rdf:resource="http://web.resource.org/cc/PublicDomain" />
  </Work>
  
  <License rdf:about="http://web.resource.org/cc/PublicDomain">
     <permits rdf:resource="http://web.resource.org/cc/Reproduction" />
     <permits rdf:resource="http://web.resource.org/cc/Distribution" />
     <permits rdf:resource="http://web.resource.org/cc/DerivativeWorks" />
  </License>
  
</rdf:RDF>
*/


struct rdf_license_t rdf_licenses [] = {
	{ "Creative Commons Attribution", 
          "http://creativecommons.org/licenses/by/2.0/",
	  "\
   <permits rdf:resource=\"http://web.resource.org/cc/Reproduction\" />\
   <permits rdf:resource=\"http://web.resource.org/cc/Distribution\" />\
   <requires rdf:resource=\"http://web.resource.org/cc/Notice\" />\
   <requires rdf:resource=\"http://web.resource.org/cc/Attribution\" />\
   <permits rdf:resource=\"http://web.resource.org/cc/DerivativeWorks\" />\
",
	},

	{ "Creative Commons Attribution-ShareAlike", 
          "http://creativecommons.org/licenses/by/2.0/",
	  "\
   <permits rdf:resource=\"http://web.resource.org/cc/Reproduction\" />\
   <permits rdf:resource=\"http://web.resource.org/cc/Distribution\" />\
   <requires rdf:resource=\"http://web.resource.org/cc/Notice\" />\
   <requires rdf:resource=\"http://web.resource.org/cc/Attribution\" />\
   <permits rdf:resource=\"http://web.resource.org/cc/DerivativeWorks\" />\
   <requires rdf:resource=\"http://web.resource.org/cc/ShareAlike\" />\
",
	},

	{ "Creative Commons Attribution-NoDerivs", 
          "http://creativecommons.org/licenses/by/2.0/",
	  "\
   <permits rdf:resource=\"http://web.resource.org/cc/Reproduction\" />\
   <permits rdf:resource=\"http://web.resource.org/cc/Distribution\" />\
   <requires rdf:resource=\"http://web.resource.org/cc/Notice\" />\
   <requires rdf:resource=\"http://web.resource.org/cc/Attribution\" />\
",
	},

	{ "Creative Commons Attribution-NonCommercial", 
          "http://creativecommons.org/licenses/by/2.0/",
	  "\
   <permits rdf:resource=\"http://web.resource.org/cc/Reproduction\" />\
   <permits rdf:resource=\"http://web.resource.org/cc/Distribution\" />\
   <requires rdf:resource=\"http://web.resource.org/cc/Notice\" />\
   <requires rdf:resource=\"http://web.resource.org/cc/Attribution\" />\
   <prohibits rdf:resource=\"http://web.resource.org/cc/CommercialUse\" />\
   <permits rdf:resource=\"http://web.resource.org/cc/DerivativeWorks\" />\
",
	},

	{ "Creative Commons Attribution-NonCommercial-ShareAlike", 
          "http://creativecommons.org/licenses/by/2.0/",
	  "\
   <permits rdf:resource=\"http://web.resource.org/cc/Reproduction\" />\
   <permits rdf:resource=\"http://web.resource.org/cc/Distribution\" />\
   <requires rdf:resource=\"http://web.resource.org/cc/Notice\" />\
   <requires rdf:resource=\"http://web.resource.org/cc/Attribution\" />\
   <prohibits rdf:resource=\"http://web.resource.org/cc/CommercialUse\" />\
   <permits rdf:resource=\"http://web.resource.org/cc/DerivativeWorks\" />\
   <requires rdf:resource=\"http://web.resource.org/cc/ShareAlike\" />\
",
	},

	{ "Creative Commons Attribution-NonCommercial-NoDerivs", 
          "http://creativecommons.org/licenses/by/2.0/",
	  "\
   <permits rdf:resource=\"http://web.resource.org/cc/Reproduction\" />\
   <permits rdf:resource=\"http://web.resource.org/cc/Distribution\" />\
   <requires rdf:resource=\"http://web.resource.org/cc/Notice\" />\
   <requires rdf:resource=\"http://web.resource.org/cc/Attribution\" />\
   <prohibits rdf:resource=\"http://web.resource.org/cc/CommercialUse\" />\
",
	},

	{ "Public Domain",
	  "http://web.resource.org/cc/PublicDomain",
	  "\
     <permits rdf:resource=\"http://web.resource.org/cc/Reproduction\" />\
     <permits rdf:resource=\"http://web.resource.org/cc/Distribution\" />\
     <permits rdf:resource=\"http://web.resource.org/cc/DerivativeWorks\" />\
",
	},

	{ NULL, NULL, NULL }
};

/*
 * Takes the inkscape rdf struct and spits out a static RDF, which is only
 * useful for testing.  We must merge the rdf struct into the XML DOM for
 * changes to be saved.
 */
gchar *
rdf_string(struct rdf_t * rdf)
{
    gulong overall=0;
    gchar *string=NULL;

    gchar *rdf_head="\
<rdf:RDF xmlns=\"http://web.resource.org/cc/\"\
    xmlns:dc=\"http://purl.org/dc/elements/1.1/\"\
    xmlns:rdf=\"http://www.w3.org/1999/02/22-rdf-syntax-ns#\">\
";
    gchar *work_head="\
<Work rdf:about=\"\">\
   <dc:type rdf:resource=\"http://purl.org/dc/dcmitype/StillImage\" />\
";
    gchar *work_title=NULL;
    gchar *work_date=NULL;
    gchar *work_description=NULL;
    gchar *work_creator=NULL;
    gchar *work_owner=NULL;
    gchar *work_source=NULL;
    gchar *work_license=NULL;
    gchar *license_head=NULL;
    gchar *license=NULL;
    gchar *license_end="</License>\n";
    gchar *work_end="</Work>\n";
    gchar *rdf_end="</rdf:RDF>\n";

    if (rdf && rdf->work_title && rdf->work_title[0]) {
        work_title=g_markup_printf_escaped("   <dc:title>%s</dc:title>\n",
            rdf->work_title);
	overall+=strlen(work_title);
    }
    if (rdf && rdf->work_date && rdf->work_date[0]) {
        work_date=g_markup_printf_escaped("   <dc:date>%s</dc:date>\n",
            rdf->work_date);
	overall+=strlen(work_date);
    }
    if (rdf && rdf->work_description && rdf->work_description[0]) {
        work_description=g_markup_printf_escaped("   <dc:description>%s</dc:description>\n",
            rdf->work_description);
	overall+=strlen(work_description);
    }
    if (rdf && rdf->work_creator && rdf->work_creator[0]) {
        work_creator=g_markup_printf_escaped("   <dc:creator><Agent>\
      <dc:title>%s</dc:title>\
   </Agent></dc:creator>\n",
            rdf->work_creator);
	overall+=strlen(work_creator);
    }
    if (rdf && rdf->work_owner && rdf->work_owner[0]) {
        work_owner=g_markup_printf_escaped("   <dc:rights><Agent>\
      <dc:title>%s</dc:title>\
   </Agent></dc:rights>\n",
            rdf->work_owner);
	overall+=strlen(work_owner);
    }
    if (rdf && rdf->work_source && rdf->work_source[0]) {
        work_source=g_markup_printf_escaped("   <dc:source rdf:resource=\"%s\" />\n",
            rdf->work_source);
	overall+=strlen(work_source);
    }
    if (rdf && rdf->license && rdf->license->work_rdf && rdf->license->work_rdf[0]) {
        work_license=g_markup_printf_escaped("   <license rdf:resource=\"%s\" />\n",
            rdf->license->work_rdf);
	overall+=strlen(work_license);

	license_head=g_markup_printf_escaped("<License rdf:about=\"%s\">\n",
			rdf->license->work_rdf);
	overall+=strlen(license_head);
	overall+=strlen(rdf->license->license_rdf);
	overall+=strlen(license_end);
    }

    overall+=strlen(rdf_head)+strlen(rdf_end);
    overall+=strlen(work_head)+strlen(work_end);

    overall++; // NULL term

    if (!(string=(gchar*)g_malloc(overall))) {
        return NULL;
    }

    string[0]='\0';
    strcat(string,rdf_head);
    strcat(string,work_head);

    if (work_title)       strcat(string,work_title);
    if (work_date)        strcat(string,work_date);
    if (work_description) strcat(string,work_description);
    if (work_creator)     strcat(string,work_creator);
    if (work_owner)       strcat(string,work_owner);
    if (work_source)      strcat(string,work_source);
    if (work_license)     strcat(string,work_license);

    strcat(string,work_end);
    if (license_head) {
        strcat(string,license_head);
	strcat(string,rdf->license->license_rdf);
	strcat(string,license_end);
    }
    strcat(string,rdf_end);

    return string;
}

