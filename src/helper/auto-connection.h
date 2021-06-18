// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef SEEN_AUTO_CONNECTION_H
#define SEEN_AUTO_CONNECTION_H

#include <sigc++/connection.h>

namespace Inkscape {

// class to simplify re-subsribing to connections; automates disconnecting

class auto_connection {
public:
    auto_connection(const sigc::connection& c): _connection(c) {}

    auto_connection() = default;

    ~auto_connection() {
        _connection.disconnect();
    }

    auto_connection(const auto_connection&) = delete;
    auto_connection& operator = (const auto_connection&) = delete;

    // re-assign
    auto_connection& operator = (const sigc::connection& c) {
        _connection.disconnect();
        _connection = c;
        return *this;
    }

    void disconnect() {
        *this = sigc::connection();
    }

private:
    sigc::connection _connection;
};

} // namespace

#endif
