/* Co-dependent headerfiles.  This works out, trust me --Ted */
#include "implementation/implementation.h"

#ifndef __INK_EXTENSION_H__
#define __INK_EXTENSION_H__

/*
 * Frontend to certain, possibly pluggable, actions
 *
 * Authors:
 *   Ted Gould <ted@gould.cx>
 *
 * Copyright (C) 2002-2004 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <gtk/gtk.h>
#include "widgets/menu.h"
#include "xml/repr.h"
#include "forward.h"
#include <libnr/nr-path.h>
#include <display/nr-arena-forward.h>

/** The key that is used to identify that the I/O should be autodetected */
#define SP_MODULE_KEY_AUTODETECT "autodetect"
/** This is the key for the SVG input module */
#define SP_MODULE_KEY_INPUT_SVG "modules.input.SVG"
/** Specifies the input module that should be used if none are selected */
#define SP_MODULE_KEY_INPUT_DEFAULT SP_MODULE_KEY_AUTODETECT
/** The key for outputing standard W3C SVG */
#define SP_MODULE_KEY_OUTPUT_SVG "modules.output.SVG.plain"
/** This is an output file that has SVG data with the Sodipodi namespace extensions */
#define SP_MODULE_KEY_OUTPUT_SVG_INKSCAPE "modules.output.SVG.sodipodi"
/** Which output module should be used? */
#define SP_MODULE_KEY_OUTPUT_DEFAULT SP_MODULE_KEY_AUTODETECT

#define SP_MODULE_KEY_PRINT_PS    "modules.print.ps"
#define SP_MODULE_KEY_PRINT_GNOME "modules.print.gnome"
#define SP_MODULE_KEY_PRINT_WIN32 "modules.print.win32"
#ifdef WIN32
#define SP_MODULE_KEY_PRINT_DEFAULT  SP_MODULE_KEY_PRINT_WIN32
#else
#ifdef WITH_GNOME_PRINT
#define SP_MODULE_KEY_PRINT_DEFAULT  SP_MODULE_KEY_PRINT_GNOME
#else
#define SP_MODULE_KEY_PRINT_DEFAULT  SP_MODULE_KEY_PRINT_PS
#endif
#endif

/* New C++ Stuff */
#define MIME_SVG "image/svg+xml"

namespace Inkscape {
namespace Extension {

/* Some early prototypes just for fun. */
class Input;
class Output;
class Filter;

/** The object that is the basis for the Extension system.  This object
    contains all of the information that all Extension have.  The
	individual items are detailed within. */
class Extension {
public:
	/** An enumeration to identify if the Extension has been loaded or not. */
	typedef enum {
		STATE_LOADED,
		STATE_UNLOADED
	} state_t;

private:
	gchar *id;                          /**< The unique identifier for the Extension */
	gchar *name;                        /**< A user friendly name for the Extension */
	state_t state;                      /**< Which state the Extension is currently in */

protected:
	SPRepr *repr;                       /**< The XML description of the Extension */
	Implementation::Implementation * imp;         /**< An object that holds all the functions for making this work */

public:
	Extension(SPRepr * in_repr);
	virtual ~Extension(void);

	void set_state (state_t in_state);
	state_t get_state (void);
	bool loaded (void);
	SPRepr * get_repr (void);
	gchar * get_id (void);
	gchar * get_name (void);
	Implementation::Implementation * set_implementation (Implementation::Implementation * in_imp);
};

class Input : public Extension {
	gchar *mimetype;             /**< What is the mime type this inputs? */
	gchar *extension;            /**< The extension of the input files */
	gchar *filetypename;         /**< A userfriendly name for the file type */
	gchar *filetypetooltip;      /**< A more detailed description of the filetype */

public:
    Input (SPRepr * in_repr);
	virtual ~Input (void);
	SPDocument * open (const gchar *uri);
	gchar * get_extension(void);
	gchar * get_filetypename(void);
	gchar * get_filetypetooltip(void);
	GtkDialog * prefs (const gchar *uri);
};

class Output : public Extension {
	gchar *mimetype;             /**< What is the mime type this inputs? */
	gchar *extension;            /**< The extension of the input files */
	gchar *filetypename;         /**< A userfriendly name for the file type */
	gchar *filetypetooltip;      /**< A more detailed description of the filetype */

public:
	Output (SPRepr * in_repr);
	virtual ~Output (void);

	void save (SPDocument *doc, const gchar *uri);
	GtkDialog * prefs (void);
	gchar * get_extension();
	gchar * get_filetypename();
	gchar * get_filetypetooltip();
};

class Filter : public Extension {

public:
	Filter (SPRepr * in_repr);
	virtual ~Filter (void);

	GtkDialog * prefs (void);
	void filter (SPDocument * doc);
};

class Print : public Extension {

public: /* TODO: These are public for the short term, but this should be fixed */
	SPItem *base;
	NRArena *arena;
	NRArenaItem *root;
	unsigned int dkey;

public:
	Print (SPRepr * in_repr);
	~Print (void);

	/* FALSE means user hit cancel */
	unsigned int setup (void);
	unsigned int set_preview (void);

	unsigned int begin (SPDocument *doc);
	unsigned int finish (void);

	/* Rendering methods */
	unsigned int bind (const NRMatrix *transform, float opacity);
	unsigned int release (void);
	unsigned int fill (const NRBPath *bpath, const NRMatrix *ctm, const SPStyle *style,
			       const NRRect *pbox, const NRRect *dbox, const NRRect *bbox);
	unsigned int stroke (const NRBPath *bpath, const NRMatrix *transform, const SPStyle *style,
				 const NRRect *pbox, const NRRect *dbox, const NRRect *bbox);
	unsigned int image (unsigned char *px, unsigned int w, unsigned int h, unsigned int rs,
				const NRMatrix *transform, const SPStyle *style);
};

}; /* namespace Extension */
}; /* namespace Inkscape */

#endif /* __INK_EXTENSION_H__ */
