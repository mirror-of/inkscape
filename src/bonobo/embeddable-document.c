#define SP_EMBEDDABLE_DOCUMENT_C

#include <config.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#include <bonobo.h>
#include <bonobo/bonobo-print.h>
#include <libgnomeprint/gnome-print.h>

#include "../forward.h"
#include "../sp-object.h"
#include "../sp-item.h"
#include "../sodipodi.h"
#include "../document.h"
#include "sodipodi-bonobo.h"
#include "embeddable-desktop.h"
#include "embeddable-drawing.h"
#include "embeddable-document.h"

#define STREAM_CHUNK_SIZE 4096

#if 0
static void sp_embeddable_document_destroyed (SPEmbeddableDocument * document);
#endif


static void
sp_embeddable_document_class_init (GtkObjectClass * klass)
{
}

static void
sp_embeddable_document_init (GtkObject * object)
{
	SPEmbeddableDocument * document;

	document = (SPEmbeddableDocument *) object;

	document->document = NULL;
}

GtkType
sp_embeddable_document_get_type (void)
{
	static GtkType type = 0;
	if (!type) {
		GtkTypeInfo info = {
			"SPEmbeddableDocument",
			sizeof (SPEmbeddableDocument),
			sizeof (SPEmbeddableDocumentClass),
			(GtkClassInitFunc) sp_embeddable_document_class_init,
			(GtkObjectInitFunc) sp_embeddable_document_init,
			NULL, NULL, NULL
		};
		type = bonobo_x_type_unique (BONOBO_EMBEDDABLE_TYPE, NULL, NULL, 0, &info);
	}
	return type;
}

static void
sp_embeddable_document_destroyed (SPEmbeddableDocument * document)
{
#if 0
	if (--sp_bonobo_objects > 0) return;

	/* Should we destroy factory */

	gtk_main_quit ();
#else
	sp_document_unref (document->document);
#endif
}

static int
sp_embeddable_document_pf_load (BonoboPersistFile *pfile,
				const CORBA_char  *filename,
				CORBA_Environment *ev,
				gpointer           closure)
{
	SPEmbeddableDocument * document;
	SPDocument * newdocument;

	document = SP_EMBEDDABLE_DOCUMENT (closure);

	newdocument = sp_document_new (filename, FALSE);

	if (newdocument == NULL)
		return -1;

	gtk_object_unref (GTK_OBJECT (document->document));
	document->document = newdocument;

	bonobo_embeddable_foreach_view (BONOBO_EMBEDDABLE (document),
		sp_embeddable_desktop_new_doc, NULL);

	bonobo_embeddable_foreach_item (BONOBO_EMBEDDABLE (document),
		sp_embeddable_drawing_new_doc, NULL);

	return 0;
}

static int
sp_embeddable_document_pf_save (BonoboPersistFile *pfile,
				const CORBA_char  *filename,
				CORBA_Environment *ev,
				gpointer           closure)
{
	SPEmbeddableDocument * document;

	document = SP_EMBEDDABLE_DOCUMENT (closure);

	sp_repr_save_file (sp_document_repr_doc (document->document), filename);

	return 0;
}

static gint
sp_bonobo_stream_read (Bonobo_Stream stream, gchar ** buffer)
{
	Bonobo_Stream_iobuf * iobuf;
	CORBA_Environment ev;
	gint len;

	CORBA_exception_init (&ev);

	* buffer = NULL;
	len = 0;

	do {
		Bonobo_Stream_read (stream, STREAM_CHUNK_SIZE, &iobuf, &ev);

		if (ev._major != CORBA_NO_EXCEPTION) {
			if (* buffer != NULL) g_free (* buffer);
			len = -1;
			break;
		}

		* buffer = g_realloc (* buffer, len + iobuf->_length);
		memcpy (* buffer + len, iobuf->_buffer, iobuf->_length);

		len += iobuf->_length;

	} while (iobuf->_length > 0);

	CORBA_exception_free (&ev);

	return len;
}

static void
sp_embeddable_document_ps_load (BonoboPersistStream *ps, const Bonobo_Stream stream,
				Bonobo_Persist_ContentType type, gpointer closure,
				CORBA_Environment *ev)
{
	SPEmbeddableDocument * document;
	SPDocument * newdocument;
	gchar * buffer;
	gint len;

	document = SP_EMBEDDABLE_DOCUMENT (closure);

	len = sp_bonobo_stream_read (stream, &buffer);
	if (len < 0) {
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
				     ex_Bonobo_Persist_WrongDataType, NULL);
		return;
	}

	newdocument = sp_document_new_from_mem (buffer, len, FALSE);

	g_free (buffer);

	if (newdocument == NULL) {
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
				     ex_Bonobo_Persist_WrongDataType, NULL);
		return;
	}

	gtk_object_unref (GTK_OBJECT (document->document));
	document->document = newdocument;

	bonobo_embeddable_foreach_view (BONOBO_EMBEDDABLE (document),
		sp_embeddable_desktop_new_doc, NULL);

	bonobo_embeddable_foreach_item (BONOBO_EMBEDDABLE (document),
		sp_embeddable_drawing_new_doc, NULL);
}

