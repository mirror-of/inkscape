#ifndef SP_CANVAS_TRANSLATOR_H
#define SP_CANVAS_TRANSLATOR_H

#include <libgnomeui/gnome-canvas.h>

#define SP_TYPE_CANVAS_TRANSLATOR            (sp_canvas_translator_get_type ())
#define SP_CANVAS_TRANSLATOR(obj)            (GTK_CHECK_CAST ((obj), SP_TYPE_CANVAS_TRANSLATOR, SPCanvasTranslator))
#define SP_CANVAS_TRANSLATOR_CLASS(klass)    (GTK_CHECK_CLASS_CAST ((klass), SP_TYPE_CANVAS_TRANSLATOR, SPCanvasTranslatorClass))
#define SP_IS_CANVAS_TRANSLATOR(obj)         (GTK_CHECK_TYPE ((obj), SP_TYPE_CANVAS_TRANSLATOR))
#define SP_IS_CANVAS_TRANSLATOR_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), SP_TYPE_CANVAS_TRANSLATOR))

typedef struct _SPCanvasTranslator SPCanvasTranslator;
typedef struct _SPCanvasTranslatorClass SPCanvasTranslatorClass;

struct _SPCanvasTranslator {
    GnomeCanvasGroup group;
};

struct _SPCanvasTranslatorClass {
    GnomeCanvasGroupClass parent_class;
};


/* Standard Gtk function */

GtkType sp_canvas_translator_get_type (void);

#endif

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:99
  End:
  vim: filetype=c++:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
*/
