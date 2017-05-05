#ifndef INKSCAPE_LIVEPATHEFFECT_PARAMETER_ARRAY_H
#define INKSCAPE_LIVEPATHEFFECT_PARAMETER_ARRAY_H

/*
 * Inkscape::LivePathEffectParameters
 *
* Copyright (C) Johan Engelen 2008 <j.b.c.engelen@utwente.nl>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <vector>

#include <glib.h>

#include "live_effects/parameter/parameter.h"

#include "svg/svg.h"
#include "svg/stringstream.h"

namespace Inkscape {

namespace LivePathEffect {

template <typename StorageType>
class ArrayParam : public Parameter {
public:
    ArrayParam( const Glib::ustring& label,
                const Glib::ustring& tip,
                const Glib::ustring& key,
                Inkscape::UI::Widget::Registry* wr,
                Effect* effect,
                size_t n = 0 )
        : Parameter(label, tip, key, wr, effect), _vector(n), _default_size(n)
    {

    }

    virtual ~ArrayParam() {

    };

    std::vector<StorageType> const & data() const {
        return _vector;
    }

    virtual Gtk::Widget * param_newWidget() {
        return NULL;
    }

    virtual bool param_readSVGValue(const gchar * strvalue) {
        _vector.clear();
        gchar ** strarray = g_strsplit(strvalue, "|", 0);
        gchar ** iter = strarray;
        while (*iter != NULL) {
            _vector.push_back( readSVG(*iter) );
            iter++;
        }
        g_strfreev (strarray);
        return true;
    }

    virtual gchar * param_getSVGValue() const {
        Inkscape::SVGOStringStream os;
        writeSVG(os, _vector);
        gchar * str = g_strdup(os.str().c_str());
        return str;
    }

    void param_setValue(std::vector<StorageType> const &new_vector) {
        _vector = new_vector;
    }

    void param_valueFromDefault() {
        param_setValue( std::vector<StorageType>(_default_size) );
    }

    void param_setAndWriteNewValue(std::vector<StorageType> const &new_vector) {
        Inkscape::SVGOStringStream os;
        writeSVG(os, new_vector);
        gchar * str = g_strdup(os.str().c_str());
        param_writeToRepr(str);
        g_free(str);
    }

protected:
    std::vector<StorageType> _vector;
    size_t _default_size;

    void writeSVG(SVGOStringStream &str, std::vector<StorageType> const &vector) const {
        for (unsigned int i = 0; i < vector.size(); ++i) {
            if (i != 0) {
                // separate items with pipe symbol
                str << " | ";
            }
            str << vector[i];
        }
    }

    StorageType readSVG(const gchar * str);

private:
    ArrayParam(const ArrayParam&);
    ArrayParam& operator=(const ArrayParam&);
};


} //namespace LivePathEffect

} //namespace Inkscape

#endif

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
