/*
 * Inkscape::Debug::Logger - debug logging facility
 *
 * Authors:
 *   MenTaLguY <mental@rydia.net>
 *
 * Copyright (C) 2005 MenTaLguY
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <cstdlib>
#include <fstream>
#include <cstring>
#include <vector>
#include "debug/logger.h"
#include "debug/event.h"
#include "debug/simple-event.h"
#include "gc-alloc.h"

namespace Inkscape {

namespace Debug {

bool Logger::_enabled=false;

namespace {

static void write_escaped_value(std::ostream &os, Util::SharedCStringPtr value) {
    for ( char const *current=value ; *current ; ++current ) {
        switch (*current) {
        case '&':
            os << "&amp;";
            break;
        case '"':
            os << "&quot;";
            break;
        case '\'':
            os << "&apos;";
            break;
        case '<':
            os << "&lt;";
            break;
        case '>':
            os << "&gt;";
            break;
        default:
            os.put(*current);
        }
    }
}

static void write_indent(std::ostream &os, unsigned depth) {
    for ( unsigned i = 0 ; i < depth ; i++ ) {
        os.write("  ", 2);
    }
}

static std::ofstream log_stream;
static bool empty_tag=false;
static std::vector<Util::SharedCStringPtr, GC::Alloc<Util::SharedCStringPtr, GC::MANUAL> > tag_stack;

static void do_shutdown() {
    Debug::Logger::shutdown();
}

}

class SessionEvent : public SimpleEvent {
public:
    SessionEvent() : SimpleEvent(Util::SharedCStringPtr::coerce("session")) {}
};

void Logger::init() {
    if (!_enabled) {
        char const *log_filename=std::getenv("INKSCAPE_DEBUG_LOG");
        if (log_filename) {
            log_stream.open(log_filename);
            if (log_stream.is_open()) {
                log_stream << "<?xml version=\"1.0\"?>\n";
                log_stream.flush();
                _enabled = true;
                start<SessionEvent>();
                std::atexit(&do_shutdown);
            }
        }
    }
}

void Logger::_start(Event const &event) {
    Util::SharedCStringPtr name=event.name();

    if (empty_tag) {
        log_stream << ">\n";
    }

    write_indent(log_stream, tag_stack.size());

    log_stream << "<" << name.cString();

    unsigned property_count=event.propertyCount();
    for ( unsigned i = 0 ; i < property_count ; i++ ) {
        Event::PropertyPair property=event.property(i);
        log_stream << " " << property.name.cString() << "=\"";
        write_escaped_value(log_stream, property.value);
        log_stream << "\"";
    }

    log_stream.flush();

    tag_stack.push_back(name);
    empty_tag = true;
}

void Logger::_finish() {
    if (empty_tag) {
        log_stream << "/>\n";
    } else {
        write_indent(log_stream, tag_stack.size() - 1);
        log_stream << "</" << tag_stack.back().cString() << ">\n";
    }

    log_stream.flush();

    tag_stack.pop_back();
    empty_tag = false;
}

void Logger::shutdown() {
    if (_enabled) {
        while (!tag_stack.empty()) {
            finish();
        }
    }
}

}

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
