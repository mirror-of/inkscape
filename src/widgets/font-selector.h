#ifndef __SP_FONT_SELECTOR_H__
#define __SP_FONT_SELECTOR_H__

/*
 * Font selection widgets
 *
 * Authors:
 *   Chris Lahey <clahey@ximian.com>
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 1999-2001 Ximian, Inc.
 * Copyright (C) 2002 Lauris Kaplinski
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <glib.h>

G_BEGIN_DECLS

typedef struct _SPFontSelector SPFontSelector;
typedef struct _SPFontSelectorClass SPFontSelectorClass;

typedef struct _SPFontPreview SPFontPreview;
typedef struct _SPFontPreviewClass SPFontPreviewClass;

#define SP_TYPE_FONT_SELECTOR (sp_font_selector_get_type ())
#define SP_FONT_SELECTOR(o) (GTK_CHECK_CAST ((o), SP_TYPE_FONT_SELECTOR, SPFontSelector))
#define SP_IS_FONT_SELECTOR(o) (GTK_CHECK_TYPE ((o), SP_TYPE_FONT_SELECTOR))

#define SP_TYPE_FONT_PREVIEW (sp_font_preview_get_type ())
#define SP_FONT_PREVIEW(o) (GTK_CHECK_CAST ((o), SP_TYPE_FONT_PREVIEW, SPFontPreview))
#define SP_IS_FONT_PREVIEW(o) (GTK_CHECK_TYPE ((o), SP_TYPE_FONT_PREVIEW))

#include <libnrtype/nr-font.h>
#include <gtk/gtkwidget.h>

/* SPFontSelector */

GtkType sp_font_selector_get_type (void);

GtkWidget *sp_font_selector_new (void);

void sp_font_selector_set_font (SPFontSelector *fsel, NRFont *font);
void sp_font_selector_set_font_fuzzy (SPFontSelector *fsel, const guchar *family, const guchar *style);

NRFont *sp_font_selector_get_font (SPFontSelector *fsel);

/* SPFontPreview */

GtkType sp_font_preview_get_type (void);

GtkWidget *sp_font_preview_new (void);

void sp_font_preview_set_font (SPFontPreview *fprev, NRFont *font);
void sp_font_preview_set_rgba32 (SPFontPreview *fprev, guint32 rgba);
void sp_font_preview_set_phrase (SPFontPreview *fprev, const guchar *phrase);

G_END_DECLS

#endif







