#define __INK_STOCK_ITEMS__

/*
 * Stock-items
 *
 * Stock Item management code
 *
 * Authors:
 *  John Cliff <simarilius@yahoo.com>
 *
 * Copyright 2004 John Cliff
 *
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#define noSP_SS_VERBOSE

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif
#include "path-prefix.h"

#include <string.h>
#include <glib.h>

#include <libnr/nr-values.h>
#include <libnr/nr-matrix.h>
#include <xml/repr.h>
#include "sp-defs.h"
#include "document-private.h"
#include "sp-gradient.h"
#include "sp-pattern.h"
#include "sp-marker.h"
#include "desktop-handles.h"
#include "inkscape.h"

#include <gtk/gtk.h>



static SPObject *sp_gradient_load_from_svg(gchar const *name, SPDocument *current_doc);
static SPObject *sp_marker_load_from_svg(gchar const *name, SPDocument *current_doc);
static SPObject *sp_gradient_load_from_svg(gchar const *name, SPDocument *current_doc);



static SPObject * sp_marker_load_from_svg(gchar const *name, SPDocument *current_doc)
{
    static SPDocument *doc = NULL;
    static unsigned int edoc = FALSE;
    if (!current_doc) {
        return NULL;
    }
    /* Try to load from document */
    if (!edoc && !doc) {
        char *markers = g_build_filename(INKSCAPE_MARKERSDIR, "/markers.svg", NULL);
        if (g_file_test(markers, G_FILE_TEST_IS_REGULAR)) {
            doc = sp_document_new(markers, FALSE, FALSE);
        }
        if ( !doc && g_file_test( markers,
                                  G_FILE_TEST_IS_REGULAR) )
        {
            doc = sp_document_new( markers,
                                   FALSE, FALSE );
        }
        g_free(markers);
        if (doc) {
            sp_document_ensure_up_to_date(doc);
        } else {
            edoc = TRUE;
        }
    }
    if (!edoc && doc) {
        /* Get the marker we want */
        SPObject *object = doc->getObjectById(name);
        if (object && SP_IS_MARKER(object)) {
            SPDefs *defs= (SPDefs *) SP_DOCUMENT_DEFS(current_doc);
            SPRepr *mark_repr = sp_repr_duplicate(SP_OBJECT_REPR(object));
            sp_repr_add_child (SP_OBJECT_REPR(defs), mark_repr, NULL);
            SPObject *cloned_item = current_doc->getObjectByRepr(mark_repr);
            sp_repr_unref(mark_repr);
            return cloned_item;
        }
    }
    return NULL;
}


static SPObject *
sp_pattern_load_from_svg(gchar const *name, SPDocument *current_doc)
{
    static SPDocument *doc = NULL;
    static unsigned int edoc = FALSE;
    if (!current_doc) {
        return NULL;
    }
    /* Try to load from document */
    if (!edoc && !doc) {
        char *patterns = g_build_filename(INKSCAPE_PATTERNSDIR, "/patterns.svg", NULL);
        if (g_file_test(patterns, G_FILE_TEST_IS_REGULAR)) {
            doc = sp_document_new(patterns, FALSE, FALSE);
        }
        if ( !doc && g_file_test( patterns,
                                  G_FILE_TEST_IS_REGULAR) )
        {
            doc = sp_document_new( patterns,
                                   FALSE, FALSE );
        }
        g_free(patterns);
        if (doc) {
            sp_document_ensure_up_to_date(doc);
        } else {
            edoc = TRUE;
        }
    }
    if (!edoc && doc) {
        /* Get the pattern we want */
        SPObject *object = doc->getObjectById(name);
        if (object && SP_IS_PATTERN(object)) {
            SPDefs *defs= (SPDefs *) SP_DOCUMENT_DEFS(current_doc);
            SPRepr *pat_repr = sp_repr_duplicate(SP_OBJECT_REPR(object));
            sp_repr_add_child (SP_OBJECT_REPR(defs), pat_repr, NULL);
            sp_repr_unref(pat_repr);
            return object;
        }
    }
    return NULL;
}


