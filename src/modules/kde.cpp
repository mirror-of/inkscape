#define __SP_KDE_CPP__

/*
 * KDE utilities for Sodipodi
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 2003 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <config.h>

#include <libnr/nr-macros.h>
#include <libnr/nr-matrix.h>

#include <qtimer.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qcombobox.h>
#include <kapp.h>
#include <kfiledialog.h>
#include <ktoolbar.h>
#include <gtk/gtkmain.h>

#include <helper/sp-intl.h>

#include "kde.h"
#include "kde-private.h"

#define SP_FOREIGN_FREQ 32
#define SP_FOREIGN_MAX_ITER 4

void
SPKDEBridge::EventHook (void) {
	int cdown = 0;
	while ((cdown++ < SP_FOREIGN_MAX_ITER) && gdk_events_pending ()) {
		gtk_main_iteration_do (FALSE);
	}
	gtk_main_iteration_do (FALSE);
}

void
SPKDEBridge::TimerHook (void) {
	int cdown = 10;
	while ((cdown++ < SP_FOREIGN_MAX_ITER) && gdk_events_pending ()) {
		gtk_main_iteration_do (FALSE);
	}
	gtk_main_iteration_do (FALSE);
}

static KApplication *KDESodipodi = NULL;
static SPKDEBridge *Bridge = NULL;
static bool SPKDEModal = FALSE;

static void
sp_kde_gdk_event_handler (GdkEvent *event)
{
	if (SPKDEModal) {
		// KDE widget is modal, filter events
		switch (event->type) {
		case GDK_NOTHING:
		case GDK_DELETE:
		case GDK_SCROLL:
		case GDK_BUTTON_PRESS:
		case GDK_2BUTTON_PRESS:
		case GDK_3BUTTON_PRESS:
		case GDK_BUTTON_RELEASE:
		case GDK_KEY_PRESS:
		case GDK_KEY_RELEASE:
		case GDK_DRAG_STATUS:
		case GDK_DRAG_ENTER:
		case GDK_DRAG_LEAVE:
		case GDK_DRAG_MOTION:
		case GDK_DROP_START:
		case GDK_DROP_FINISHED:
			return;
			break;
		default:
			break;
		}
	}
	gtk_main_do_event (event);
}

void
sp_kde_init (int argc, char **argv, const char *name)
{
	KDESodipodi = new KApplication (argc, argv, name);
	Bridge = new SPKDEBridge ("KDE Bridge");

	QObject::connect (KDESodipodi, SIGNAL (guiThreadAwake ()), Bridge, SLOT (EventHook ()));

	gdk_event_handler_set ((GdkEventFunc) sp_kde_gdk_event_handler, NULL, NULL);
}

void
sp_kde_finish (void)
{
	delete Bridge;
	delete KDESodipodi;
}

char *
sp_kde_get_open_filename (unsigned char *dir, unsigned char *filter, unsigned char *title)
{
	QString fileName;

	QTimer timer;
	QObject::connect (&timer, SIGNAL (timeout ()), Bridge, SLOT (TimerHook ()));
	timer.changeInterval (1000 / SP_FOREIGN_FREQ);
	SPKDEModal = TRUE;

	fileName = KFileDialog::getOpenFileName ((const char *) dir,
						 (const char *) filter,
						 NULL,
						 (const char *) title);

	SPKDEModal = FALSE;

        return g_strdup (fileName);
}

char *
sp_kde_get_write_filename (unsigned char *dir, unsigned char *filter, unsigned char *title)
{
	QString fileName;

	QTimer timer;
	QObject::connect (&timer, SIGNAL (timeout ()), Bridge, SLOT (TimerHook ()));
	timer.changeInterval (1000 / SP_FOREIGN_FREQ);
	SPKDEModal = TRUE;

	fileName = KFileDialog::getSaveFileName ((const char *) dir,
						 (const char *) filter,
						 NULL,
						 (const char *) title);

	SPKDEModal = FALSE;

        return g_strdup (fileName);
}

char *
sp_kde_get_save_filename (unsigned char *dir, unsigned int *spns)
{
	QString fileName;

	QTimer timer;
	QObject::connect (&timer, SIGNAL (timeout ()), Bridge, SLOT (TimerHook ()));
	timer.changeInterval (1000 / SP_FOREIGN_FREQ);
	SPKDEModal = TRUE;

	QWidget *w = new QWidget;
	QHBoxLayout *box = new QHBoxLayout (w);
	box->addStretch ();
	box->addWidget (new QLabel (_("Document variant:"), w));

	QComboBox *cb = new QComboBox (w, "File type");
	cb->insertItem ("SVG with sodipodi namespace");
	cb->insertItem ("Standard SVG");

	box->addWidget (cb);

	// SPSaveFileDialog *dlg = new SPSaveFileDialog (QDir::currentDirPath (), _("*.svg *.svgz|SVG files\n*|All files"), NULL);
	KFileDialog *dlg = new KFileDialog ((const char *) dir,
					    _("*.svg *.svgz|SVG files\n*.xml|XML files\n*|All files"), NULL,
					    NULL, TRUE, w);
	dlg->setCaption (_("Save document as"));
	dlg->setOperationMode (KFileDialog::Saving);

	if (dlg->exec () == QDialog::Accepted) {
		fileName = dlg->selectedFile ();
		*spns = (cb->currentItem () == 0);
	} else {
		fileName = (const char *) NULL;
	}
	delete dlg;

	SPKDEModal = FALSE;

        return g_strdup (fileName);
}

// Printing

#include <kprinter.h>
#include <qpainter.h>
#include <qimage.h>

G_BEGIN_DECLS
#include "display/nr-arena-item.h"
#include "document.h"
G_END_DECLS

#define SP_TYPE_MODULE_PRINT_KDE (sp_module_print_kde_get_type())
#define SP_MODULE_PRINT_KDE(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), SP_TYPE_MODULE_PRINT_KDE, SPModulePrintKDE))
#define SP_IS_MODULE_PRINT_KDE(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), SP_TYPE_MODULE_PRINT_KDE))

typedef struct _SPModulePrintKDE SPModulePrintKDE;
typedef struct _SPModulePrintKDEClass SPModulePrintKDEClass;

struct _SPModulePrintKDE {
	SPModulePrint module;

	float width;
	float height;

	KPrinter *kprinter;
	QPainter *painter;
};

struct _SPModulePrintKDEClass {
	SPModulePrintClass module_print_class;
};

unsigned int sp_module_print_kde_get_type (void);

static void sp_module_print_kde_class_init (SPModulePrintClass *klass);
static void sp_module_print_kde_init (SPModulePrintKDE *gpmod);
static void sp_module_print_kde_finalize (GObject *object);

static unsigned int sp_module_print_kde_setup (SPModulePrint *mod);
static unsigned int sp_module_print_kde_set_preview (SPModulePrint *mod);
static unsigned int sp_module_print_kde_begin (SPModulePrint *mod, SPDocument *doc);
static unsigned int sp_module_print_kde_finish (SPModulePrint *mod);

static unsigned int sp_module_print_kde_bind (SPModulePrint *mod, const NRMatrixF *transform, float opacity);
static unsigned int sp_module_print_kde_release (SPModulePrint *mod);
static unsigned int sp_module_print_kde_fill (SPModulePrint *mod, const NRBPath *bpath, const NRMatrixF *ctm, const SPStyle *style,
						const NRRectF *pbox, const NRRectF *dbox, const NRRectF *bbox);
static unsigned int sp_module_print_kde_stroke (SPModulePrint *mod, const NRBPath *bpath, const NRMatrixF *ctm, const SPStyle *style,
						  const NRRectF *pbox, const NRRectF *dbox, const NRRectF *bbox);
static unsigned int sp_module_print_kde_image (SPModulePrint *mod, unsigned char *px, unsigned int w, unsigned int h, unsigned int rs,
						 const NRMatrixF *transform, const SPStyle *style);

static SPModulePrintClass *print_kde_parent_class;

unsigned int
sp_module_print_kde_get_type (void)
{
	static GType type = 0;
	if (!type) {
		GTypeInfo info = {
			sizeof (SPModulePrintKDEClass),
			NULL, NULL,
			(GClassInitFunc) sp_module_print_kde_class_init,
			NULL, NULL,
			sizeof (SPModulePrintKDE),
			16,
			(GInstanceInitFunc) sp_module_print_kde_init,
		};
		type = g_type_register_static (SP_TYPE_MODULE_PRINT, "SPModulePrintKDE", &info, (GTypeFlags) 0);
	}
	return type;
}

static void
sp_module_print_kde_class_init (SPModulePrintClass *klass)
{
	GObjectClass *g_object_class;
	SPModulePrintClass *module_print_class;

	g_object_class = (GObjectClass *)klass;
	module_print_class = (SPModulePrintClass *) klass;

	print_kde_parent_class = (SPModulePrintClass *) g_type_class_peek_parent (klass);

	g_object_class->finalize = sp_module_print_kde_finalize;

	module_print_class->setup = sp_module_print_kde_setup;
	module_print_class->set_preview = sp_module_print_kde_set_preview;
	module_print_class->begin = sp_module_print_kde_begin;
	module_print_class->finish = sp_module_print_kde_finish;
	module_print_class->bind = sp_module_print_kde_bind;
	module_print_class->release = sp_module_print_kde_release;
	module_print_class->fill = sp_module_print_kde_fill;
	module_print_class->stroke = sp_module_print_kde_stroke;
	module_print_class->image = sp_module_print_kde_image;
}

static void
sp_module_print_kde_init (SPModulePrintKDE *kpmod)
{
	kpmod->kprinter = new KPrinter (TRUE, QPrinter::PrinterResolution);
	kpmod->kprinter->setFullPage (TRUE);
	kpmod->kprinter->setPageSelection (KPrinter::ApplicationSide);
}

static void
sp_module_print_kde_finalize (GObject *object)
{
	SPModulePrintKDE *kpmod;

	kpmod = (SPModulePrintKDE *) object;

	if (kpmod->painter) delete kpmod->painter;
	delete kpmod->kprinter;

	G_OBJECT_CLASS (print_kde_parent_class)->finalize (object);
}

static unsigned int
sp_module_print_kde_setup (SPModulePrint *mod)
{
	SPModulePrintKDE *kpmod;
	unsigned int ret;

	kpmod = (SPModulePrintKDE *) mod;

	QTimer timer;
	QObject::connect (&timer, SIGNAL (timeout ()), Bridge, SLOT (TimerHook ()));
	timer.changeInterval (1000 / SP_FOREIGN_FREQ);

	SPKDEModal = TRUE;
	ret = kpmod->kprinter->setup (NULL);
	SPKDEModal = FALSE;

	return ret;
}

static unsigned int
sp_module_print_kde_set_preview (SPModulePrint *mod)
{
	SPModulePrintKDE *kpmod;
	unsigned int ret;

	kpmod = (SPModulePrintKDE *) mod;

	// use a "clean" KPrinter object (independant from previous print jobs),
	// this is not necessary, it depends on the application

	// KPrinter prt( false );
	kpmod->kprinter->setPreviewOnly (TRUE);
}

static unsigned int
sp_module_print_kde_begin (SPModulePrint *mod, SPDocument *doc)
{
	SPModulePrintKDE *kpmod;

	kpmod = (SPModulePrintKDE *) mod;

	kpmod->width = sp_document_width (doc);
	kpmod->height = sp_document_height (doc);

	kpmod->painter = new QPainter (kpmod->kprinter);

	return 0;
}

#define RESOLUTION 72
#define PS2PRINTER (600.0 / 72.0)

static unsigned int
sp_module_print_kde_finish (SPModulePrint *mod)
{
	SPModulePrintKDE *kpmod;
	NRMatrixF affine;
	double x0, y0, x1, y1;

	kpmod = (SPModulePrintKDE *) mod;

	QTimer timer;
	QObject::connect (&timer, SIGNAL (timeout ()), Bridge, SLOT (TimerHook ()));
	timer.changeInterval (1000 / SP_FOREIGN_FREQ);
	SPKDEModal = TRUE;

	int dpi = kpmod->kprinter->resolution ();
	float scale = dpi / 72.0;

	y0 = 0.0;
	x0 = 0.0;
	x1 = kpmod->width;
	y1 = kpmod->height;

	int width = (int) (kpmod->width * scale + 0.5);
	int height = (int) (kpmod->height * scale + 0.5);

	// int kpwidth = (int) (kpmod->width * PS2PRINTER + 0.5);
	// int kpheight = (int) (kpmod->height * PS2PRINTER + 0.5);

	affine.c[0] = width / ((x1 - x0) * 1.25);
	affine.c[1] = 0.0;
	affine.c[2] = 0.0;
	affine.c[3] = height / ((y1 - y0) * 1.25);
	affine.c[4] = -affine.c[0] * x0 * 1.25;
	affine.c[5] = -affine.c[3] * y0 * 1.25;

	nr_arena_item_set_transform (mod->root, &affine);

	QImage img (width, 64, 32);

	unsigned char *px = nr_new (unsigned char, 4 * width * 64);

	int y;
	for (y = 0; y < height; y += 64) {
		NRRectL bbox;
		NRGC gc;
		/* Set area of interest */
		bbox.x0 = 0;
		bbox.y0 = y;
		bbox.x1 = width;
		bbox.y1 = MIN (height, y + 64);
		/* Update to renderable state */
		nr_matrix_d_set_identity (&gc.transform);
		nr_arena_item_invoke_update (mod->root, &bbox, &gc, NR_ARENA_ITEM_STATE_ALL, NR_ARENA_ITEM_STATE_NONE);
		/* Render */
		NRPixBlock pb;
		nr_pixblock_setup_extern (&pb, NR_PIXBLOCK_MODE_R8G8B8A8N,
					  bbox.x0, bbox.y0, bbox.x1, bbox.y1,
					  px, 4 * width, FALSE, FALSE);
		memset (px, 0xff, 4 * width * 64);
		nr_arena_item_invoke_render (mod->root, &bbox, &pb, 0);
		/* Blit into QImage */
		int xx, yy;
		for (yy = bbox.y0; yy < bbox.y1; yy++) {
			unsigned char *s = NR_PIXBLOCK_PX (&pb) + pb.rs * (yy - bbox.y0);
			unsigned int *d = (unsigned int *) img.scanLine (yy - bbox.y0);
			for (xx = bbox.x0; xx < bbox.x1; xx++) {
				d[0] = qRgb (s[0], s[1], s[2]);
				s += 4;
				d += 1;
			}
		}
		nr_pixblock_release (&pb);
		kpmod->painter->drawImage (0, y, img, 0, 0, width, 64);
	}

	nr_free (px);

	kpmod->painter->end ();
	delete kpmod->painter;
	kpmod->painter = NULL;

	SPKDEModal = FALSE;

	return 0;
}

