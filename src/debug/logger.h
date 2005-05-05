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

#ifndef SEEN_INKSCAPE_DEBUG_LOGGER_H
#define SEEN_INKSCAPE_DEBUG_LOGGER_H

namespace Inkscape {

namespace Debug {

class Event;

class Logger {
public:
    static void init();

    template <typename EventType>
    inline static void start() {
        if (_enabled) {
            _start(EventType());
        }
    }

    template <typename EventType, typename A>
    inline static void start(A const &a) {
        if (_enabled) {
            _start(EventType(a));
        }
    }

    template <typename EventType, typename A, typename B>
    inline static void start(A const &a, B const &b) {
        if (_enabled) {
            _start(EventType(a, b));
        }
    }

    template <typename EventType, typename A, typename B, typename C>
    inline static void start(A const &a, B const &b, C const &c) {
        if (_enabled) {
            _start(EventType(a, b, c));
        }
    }

    template <typename EventType, typename A, typename B,
                                  typename C, typename D>
    inline static void start(A const &a, B const &b, C const &c, D const &d) {
        if (_enabled) {
            _start(EventType(a, b, c, d));
        }
    }

    template <typename EventType, typename A, typename B, typename C,
                                  typename D, typename E>
    inline static void start(A const &a, B const &b, C const &c,
                              D const &d, E const &e)
    {
        if (_enabled) {
            _start(EventType(a, b, c, d, e));
        }
    }

    template <typename EventType, typename A, typename B, typename C,
                                  typename D, typename E, typename F>
    inline static void start(A const &a, B const &b, C const &c,
                              D const &d, E const &e, F const &f)
    {
        if (_enabled) {
            _start(EventType(a, b, c, d, e, f));
        }
    }

    template <typename EventType, typename A, typename B, typename C,
                                  typename D, typename E, typename F,
                                  typename G>
    inline static void start(A const &a, B const &b, C const &c, D const &d,
                              E const &e, F const &f, G const &g)
    {
        if (_enabled) {
            _start(EventType(a, b, c, d, e, f, g));
        }
    }

    template <typename EventType, typename A, typename B, typename C,
                                  typename D, typename E, typename F,
                                  typename G, typename H>
    inline static void start(A const &a, B const &b, C const &c, D const &d,
                              E const &e, F const &f, G const &g, H const &h)
    {
        if (_enabled) {
            _start(EventType(a, b, c, d, e, f, g, h));
        }
    }

    inline static void finish() {
        if (_enabled) {
            _finish();
        }
    }

    static void shutdown();

private:
    static bool _enabled;

    static void _start(Event const &event);
    static void _finish();
};

}

}

#endif
/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=c++:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
