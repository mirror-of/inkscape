#ifndef SEEN_INKSCAPE_XML_SP_REPR_ACTION_FNS_H
#define SEEN_INKSCAPE_XML_SP_REPR_ACTION_FNS_H

namespace Inkscape {
namespace XML {
class Document;
}
}

namespace Inkscape {
namespace XML {
class Event;
}
}


void sp_repr_begin_transaction (Inkscape::XML::Document *doc);
void sp_repr_rollback (Inkscape::XML::Document *doc);
void sp_repr_commit (Inkscape::XML::Document *doc);
Inkscape::XML::Event *sp_repr_commit_undoable (Inkscape::XML::Document *doc);

void sp_repr_undo_log (Inkscape::XML::Event *log);
void sp_repr_replay_log (Inkscape::XML::Event *log);
Inkscape::XML::Event *sp_repr_coalesce_log (Inkscape::XML::Event *a, Inkscape::XML::Event *b);
void sp_repr_free_log (Inkscape::XML::Event *log);

#endif
