#ifndef __MODULES_IMPLEMENTATION_H__
#define __MODULES_IMPLEMENTATION_H__

#include <gtk/gtkdialog.h>

namespace Inkscape {
namespace Extension {
namespace Implementation {
class Implementation;
};};};

#include <extension/extension.h>

namespace Inkscape {
namespace Extension {
namespace Implementation {

class Implementation {
public:
	/* Basic functions for all Extension */
	virtual bool load (Inkscape::Extension::Extension * module);    /**< The function that should be called to load the module */
	virtual void unload (Inkscape::Extension::Extension * module);  /**< The function that should be called to unload the module */

	/* Input functions */
	virtual GtkDialog * prefs (Inkscape::Extension::Input * module, const gchar * filename);
	                             /**< The function to find out information about the file */
	virtual SPDocument * open (Inkscape::Extension::Input * module, const gchar * filename);
	                             /**< Hey, there needs to be some function to do the work! */

	/* Output functions */
	virtual GtkDialog * prefs (Inkscape::Extension::Output * module);
	                             /**< The function to find out information about the file */
	virtual void save (Inkscape::Extension::Output * module, SPDocument * doc, const gchar * filename);
	                             /**< Hey, there needs to be some function to do the work! */

	/* Filter functions */
	virtual GtkDialog * prefs (Inkscape::Extension::Filter * module);
	                  /**< The function to find out information about the file */
	/* TODO: need to figure out what we need here */
	virtual void filter (Inkscape::Extension::Filter * module, SPDocument * document);
	                  /**< Hey, there needs to be some function to do the work! */

	/* Print functions */
	virtual unsigned int setup (Inkscape::Extension::Print * module);
	virtual unsigned int set_preview (Inkscape::Extension::Print * module);

	virtual unsigned int begin (Inkscape::Extension::Print * module, SPDocument *doc);
	virtual unsigned int finish (Inkscape::Extension::Print * module);

	/* Rendering methods */
	virtual unsigned int bind (Inkscape::Extension::Print * module, const NRMatrix *transform, float opacity);
	virtual unsigned int release (Inkscape::Extension::Print * module);
	virtual unsigned int fill (Inkscape::Extension::Print * module, const NRBPath *bpath, const NRMatrix *ctm, const SPStyle *style,
			       const NRRect *pbox, const NRRect *dbox, const NRRect *bbox);
	virtual unsigned int stroke (Inkscape::Extension::Print * module, const NRBPath *bpath, const NRMatrix *transform, const SPStyle *style,
				 const NRRect *pbox, const NRRect *dbox, const NRRect *bbox);
	virtual unsigned int image (Inkscape::Extension::Print * module, unsigned char *px, unsigned int w, unsigned int h, unsigned int rs,
				const NRMatrix *transform, const SPStyle *style);
};

}; /* namespace Implementation */
}; /* namespace Extension */
}; /* namespace Inkscape */

#endif /* __MODULES_IMPLEMENTATION_H__ */
