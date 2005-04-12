/** \file
 * Parameters for extensions.
 */

/*
 * Authors:
 *   Ted Gould <ted@gould.cx>
 *
 * Copyright (C) 2005 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <glib.h>

#include <gtkmm/adjustment.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/box.h>
#include <gtkmm/label.h>
#include <gtkmm/spinbutton.h>

#include <glibmm/i18n.h>

#include "extension.h"
#include "prefs-utils.h"

#include "parameter.h"

/** \brief  The root directory in the preferences database for extension
            related parameters. */
#define PREF_DIR "extensions"

namespace Inkscape {
namespace Extension {

/** \brief  A boolean parameter */
class ParamBool : public Parameter {
private:
    /** \brief  Internal value. */
    bool _value;
public:
    /** \brief  Use the superclass' allocator and set the \c _value */
    ParamBool(const gchar * name, const gchar * guitext, Inkscape::Extension::Extension * ext, bool value) :
        Parameter(name, guitext, ext), _value(value) {};
    /** \brief  Returns \c _value */
    bool get (const Inkscape::XML::Document * doc) { return _value; }
    bool set (bool in, Inkscape::XML::Document * doc);
    Gtk::Widget * get_widget(void);
};

class ParamInt : public Parameter {
private:
    /** \brief  Internal value. */
    int _value;
public:
    /** \brief  Use the superclass' allocator and set the \c _value */
    ParamInt(const gchar * name, const gchar * guitext, Inkscape::Extension::Extension * ext, int value) :
        Parameter(name, guitext, ext), _value(value) {};
    /** \brief  Returns \c _value */
    int get (const Inkscape::XML::Document * doc) { return _value; }
    int set (int in, Inkscape::XML::Document * doc);
    Gtk::Widget * get_widget(void);
};

class ParamFloat : public Parameter {
private:
    /** \brief  Internal value. */
    float _value;
public:
    /** \brief  Use the superclass' allocator and set the \c _value */
    ParamFloat(const gchar * name, const gchar * guitext, Inkscape::Extension::Extension * ext, float value) :
        Parameter(name, guitext, ext), _value(value) {};
    /** \brief  Returns \c _value */
    float get (const Inkscape::XML::Document * doc) { return _value; }
    float set (float in, Inkscape::XML::Document * doc);
    Gtk::Widget * get_widget(void);
};

class ParamString : public Parameter {
private:
    /** \brief  Internal value.  This should point to a string that has
                been allocated in memory.  And should be free'd. */
    gchar * _value;
public:
    ParamString(const gchar * name, const gchar * guitext, Inkscape::Extension::Extension * ext, const gchar * value);
    ~ParamString(void);
    /** \brief  Returns \c _value, with a \i const to protect it. */
    const gchar * get (const Inkscape::XML::Document * doc) { return _value; }
    const gchar * set (const gchar * in, Inkscape::XML::Document * doc);
};

/**
    \return None
    \brief  This function creates a parameter that can be used later.  This
            is typically done in the creation of the extension and defined
            in the XML file describing the extension (it's private so people
            have to use the system) :)
    \param  in_repr  The XML describing the parameter

    This function first grabs all of the data out of the Repr and puts
    it into local variables.  Actually, these are just pointers, and the
    data is not duplicated so we need to be careful with it.  If there
    isn't a name or a type in the XML, then no parameter is created as
    the function just returns.

    From this point on, we're pretty committed as we've allocated an
    object and we're starting to fill it.  The name is set first, and
    is created with a strdup to actually allocate memory for it.  Then
    there is a case statement (roughly because strcmp requires 'ifs')
    based on what type of parameter this is.  Depending which type it
    is, the value is interpreted differently, but they are relatively
    straight forward.  In all cases the value is set to the default
    value from the XML and the type is set to the interpreted type.
*/
Parameter *
Parameter::make (Inkscape::XML::Node * in_repr, Inkscape::Extension::Extension * in_ext)
{
    const char * name;
    const char * type;
    const char * defaultval;
    const char * guitext;
    gchar * param_name;
    Parameter * param = NULL;

    name = in_repr->attribute("name");
    type = in_repr->attribute("type");
    guitext = in_repr->attribute("guitext");
    defaultval = sp_repr_children(in_repr)->content();
    param_name = g_strdup_printf("%s.%s", in_ext->get_id(), name);

    /* In this case we just don't have enough information */
    if (name == NULL || type == NULL) {
        return NULL;
    }

    if (!strcmp(type, "boolean")) {
        bool default_local;

        if (defaultval != NULL && !strcmp(defaultval, "TRUE")) {
            default_local = true;
        } else {
            default_local = false;
        }
        param = new ParamBool(name, guitext, in_ext, (bool)prefs_get_int_attribute(PREF_DIR, param_name, default_local));
    } else if (!strcmp(type, "int")) { 
        int default_local;
        if (defaultval != NULL) {
            default_local = atoi(defaultval);
        } else {
            default_local = 0;
        }
        param = new ParamInt(name, guitext, in_ext, prefs_get_int_attribute(PREF_DIR, param_name, (gint)default_local));
    } else if (!strcmp(type, "float")) { 
        float default_local;
        // std::cout << "Float value: " << defaultval;
        if (defaultval != NULL) {
            default_local = atof(defaultval);
        } else {
            default_local = 0.0;
        }
        param = new ParamFloat(name, guitext, in_ext, prefs_get_double_attribute(PREF_DIR, param_name, (gfloat)default_local));
        // std::cout << " after: " << param->val.t_float << std::endl;
    } else if (!strcmp(type, "string")) { 
        const gchar * temp_str;

        temp_str = prefs_get_string_attribute(PREF_DIR, param_name);
        if (temp_str == NULL)
            temp_str = defaultval;

        param = new ParamString(name, guitext, in_ext, temp_str);
    }

    g_free(param_name);
    if (param == NULL) return NULL;

    return param;
}

/** \brief  A function to set the \c _value
    \param  in   The value to set to
    \param  doc  A document that should be used to set the value.

    This function sets the internal value, but it also sets the value
    in the preferences structure.  To put it in the right place, \c PREF_DIR
    and \c pref_name() are used.
*/
bool
ParamBool::set (bool in, Inkscape::XML::Document * doc)
{
    _value = in;

    gchar * prefname = this->pref_name();
    prefs_set_int_attribute(PREF_DIR, prefname, _value == true ? 1 : 0);
    g_free(prefname);

    return _value;
}

/** \brief  A function to set the \c _value
    \param  in   The value to set to
    \param  doc  A document that should be used to set the value.

    This function sets the internal value, but it also sets the value
    in the preferences structure.  To put it in the right place, \c PREF_DIR
    and \c pref_name() are used.
*/
int
ParamInt::set (int in, Inkscape::XML::Document * doc)
{
    _value = in;

    gchar * prefname = this->pref_name();
    prefs_set_int_attribute(PREF_DIR, prefname, _value);
    g_free(prefname);

    return _value;
}

/** \brief  A function to set the \c _value
    \param  in   The value to set to
    \param  doc  A document that should be used to set the value.

    This function sets the internal value, but it also sets the value
    in the preferences structure.  To put it in the right place, \c PREF_DIR
    and \c pref_name() are used.
*/
float
ParamFloat::set (float in, Inkscape::XML::Document * doc)
{
    _value = in;

    gchar * prefname = this->pref_name();
    prefs_set_double_attribute(PREF_DIR, prefname, _value);
    g_free(prefname);

    return _value;
}

/** \brief  A function to set the \c _value
    \param  in   The value to set to
    \param  doc  A document that should be used to set the value.

    This function sets the internal value, but it also sets the value
    in the preferences structure.  To put it in the right place, \c PREF_DIR
    and \c pref_name() are used.

    To copy the data into _value the old memory must be free'd first.
    It is important to note that \c g_free handles \c NULL just fine.  Then
    the passed in value is duplicated using \c g_strdup().
*/
const gchar *
ParamString::set (const gchar * in, Inkscape::XML::Document * doc)
{
    if (in == NULL) return NULL; /* Can't have NULL string */

    g_free(_value);
    _value = g_strdup(in);

    gchar * prefname = this->pref_name();
    prefs_set_string_attribute(PREF_DIR, prefname, _value);
    g_free(prefname);

    return _value;
}

/** \brief  Wrapper to cast to the object and use it's function.  */
bool
Parameter::get_bool (const Inkscape::XML::Document * doc)
{
    ParamBool * boolpntr;
    boolpntr = dynamic_cast<ParamBool *>(this);
    if (boolpntr == NULL)
        throw Extension::param_wrong_type();
    return boolpntr->get(doc); 
}

/** \brief  Wrapper to cast to the object and use it's function.  */
int
Parameter::get_int (const Inkscape::XML::Document * doc)
{
    ParamInt * intpntr;
    intpntr = dynamic_cast<ParamInt *>(this);
    if (intpntr == NULL)
        throw Extension::param_wrong_type();
    return intpntr->get(doc); 
}

/** \brief  Wrapper to cast to the object and use it's function.  */
float
Parameter::get_float (const Inkscape::XML::Document * doc)
{
    ParamFloat * floatpntr;
    floatpntr = dynamic_cast<ParamFloat *>(this);
    if (floatpntr == NULL)
        throw Extension::param_wrong_type();
    return floatpntr->get(doc); 
}

/** \brief  Wrapper to cast to the object and use it's function.  */
const gchar *
Parameter::get_string (const Inkscape::XML::Document * doc)
{
    ParamString * stringpntr;
    stringpntr = dynamic_cast<ParamString *>(this);
    if (stringpntr == NULL)
        throw Extension::param_wrong_type();
    return stringpntr->get(doc); 
}

/** \brief  Wrapper to cast to the object and use it's function.  */
bool
Parameter::set_bool (bool in, Inkscape::XML::Document * doc)
{
    ParamBool * boolpntr;
    boolpntr = dynamic_cast<ParamBool *>(this);
    if (boolpntr == NULL)
        throw Extension::param_wrong_type();
    return boolpntr->set(in, doc); 
}

/** \brief  Wrapper to cast to the object and use it's function.  */
int
Parameter::set_int (int in, Inkscape::XML::Document * doc)
{
    ParamInt * intpntr;
    intpntr = dynamic_cast<ParamInt *>(this);
    if (intpntr == NULL)
        throw Extension::param_wrong_type();
    return intpntr->set(in, doc); 
}

/** \brief  Wrapper to cast to the object and use it's function.  */
float
Parameter::set_float (float in, Inkscape::XML::Document * doc)
{
    ParamFloat * floatpntr;
    floatpntr = dynamic_cast<ParamFloat *>(this);
    if (floatpntr == NULL)
        throw Extension::param_wrong_type();
    return floatpntr->set(in, doc); 
}

/** \brief  Wrapper to cast to the object and use it's function.  */
const gchar *
Parameter::set_string (const gchar * in, Inkscape::XML::Document * doc)
{
    ParamString * stringpntr;
    stringpntr = dynamic_cast<ParamString *>(this);
    if (stringpntr == NULL)
        throw Extension::param_wrong_type();
    return stringpntr->set(in, doc); 
}

/** \brief  Initialize the object, to do that, copy the data. */
ParamString::ParamString (const gchar * name, const gchar * guitext, Inkscape::Extension::Extension * ext, const gchar * value) :
    Parameter(name, guitext, ext), _value(NULL)
{
    _value = g_strdup(value);
}

/** \brief  Free the allocated data. */
ParamString::~ParamString(void)
{
    g_free(_value);
}

/** \brief  Oop, now that we need a parameter, we need it's name.  */
Parameter::Parameter (const gchar * name, const gchar * guitext, Inkscape::Extension::Extension * ext) :
    extension(ext), _name(NULL), _text(NULL)
{
    _name = g_strdup(name);
    if (guitext != NULL)
        _text = g_strdup(guitext);
    else
        _text = g_strdup(name);
}

/** \brief  Just free the allocated name. */
Parameter::~Parameter (void)
{
    g_free(_name);
    g_free(_text);
}

/** \brief  Build the name to write the parameter from the extension's
            ID and the name of this parameter. */
gchar *
Parameter::pref_name (void)
{
    return g_strdup_printf("%s.%s", extension->get_id(), _name);
}

/** \brief  Basically, if there is no widget pass a NULL. */
Gtk::Widget *
Parameter::get_widget (void)
{
    return NULL;
}

/** \brief  A class to make an adjustment that uses Extension params */
class ParamFloatAdjustment : public Gtk::Adjustment {
    /** The parameter to adjust */
    ParamFloat * _pref;
public:
    /** \brief  Make the adjustment using an extension and the string
                describing the parameter. */
    ParamFloatAdjustment (ParamFloat * param) :
            Gtk::Adjustment(0.0, 0.0, 10.0, 0.1), _pref(param) {
        this->set_value(_pref->get(NULL) /* \todo fix */); 
        this->signal_value_changed().connect(sigc::mem_fun(this, &ParamFloatAdjustment::val_changed));
        return;
    };