static SPObject *
sp_gradient_load_from_svg(gchar const *name, SPDocument *current_doc)
{
    static SPDocument *doc = NULL;
    static unsigned int edoc = FALSE;
    if (!current_doc) {
        return NULL;
    }
    /* Try to load from document */
    if (!edoc && !doc) {
        char *gradients = g_build_filename(INKSCAPE_GRADIENTSDIR, "/gradients.svg", NULL);
        if (g_file_test(gradients, G_FILE_TEST_IS_REGULAR)) {
            doc = sp_document_new(gradients, FALSE, FALSE);
        }
        if ( !doc && g_file_test( gradients,
                                  G_FILE_TEST_IS_REGULAR) )
        {
            doc = sp_document_new( gradients,
                                   FALSE, FALSE );
        }
        g_free(gradients);
        if (doc) {
            sp_document_ensure_up_to_date(doc);
        } else {
            edoc = TRUE;
        }
    }
    if (!edoc && doc) {
        /* Get the gradient we want */
        SPObject *object = doc->getObjectById(name);
        if (object && SP_IS_GRADIENT(object)) {
            SPDefs *defs= (SPDefs *) SP_DOCUMENT_DEFS(current_doc);
            SPRepr *pat_repr = sp_repr_duplicate(SP_OBJECT_REPR(object));
            sp_repr_add_child (SP_OBJECT_REPR(defs), pat_repr, NULL);
            sp_repr_unref(pat_repr);
            return object;
        }
    }
    return NULL;
}

// get_stock_item returns a pointer to an instance of the desired stock object in the current doc
// if necessary it will import the object. Copes with name clashes through use of the inkscape:stockid property
// This should be set to be the same as the id in the libary file.

SPObject*
get_stock_item(gchar const *urn)
{
    g_assert(urn!=NULL);
    /* check its an inkscape URN */
    if (!strncmp (urn, "urn:inkscape:", 13)) {
    const gchar *e = urn + 13;
    int a =0;
    gchar *name = g_strdup(e);
    while (*name != ':' && *name != '\0'){
        name++;
        a++;
    }
    if (*name ==':') name++;
    gchar *base = g_strndup(e,a);


    SPDesktop *desktop = inkscape_active_desktop();
    SPDocument *doc = SP_DT_DOCUMENT(desktop);
    SPDefs *defs= (SPDefs *) SP_DOCUMENT_DEFS(doc);

    SPObject *object = NULL;
    if (!strcmp(base,"marker"))  {
                                   for (SPObject *child = sp_object_first_child(SP_OBJECT(defs)) ;
                                        child != NULL;
                                        child = SP_OBJECT_NEXT(child) )
                                           {

                                               if (sp_repr_attr(SP_OBJECT_REPR(child),"inkscape:stockid") && !strcmp(name, sp_repr_attr(SP_OBJECT_REPR(child),"inkscape:stockid")) &&
                                                    SP_IS_MARKER(child))
                                                    {
                                                        object = child;
                                                    }
                                           }

      }
    else if (!strcmp(base,"pattern"))  {
                                   for (SPObject *child = sp_object_first_child(SP_OBJECT(defs)) ;
                                        child != NULL;
                                        child = SP_OBJECT_NEXT(child) )
                                           {
                                               if (sp_repr_attr(SP_OBJECT_REPR(child),"inkscape:stockid") && !strcmp(name, sp_repr_attr(SP_OBJECT_REPR(child),"inkscape:stockid")) &&
                                                    SP_IS_PATTERN(child))
                                                    {
                                                        object = child;
                                                    }
                                           }

     }
     else   if (!strcmp(base,"gradient"))  {
                                   for (SPObject *child = sp_object_first_child(SP_OBJECT(defs)) ;
                                        child != NULL;
                                        child = SP_OBJECT_NEXT(child) )
                                           {
                                               if (sp_repr_attr(SP_OBJECT_REPR(child),"inkscape:stockid") && !strcmp(name, sp_repr_attr(SP_OBJECT_REPR(child),"inkscape:stockid")) &&
                                                    SP_IS_GRADIENT(child))
                                                    {
                                                        object = child;
                                                    }
                                           }

                    }

    if (object == NULL) {

                   if (!strcmp(base,"marker"))  {
                           object = sp_marker_load_from_svg( name, doc);
                        }
                   else if (!strcmp(base,"pattern"))  {
                           object = sp_pattern_load_from_svg( name, doc);
                        }
                   else if (!strcmp(base,"gradient"))  {
                           object = sp_gradient_load_from_svg( name, doc);
                        }
                  }
    free(base);
    free(name);
    return object;
    }
    else {
         SPDesktop *desktop = inkscape_active_desktop();
         SPDocument *doc = SP_DT_DOCUMENT(desktop);
         SPObject *object =  doc->getObjectById(urn);

         return object;
    }
}

