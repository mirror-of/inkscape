// SPDX-License-Identifier: GPL-2.0-or-later
#include <fstream>
#include <iostream>
#include <boost/filesystem.hpp> // Using boost::filesystem instead of std::filesystem due to broken C++17 on MacOS.
#include "framecheck.h"
namespace fs = boost::filesystem;

namespace Inkscape {
namespace FrameCheck {

std::ostream &logfile()
{
    static std::ofstream f;
    
    if (!f.is_open()) {
        try {
            auto path = fs::temp_directory_path() / "framecheck.txt";
            auto mode = std::ios_base::out | std::ios_base::app | std::ios_base::binary;
            f.open(path.string(), mode);
        } catch (...) {
            std::cerr << "failed to create framecheck logfile" << std::endl;
        }
    }
    
    return f;
}

} // namespace FrameCheck
} // namespace Inkscape