    void val_changed (void);
}; /* class ParamFloatAdjustment */

/** \brief  A function to respond to the value_changed signal from the
            adjustment.

    This function just grabs the value from the adjustment and writes
    it to the parameter.  Very simple, but yet beautiful.
*/
void
ParamFloatAdjustment::val_changed (void)
{
    // std::cout << "Value Changed to: " << this->get_value() << std::endl;
    _pref->set(this->get_value(), NULL /* \todo fix */);
    return;
}

/** \brief  A class to make an adjustment that uses Extension params */
class ParamIntAdjustment : public Gtk::Adjustment {
    /** The parameter to adjust */
    ParamInt * _pref;
public:
    /** \brief  Make the adjustment using an extension and the string
                describing the parameter. */
    ParamIntAdjustment (ParamInt * param) :
            Gtk::Adjustment(0.0, 0.0, 10.0, 1.0), _pref(param) {
        this->set_value(_pref->get(NULL) /* \todo fix */); 
        this->signal_value_changed().connect(sigc::mem_fun(this, &ParamIntAdjustment::val_changed));
        return;
    };

    void val_changed (void);
}; /* class ParamIntAdjustment */

/** \brief  A function to respond to the value_changed signal from the
            adjustment.

    This function just grabs the value from the adjustment and writes
    it to the parameter.  Very simple, but yet beautiful.
*/
void
ParamIntAdjustment::val_changed (void)
{
    // std::cout << "Value Changed to: " << this->get_value() << std::endl;
    _pref->set((int)this->get_value(), NULL /* \todo fix */);
    return;
}

Gtk::Widget *
ParamFloat::get_widget (void)
{
    Gtk::HBox * hbox = new Gtk::HBox();

    Gtk::Label * label = new Gtk::Label(_(_text), Gtk::ALIGN_LEFT);
    label->show();
    hbox->pack_start(*label, true, true);

    ParamFloatAdjustment * fadjust = new ParamFloatAdjustment(this);
    Gtk::SpinButton * spin = new Gtk::SpinButton(*fadjust, 0.1, 1);
    spin->show();
    hbox->pack_start(*spin, false, false);

    hbox->show();

    return dynamic_cast<Gtk::Widget *>(hbox);
}

Gtk::Widget *
ParamInt::get_widget (void)
{
    Gtk::HBox * hbox = new Gtk::HBox();

    Gtk::Label * label = new Gtk::Label(_(_text), Gtk::ALIGN_LEFT);
    label->show();
    hbox->pack_start(*label, true, true);

    ParamIntAdjustment * fadjust = new ParamIntAdjustment(this);
    Gtk::SpinButton * spin = new Gtk::SpinButton(*fadjust, 0.1, 1);
    spin->show();
    hbox->pack_start(*spin, false, false);

    hbox->show();

    return dynamic_cast<Gtk::Widget *>(hbox);
}

Gtk::Widget *
ParamBool::get_widget (void)
{
    Gtk::HBox * hbox = new Gtk::HBox();

    Gtk::Label * label = new Gtk::Label(_(_text), Gtk::ALIGN_LEFT);
    label->show();
    hbox->pack_start(*label, true, true);

    Gtk::CheckButton * checkbox = new Gtk::CheckButton();
    hbox->pack_start(*checkbox, false, false);

    hbox->show();

    return dynamic_cast<Gtk::Widget *>(hbox);
}



}  /* namespace Extension */
}  /* namespace Inkscape */

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
