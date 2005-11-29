#ifndef __EXTENSION_INTERNAL_PS_H__
#define __EXTENSION_INTERNAL_PS_H__

/*
 * This is the internal module used to do Postscript Printing
 *
 * Author:
 * 	   Lauris Kaplinski <lauris@kaplinski.com>
 * 	   Ted Gould <ted@gould.cx>
 *
 * Lauris: This code is in the public domain
 * Ted:    This code is under the GNU GPL
 */

#include <config.h>
#include "extension/extension.h"
#include "extension/implementation/implementation.h"
#include <set>

#include "libnr/nr-path.h"

#include "svg/stringstream.h"

namespace Inkscape {
namespace Extension {
namespace Internal {

class PrintPS : public Inkscape::Extension::Implementation::Implementation {
    float  _width;
    float  _height;
    FILE * _stream;
    unsigned short _dpi;
    bool   _bitmap;
    std::set<std::string> _latin1_encoded_fonts;
    bool   _newlatin1font_proc_defined;

	void print_bpath (SVGOStringStream &os, NArtBpath const *bp);

	void PrintPS::print_fill_style (SVGOStringStream &os, SPStyle const *style, NRRect const *pbox);
	void PrintPS::print_stroke_style (SVGOStringStream &os, SPStyle const *style);

	char const *PrintPS::PSFontName (SPStyle const *style);

	unsigned int print_image (FILE *ofp, guchar *px, unsigned int width, unsigned int height, unsigned int rs,
				       NRMatrix const *transform);
	void compress_packbits (int nin, guchar *src, int *nout, guchar *dst);

	/* ASCII 85 variables */
	guint32 ascii85_buf;
	int ascii85_len;
	int ascii85_linewidth;
	/* ASCII 85 Functions */
	void ascii85_init (void);
	void ascii85_flush (SVGOStringStream &os);
	inline void ascii85_out (guchar byte, SVGOStringStream &os);
	void ascii85_nout (int n, guchar *uptr, SVGOStringStream &os);
	void ascii85_done (SVGOStringStream &os);


public:
	PrintPS (void);
	virtual ~PrintPS (void);

	/* Print functions */
	virtual unsigned int setup (Inkscape::Extension::Print * module);
	/*
	virtual unsigned int set_preview (Inkscape::Extension::Print * module);
	*/

	virtual unsigned int begin (Inkscape::Extension::Print * module, SPDocument *doc);
	virtual unsigned int finish (Inkscape::Extension::Print * module);

	/* Rendering methods */
	virtual unsigned int bind (Inkscape::Extension::Print * module, NRMatrix const *transform, float opacity);
	virtual unsigned int release (Inkscape::Extension::Print * module);
	virtual unsigned int comment (Inkscape::Extension::Print * module, char const *comment);
	virtual unsigned int fill (Inkscape::Extension::Print * module, NRBPath const *bpath, NRMatrix const *ctm, SPStyle const *style,
			       NRRect const *pbox, NRRect const *dbox, NRRect const *bbox);
	virtual unsigned int stroke (Inkscape::Extension::Print * module, NRBPath const *bpath, NRMatrix const *transform, SPStyle const *style,
				 NRRect const *pbox, NRRect const *dbox, NRRect const *bbox);
	virtual unsigned int image (Inkscape::Extension::Print * module, unsigned char *px, unsigned int w, unsigned int h, unsigned int rs,
				NRMatrix const *transform, SPStyle const *style);
        virtual unsigned int text (Inkscape::Extension::Print *module, char const *text,
				   NR::Point p, SPStyle const *style);

	bool textToPath (Inkscape::Extension::Print * ext);
	static void init (void);
};

}  /* namespace Internal */
}  /* namespace Extension */
}  /* namespace Inkscape */

#endif /* __EXTENSION_INTERNAL_PS_H__ */
