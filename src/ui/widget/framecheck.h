// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Functions for logging timing events.
 * Copyright (C) 2022 PBS <pbs3141@gmail.com>
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */

#ifndef FRAMECHECK_H
#define FRAMECHECK_H

#include <ostream>
#include <glib.h>

namespace Inkscape {
namespace FrameCheck {

extern std::ostream &logfile();

// RAII object that logs a timing event for the duration of its lifetime.
struct Event
{
    gint64 start;
    const char *name;
    int subtype;

    Event() : start(-1) {}

    Event(const char *name, int subtype = 0) : start(g_get_monotonic_time()), name(name), subtype(subtype) {}

    Event(Event &&p)
    {
        movefrom(p);
    }

    ~Event()
    {
        finish();
    }

    Event &operator=(Event &&p)
    {
        finish();
        movefrom(p);
        return *this;
    }

    void movefrom(Event &p)
    {
        start = p.start;
        name = p.name;
        subtype = p.subtype;
        p.start = -1;
    }

    void finish()
    {
        if (start != -1) {
            logfile() << name << ' ' << start << ' ' << g_get_monotonic_time() << ' ' << subtype << '\n';
        }
    }
};

} // namespace FrameCheck
} // namespace Inkscape

#endif // FRAMECHECK_H
