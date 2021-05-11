// SPDX-License-Identifier: GPL-2.0-or-later

/** @file
 * @brief
 *
 * Authors: see git history
 *   Osama Ahmad
 *
 * Copyright (c) 2021 Osama Ahmad, Authors
 *
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */

#include <2geom/point.h>

#include "node.h"
#include "svg/stringstream.h"
#include "svg/css-ostringstream.h"
#include "svg/svg-length.h"

namespace Inkscape{
namespace XML {

void Node::setAttribute(Util::const_char_ptr key, Util::const_char_ptr value)
{
    this->setAttributeImpl(key.data(), value.data());
}

bool Node::getAttributeBoolean(Util::const_char_ptr key, bool default_value) const
{
    auto v = this->attribute(key.data());
    if (v == nullptr) {
        return default_value;
    }

    if (!g_ascii_strcasecmp(v, "true") ||
        !g_ascii_strcasecmp(v, "yes") ||
        !g_ascii_strcasecmp(v, "y") ||
        (atoi(v) != 0))
    {
        return true;
    } else {
        return false;
    }
}

int Node::getAttributeInt(Util::const_char_ptr key, int default_value) const
{
    auto v = this->attribute(key.data());
    if (v == nullptr) {
        return default_value;
    }
    return atoi(v);
}

double Node::getAttributeDouble(Util::const_char_ptr key, double default_value) const
{
    auto v = this->attribute(key.data());
    if (v == nullptr) {
        return default_value;
    }

    return g_ascii_strtod(v, nullptr);
}

bool Node::setAttributeBoolean(Util::const_char_ptr key, bool val)
{
    this->setAttribute(key, (val) ? "true" : "false");
    return true;
}

bool Node::setAttributeInt(Util::const_char_ptr key, int val)
{
    gchar c[32];

    g_snprintf(c, 32, "%d", val);

    this->setAttribute(key, c);
    return true;
}

bool Node::setAttributeCssDouble(Util::const_char_ptr key, double val)
{
    Inkscape::CSSOStringStream os;
    os << val;

    this->setAttribute(key, os.str());
    return true;
}

bool Node::setAttributeSvgDouble(Util::const_char_ptr key, double val)
{
    g_return_val_if_fail(val == val, false); // tests for nan

    Inkscape::SVGOStringStream os;
    os << val;

    this->setAttribute(key, os.str());
    return true;
}

bool Node::setAttributeSvgNonDefaultDouble(Util::const_char_ptr key, double val, double default_value)
{
    if (val == default_value) {
        this->removeAttribute(key);
        return true;
    }
    return this->setAttributeSvgDouble(key, val);
}

bool Node::setAttributeSvgLength(Util::const_char_ptr key, SVGLength const &val)
{
    this->setAttribute(key, val.write());
    return true;
}

bool Node::setAttributePoint(Util::const_char_ptr key, Geom::Point const &val)
{
    Inkscape::SVGOStringStream os;
    os << val[Geom::X] << "," << val[Geom::Y];

    this->setAttribute(key, os.str());
    return true;
}

Geom::Point Node::getAttributePoint(Util::const_char_ptr key, Geom::Point default_value) const
{
    auto v = this->attribute(key.data());
    if (v == nullptr) {
        return default_value;
    }

    gchar **strarray = g_strsplit(v, ",", 2);

    if (strarray && strarray[0] && strarray[1]) {
        double newx, newy;
        newx = g_ascii_strtod(strarray[0], nullptr);
        newy = g_ascii_strtod(strarray[1], nullptr);
        g_strfreev(strarray);
        return Geom::Point(newx, newy);
    }

    g_strfreev(strarray);
    return default_value;
}

void Node::setAttributeOrRemoveIfEmpty(Inkscape::Util::const_char_ptr key, Inkscape::Util::const_char_ptr value)
{
    this->setAttributeImpl(key.data(), (value.data() == nullptr || value.data()[0] == '\0') ? nullptr : value.data());
}

} // namespace XML
} // namespace Inkscape