static unsigned int
sp_module_print_kde_bind (SPModulePrint *mod, const NRMatrixF *t, float opacity)
{
	SPModulePrintKDE *kpmod;

	kpmod = (SPModulePrintKDE *) mod;

	// kpmod->painter->save ();
	// kpmod->painter->setWorldMatrix (QWMatrix (t->c[0], t->c[1], t->c[2], t->c[3], t->c[4], t->c[5]), TRUE);

	return 0;
}

static unsigned int
sp_module_print_kde_release (SPModulePrint *mod)
{
	SPModulePrintKDE *kpmod;

	kpmod = (SPModulePrintKDE *) mod;

	// kpmod->painter->restore ();

	return 0;
}

static unsigned int
sp_module_print_kde_fill (SPModulePrint *mod, const NRBPath *bpath, const NRMatrixF *ctm, const SPStyle *style,
			    const NRRectF *pbox, const NRRectF *dbox, const NRRectF *bbox)
{
	SPModulePrintKDE *kpmod;

	kpmod = (SPModulePrintKDE *) mod;

	// if (style->fill->type == SP_FILL_TYPE_COLOR) {
		// float rgb[3];
		// sp_color_get_rgb_floatv (&style->fill.value.color, rgb);
		// kpmod->painter->setBrush (QColor ((int) (rgb[0] * 255.9999), (int) (rgb[1] * 255.9999), (int) (rgb[2] * 255.9999)));
	// }

	return 0;
}

static unsigned int
sp_module_print_kde_stroke (SPModulePrint *mod, const NRBPath *bpath, const NRMatrixF *ctm, const SPStyle *style,
			    const NRRectF *pbox, const NRRectF *dbox, const NRRectF *bbox)
{
	SPModulePrintKDE *kpmod;

	kpmod = (SPModulePrintKDE *) mod;

	return 0;
}

static unsigned int
sp_module_print_kde_image (SPModulePrint *mod, unsigned char *px, unsigned int w, unsigned int h, unsigned int rs,
			   const NRMatrixF *transform, const SPStyle *style)
{
	SPModulePrintKDE *kpmod;

	kpmod = (SPModulePrintKDE *) mod;

	return 0;
}

SPModulePrint *
sp_kde_get_module_print (void)
{
	return (SPModulePrint *) g_object_new (SP_TYPE_MODULE_PRINT_KDE, NULL);
}


