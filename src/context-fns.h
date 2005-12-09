struct SPDesktop;

namespace Inkscape
{

class MessageContext;
class MessageStack;

extern bool have_viable_layer(SPDesktop *desktop, MessageContext *message);
extern bool have_viable_layer(SPDesktop *desktop, MessageStack *message);

}

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
