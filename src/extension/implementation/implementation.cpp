/*
    Author:  Ted Gould <ted@gould.cx>
    Copyright (c) 2003-2004

    This code is licensed under the GNU GPL.  See COPYING for details.
 
    This file is the backend to the extensions system.  These are
    the parts of the system that most users will never see, but are
    important for implementing the extensions themselves.  This file
    contains the base class for all of that.
*/

#include "../extension.h"
#include "implementation.h"
#include <libnr/nr-point.h>

namespace Inkscape {
namespace Extension {
namespace Implementation {

/**
	\return   Was the load sucessful?
	\breif    This function is the stub load.  It just returns sucess.
	\param    module   The Extension that should be loaded.
*/
bool 
Implementation::load (Inkscape::Extension::Extension * module) {
	return TRUE;
} /* Implementation::load */

void 
Implementation::unload (Inkscape::Extension::Extension * module) {
	if (module->loaded()) {
		return module->set_state(Inkscape::Extension::Extension::STATE_UNLOADED);
	}
	return;
} /* Implementation::unload */

bool
Implementation::check (Inkscape::Extension::Extension * module) {
	/* If there are no checks, they all pass */
	return TRUE;
} /* Implemenation::check */

GtkDialog *
Implementation::prefs (Inkscape::Extension::Input * module, const gchar * filename) {
	return NULL;
} /* Implementation::prefs */

SPDocument *
Implementation::open (Inkscape::Extension::Input * module, const gchar * filename) {
	/* throw open_failed(); */
	return NULL;
} /* Implementation::open */

GtkDialog *
Implementation::prefs (Inkscape::Extension::Output * module) {
	return NULL;
} /* Implementation::prefs */

void
Implementation::save (Inkscape::Extension::Output * module, SPDocument * doc, const gchar * filename) {
	/* throw save_fail */
	return;
} /* Implementation::save */

GtkDialog *
Implementation::prefs (Inkscape::Extension::Effect * module) {

	return NULL;
} /* Implementation::prefs */

void
Implementation::effect (Inkscape::Extension::Effect * module, SPDocument * document) {
	/* throw filter_fail */
	return;
} /* Implementation::filter */

unsigned int
Implementation::setup (Inkscape::Extension::Print * module)
{
	return 0;
}

unsigned int
Implementation::set_preview (Inkscape::Extension::Print * module)
{
	return 0;
}


unsigned int
Implementation::begin (Inkscape::Extension::Print * module, SPDocument *doc)
{
	return 0;
}

unsigned int
Implementation::finish (Inkscape::Extension::Print * module)
{
	return 0;
}


/* Rendering methods */
unsigned int
Implementation::bind (Inkscape::Extension::Print * module, const NRMatrix *transform, float opacity)
{
	return 0;
}

unsigned int
Implementation::release (Inkscape::Extension::Print * module)
{
	return 0;
}

unsigned int
Implementation::fill (Inkscape::Extension::Print * module, const NRBPath *bpath, const NRMatrix *ctm, const SPStyle *style,
			   const NRRect *pbox, const NRRect *dbox, const NRRect *bbox)
{
	return 0;
}

unsigned int
Implementation::stroke (Inkscape::Extension::Print * module, const NRBPath *bpath, const NRMatrix *transform, const SPStyle *style,
			 const NRRect *pbox, const NRRect *dbox, const NRRect *bbox)
{
	return 0;
}

unsigned int
Implementation::image (Inkscape::Extension::Print * module, unsigned char *px, unsigned int w, unsigned int h, unsigned int rs,
			const NRMatrix *transform, const SPStyle *style)
{
	return 0;
}

unsigned int
Implementation::text (Inkscape::Extension::Print * module, const char *text,
                      NR::Point p, const SPStyle* style)
{
	return 0;
}

/**
   \brief  Tell the printing engine whether text should be text or path
   \retval TRUE  Render the text as a path
   \retval FALSE Render text using the text function (above)
 
    Default value is FALSE because most printing engines will support
    paths more than they'll support text.  (atleast they do today)
*/
bool
Implementation::textToPath(Inkscape::Extension::Print * ext)
{
    return FALSE;
}


}; /* namespace Implementation */
}; /* namespace Extension */
}; /* namespace Inkscape */

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
