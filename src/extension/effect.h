/*
 * Authors:
 *   Ted Gould <ted@gould.cx>
 *
 * Copyright (C) 2002-2004 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */


#ifndef INKSCAPE_EXTENSION_EFFECT_H__
#define INKSCAPE_EXTENSION_EFFECT_H__

#include <config.h>

#include <gtk/gtkdialog.h>
#include "verbs.h"

#include "extension.h"

struct SPDocument;

namespace Inkscape {
namespace UI {
namespace View {
typedef SPView View;
};
};

namespace Extension {

class Effect : public Extension {
    static Effect * _last_effect;

    class EffectVerb : public Inkscape::Verb {
        private:
            static void perform (SPAction * action, void * mydata, void * otherdata);
            static SPActionEventVector vector;

            Effect * _effect;
        protected:
            virtual SPAction * make_action (Inkscape::UI::View::View * view);
        public:
            /** \brief Use the Verb initializer with the same parameters. */
            EffectVerb(gchar const * id,
                       gchar const * name,
                       gchar const * tip,
                       gchar const * image,
                       Effect *      effect) :
                    Verb(id, name, tip, image), _effect(effect) {
                /* No clue why, but this is required */
                this->set_default_sensitive(true);
            }
    };
    EffectVerb _verb;
public:
                 Effect  (Inkscape::XML::Node * in_repr,
                          Implementation::Implementation * in_imp);
    virtual     ~Effect  (void);
    virtual bool check                (void);
    bool         prefs   (Inkscape::UI::View::View * doc);
    void         effect  (Inkscape::UI::View::View * doc);
    Inkscape::Verb * get_verb (void) { return &_verb; };

    static Effect *  get_last_effect (void) { return _last_effect; };
    static void      set_last_effect (Effect * in_effect);
};

} }  /* namespace Inkscape, Extension */
#endif /* INKSCAPE_EXTENSION_EFFECT_H__ */

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