static void
sp_embeddable_document_ps_save (BonoboPersistStream *ps,
				const Bonobo_Stream stream,
				Bonobo_Persist_ContentType type,
				gpointer closure,
				CORBA_Environment *ev)
{
	int   fd;
	FILE *file;
	char *filename = NULL;
	SPEmbeddableDocument * document;

	document = SP_EMBEDDABLE_DOCUMENT (closure);

	/* FIXME: super dirty but functional */
	do {
		if (filename) g_free (filename);
		filename = g_strdup ("sodipodiXXXXXX");
		fd = mkstemp (filename);

	} while (fd == -1);

	file = fdopen (fd, "w");
	g_return_if_fail (file != NULL);

	sp_repr_save_stream (sp_document_repr_doc (document->document), file);

	fclose (file);

	{ /* Transfer file to stream ... */
		guint8 data [4096];
		gulong more;
		int    v;

		do {
			more = sizeof (data);

			do {
				v = read (fd, data, 
					  MIN (sizeof (data), more));
			} while ((v == -1) && (errno == EINTR));

			if (v == -1) 
				goto copy_to_except;

			if (v <= 0) 
				break;

			more -= v;

			bonobo_stream_client_write (stream, data, v, ev);

			if (BONOBO_EX (ev))
				goto copy_to_except;

		} while (v > 0);

	}
	
	close (fd);
	unlink (filename);
	return;

 copy_to_except:
	CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
			     ex_Bonobo_IOError, NULL);
	close (fd);
	unlink (filename);
}

/*
 * fixme:
 * diferent aspect modes
 * different clip modes
 */

static void
sp_embeddable_document_print (GnomePrintContext * ctx,
				gdouble width,
				gdouble height,
				const Bonobo_PrintScissor * scissor,
				gpointer data)
{
	SPEmbeddableDocument * document;
	gdouble scale, dw, dh;

	document = SP_EMBEDDABLE_DOCUMENT (data);

	dw = sp_document_width (document->document);
	dh = sp_document_height (document->document);
	scale = MIN (width / dw, height / dh);
	dw *= scale;
	dh *= scale;

	gnome_print_gsave (ctx);

	gnome_print_translate (ctx, width / 2 - dw / 2, height / 2 - dh / 2);

	gnome_print_scale (ctx, scale, scale);

	sp_item_print (SP_ITEM (sp_document_root (document->document)), ctx);

	gnome_print_grestore (ctx);
}

BonoboObject *
sp_embeddable_document_new (void)
{
	SPEmbeddableDocument * document;
	BonoboPersistFile * pfile;
	BonoboPersistStream * pstream;
	BonoboPrint * print;

	document = gtk_type_new (SP_EMBEDDABLE_DOCUMENT_TYPE);

	document->document = sp_document_new (NULL, FALSE);

	bonobo_embeddable_construct_full (BONOBO_EMBEDDABLE (document),
		sp_embeddable_desktop_factory, NULL,
		sp_embeddable_drawing_factory, NULL);

	/* Add interfaces */

	pfile = bonobo_persist_file_new (sp_embeddable_document_pf_load,
		sp_embeddable_document_pf_save, document);

	if (pfile == NULL) {
		gtk_object_unref (GTK_OBJECT (document));
		return CORBA_OBJECT_NIL;
	}

	bonobo_object_add_interface (BONOBO_OBJECT (document),
				     BONOBO_OBJECT (pfile));

	pstream = bonobo_persist_stream_new (sp_embeddable_document_ps_load,
					     sp_embeddable_document_ps_save,
					     NULL, NULL, document);

	if (pstream == NULL) {
		gtk_object_unref (GTK_OBJECT (document));
		bonobo_object_unref (BONOBO_OBJECT (pfile));
		return CORBA_OBJECT_NIL;
	}

	bonobo_object_add_interface (BONOBO_OBJECT (document),
				     BONOBO_OBJECT (pstream));

	print = bonobo_print_new (sp_embeddable_document_print, document);

	if (!print) {
		gtk_object_unref (GTK_OBJECT (document));
		bonobo_object_unref (BONOBO_OBJECT (pfile));
		bonobo_object_unref (BONOBO_OBJECT (pstream));
		return CORBA_OBJECT_NIL;
	}

	bonobo_object_add_interface (BONOBO_OBJECT (document),
				     BONOBO_OBJECT (print));


#if 0
#if 0
	sp_bonobo_objects++;
#else
	sodipodi_ref ();
#endif
#endif
	gtk_signal_connect (GTK_OBJECT (document), "destroy",
		GTK_SIGNAL_FUNC (sp_embeddable_document_destroyed), NULL);

	return BONOBO_OBJECT (document);
}
