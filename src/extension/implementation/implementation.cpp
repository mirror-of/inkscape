
#include "../extension.h"
#include "implementation.h"

namespace Inkscape {
namespace Extension {
namespace Implementation {

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
Implementation::prefs (Inkscape::Extension::Filter * module) {

	return NULL;
} /* Implementation::prefs */

void
Implementation::filter (Inkscape::Extension::Filter * module, SPDocument * document) {
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

}; /* namespace Implementation */
}; /* namespace Extension */
}; /* namespace Inkscape */
