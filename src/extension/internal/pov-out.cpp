/*
 * A quick hack to use the print output to write out a file.  This
 * then makes 'save as...' Postscript.
 *
 * Authors:
 *   Ted Gould <ted@gould.cx>
 *
 * Copyright (C) 2004 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include "pov-out.h"
#include <inkscape.h>
#include <sp-path.h>
#include <display/curve.h>
#include <libnr/n-art-bpath.h>
#include <extension/system.h>
#include <extension/db.h>

#include <stdio.h>
#include <vector>
#include <string.h>

namespace Inkscape {
namespace Extension {
namespace Internal {

bool
PovOutput::check (Inkscape::Extension::Extension * module)
{
	//if (NULL == Inkscape::Extension::db.get(SP_MODULE_KEY_PRINT_PS))
	//	return FALSE;

	return TRUE;
}

static void
findElementsByTagName(std::vector<SPRepr *> &results, SPRepr *node, const char *name)
{
    if (!name)
        results.push_back(node);
    else if (strcmp(sp_repr_name(node), name) == 0)
        results.push_back(node);

    for (SPRepr *child = node->children; child ; child = child->next)
        findElementsByTagName ( results, child, name );

}


/**
 * Saves the <paths> of an Inkscape SVG file as PovRay spline definitions
*/
void
PovOutput::save (Inkscape::Extension::Output *mod, SPDocument *doc, const gchar *uri)
{
    std::vector<SPRepr *>results;
    //findElementsByTagName(results, SP_ACTIVE_DOCUMENT->rroot, "path");
    findElementsByTagName(results, SP_ACTIVE_DOCUMENT->rroot, NULL);//Check all nodes
    if (results.size() == 0)
        return;
    FILE *f = fopen(uri, "w");
    if (!f)
        return;

    unsigned int indx;
    for (indx = 0; indx < results.size() ; indx++)
        {
        SPRepr *rpath = results[indx];
        gchar *id  = (gchar *)sp_repr_attr(rpath, "id");
        SPObject *reprobj = SP_ACTIVE_DOCUMENT->getObjectByRepr(rpath);
        if (!reprobj)
            continue;
        if (!SP_IS_SHAPE(reprobj))//Bulia's suggestion.  Allow all shapes
            {
            continue;
            }
        SPShape *shape = SP_SHAPE(reprobj);
        SPCurve *curve = shape->curve; 
        if (sp_curve_empty(curve))
            continue;


        int curveNr;

        //Count the NR_CURVETOs/LINETOs
        int segmentCount=0;
        NArtBpath *bp = curve->bpath;
        for (curveNr=0 ; curveNr<curve->length ; curveNr++, bp++)
            if (bp->code == NR_CURVETO || bp->code == NR_LINETO)
                segmentCount++;

        bp = curve->bpath;
        double lastx = 0.0;
        double lasty = 0.0;
        fprintf(f, "/*##############################################\n");
        fprintf(f, "### PRISM:  %s\n", id);
        fprintf(f, "##############################################*/\n");
        fprintf(f, "#declare %s = prism {\n", id);
        fprintf(f, "    linear_sweep\n");
        fprintf(f, "    bezier_spline\n");
        fprintf(f, "    0.0, //base\n");
        fprintf(f, "    1.0, //top\n");
        fprintf(f, "    %d, //nr points\n", segmentCount * 4);
        int segmentNr = 0;
        for (bp = curve->bpath, curveNr=0 ; curveNr<curve->length ; curveNr++, bp++)
            {
            switch (bp->code)
                {
                case NR_MOVETO:
                case NR_MOVETO_OPEN:
                    //fprintf(f, "moveto: %f %f\n", bp->x3, bp->y3);
                break;
                case NR_CURVETO:
                    fprintf(f, "/*%4d*/ <%f, %f>, <%f, %f>, <%f,%f>, <%f,%f>",
                        segmentNr++, lastx, lasty, bp->x1, bp->y1, 
                        bp->x2, bp->y2, bp->x3, bp->y3);
                    if (segmentNr < segmentCount)
                        fprintf(f, ",\n");
                    else
                        fprintf(f, "\n");
                break;
                case NR_LINETO:
                    fprintf(f, "/*%4d*/ <%f, %f>, <%f, %f>, <%f,%f>, <%f,%f>",
                        segmentNr++, lastx, lasty, lastx, lasty, 
                        bp->x3, bp->y3, bp->x3, bp->y3);
                    if (segmentNr < segmentCount)
                        fprintf(f, ",\n");
                    else
                        fprintf(f, "\n");
                    //fprintf(f, "lineto\n");
                break;
                case NR_END:
                    //fprintf(f, "end\n");
                break;
                }
            lastx = bp->x3;
            lasty = bp->y3;
            }
        fprintf(f, "}\n");
        fprintf(f, "/*##############################################\n");
        fprintf(f, "### end %s\n", id);
        fprintf(f, "##############################################*/\n\n\n\n");
        }

    //All done
    fclose(f);
}

/**
	\brief   A function allocate a copy of this function.

	This is the definition of postscript out.  This function just
	calls the extension system with the memory allocated XML that
	describes the data.
*/
void
PovOutput::init (void)
{
	Inkscape::Extension::build_from_mem(
		"<inkscape-extension>\n"
			"<name>PovRay Output</name>\n"
			"<id>org.inkscape.output.pov</id>\n"
			"<output>\n"
				"<extension>.pov</extension>\n"
				"<mimetype>text/x-povray-script</mimetype>\n"
				"<filetypename>PovRay (*.pov) (export splines)</filetypename>\n"
				"<filetypetooltip>PovRay Raytracer File</filetypetooltip>\n"
			"</output>\n"
		"</inkscape-extension>", new PovOutput());

	return;
}

};};}; /* namespace Inkscape, Extension, Internal */
