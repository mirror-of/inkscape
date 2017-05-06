#ifndef INKSCAPE_LIVEPATHEFFECT_PARAMETER_ENUM_H
#define INKSCAPE_LIVEPATHEFFECT_PARAMETER_ENUM_H

/*
 * Inkscape::LivePathEffectParameters
 *
* Copyright (C) Johan Engelen 2007 <j.b.c.engelen@utwente.nl>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "ui/widget/registered-enums.h"
#include <glibmm/ustring.h>
#include "live_effects/effect.h"
#include "live_effects/parameter/parameter.h"
#include "verbs.h"

namespace Inkscape {

namespace LivePathEffect {

template<typename E> class EnumParam : public Parameter {
public:
    EnumParam(  const Glib::ustring& label,
                const Glib::ustring& tip,
                const Glib::ustring& key,
                const Util::EnumDataConverter<E>& c,
                Inkscape::UI::Widget::Registry* wr,
                Effect* effect,
                E defaultvalue,
                bool sort = true)
        : Parameter(label, tip, key, wr, effect)
    {
        enumdataconv = &c;
        defvalue = defaultvalue;
        value = defvalue;
        sorted = sort;
    };

    virtual ~EnumParam() { };

    virtual Gtk::Widget * param_newWidget() {
        Inkscape::UI::Widget::RegisteredEnum<E> *regenum = Gtk::manage ( 
            new Inkscape::UI::Widget::RegisteredEnum<E>(param_label,
                                                        param_tooltip,
                                                        param_key,
                                                        *enumdataconv,
                                                        *param_wr,
                                                        param_effect->getRepr(),
                                                        param_effect->getSPDoc(),
                                                        sorted) );

        regenum->set_active_by_id(value);
        regenum->combobox()->setProgrammatically = false;
        regenum->set_undo_parameters(SP_VERB_DIALOG_LIVE_PATH_EFFECT, _("Change enumeration parameter"));
        return dynamic_cast<Gtk::Widget *> (regenum);
    };

    bool param_readSVGValue(const gchar * strvalue) {
        if (!strvalue) {
            param_valueFromDefault();
            return true;
        }
        param_setValue( enumdataconv->get_id_from_key(Glib::ustring(strvalue)) );
        return true;
    };

    gchar * param_getSVGValue() const {
        gchar * str = g_strdup( enumdataconv->get_key(value).c_str() );
        return str;
    };

    E param_getValue() const {
        return value;
    }


    void param_setValue(E val) {
        if (value != val) {
            param_effect->upd_params = true;
        }
        value = val;
    }

    void param_valueFromDefault(bool /*write*/) {
        param_setValue(defvalue);
    }

    virtual void param_updateDefault(E defaultvalue) {
        defvalue = defaultvalue;
    }

    virtual void param_updateDefault(const gchar * defaultvalue) {
        param_updateDefault(enumdataconv->get_id_from_key(Glib::ustring(defaultvalue)));
    }

private:
    EnumParam(const EnumParam&);
    EnumParam& operator=(const EnumParam&);

    E value;
    E defvalue;
    bool sorted;

    const Util::EnumDataConverter<E> * enumdataconv;
};


}; //namespace LivePathEffect

}; //namespace Inkscape

#endif
