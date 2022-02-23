// SPDX-License-Identifier: GPL-2.0-or-later
/** @file
 * LPE test file wrapper
 *//*
 * Authors: see git history
 *
 * Copyright (C) 2020 Authors
 *
 * Released under GNU GPL version 2 or later, read the file 'COPYING' for more information
 */

#include <gtest/gtest.h>
#include <src/file.h>
#include <src/helper-fns.h>
#include <src/inkscape.h>
#include <src/object/sp-root.h>
#include <src/svg/svg.h>
#include "src/extension/init.h"

#include <2geom/pathvector.h>

using namespace Inkscape;

/* This class allow test LPE's. To make possible in latest release of Inkscape
 * LPE is not updated on load (if in the future any do we must take account) so we load
 * a svg, get all "d" attribute from paths, shapes...
 * Update all path effects with root object and check equality of paths.
 * We use some helpers inside the SVG document to test:
 * inkscape:test-threshold="0.1" can be global using in root element or per item
 * inkscape:test-ignore="1" ignore this element from tests
 * Question: Maybe is better store SVG as files instead inline CPP files, there is a 
 * 1.2 started MR, I can't finish without too much work than a cmake advanced user
 */

class LPESPathsTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        // setup hidden dependency
        Application::create(false);
        Inkscape::Extension::init();
        const testing::TestInfo* const test_info =
        testing::UnitTest::GetInstance()->current_test_info();
        svg = test_info->file();;
#ifdef INKSCAPE_TESTS_DIR
        svg = INKSCAPE_TESTS_DIR;
#else
        size_t pos = svg.find("lpespaths-test.h");
        svg.erase(pos);
#endif
        svg += "/lpe_tests/"; // gitlab use this separator
        /* svg += test_info->test_suite_name(); */
        svg += test_info->name();
        svg += ".svg";
    }

    void pathCompare(const gchar *a, const gchar *b, const gchar *id, double precission = 0.001) 
    {
        failed.push_back(id);
        Geom::PathVector apv = sp_svg_read_pathv(a);
        Geom::PathVector bpv = sp_svg_read_pathv(b);
        size_t totala = apv.curveCount();
        size_t totalb = bpv.curveCount();
        ASSERT_TRUE(totala == totalb);
        std::vector<Geom::Coord> pos;
        // find initial
        size_t initial = 0;
        for (size_t i = 0; i < totala; i++) {
        Geom::Point pointa = apv.pointAt(0.0);
        Geom::Point pointb = bpv.pointAt(float(i));
        if (Geom::are_near(pointa[Geom::X], pointb[Geom::X], precission) &&
            Geom::are_near(pointa[Geom::Y], pointb[Geom::Y], precission)) 
        {
            initial = i;
            break;
        }
        }
        if (initial != 0 && initial == totala) {
        std::cout << "[ WARN     ] Curve reversed. We not block here. We reverse the path and test node positions on reverse" << std::endl;
        bpv.reverse();
        } else if (initial != 0) {
        std::cout << "[ WARN     ] Different starting node. We not block here. We gap the origin to " << initial << " de " << totala << " and test with the pathvector reindexed" << std::endl;
        }
        for (size_t i = 0; i < apv.curveCount(); i++) {
        if (initial >= totala) {
            initial = 0;
        }
        Geom::Point pointa = apv.pointAt(float(i)+0.2);
        Geom::Point pointb = bpv.pointAt(float(initial)+0.2);
        Geom::Point pointc = apv.pointAt(float(i)+0.4);
        Geom::Point pointd = bpv.pointAt(float(initial)+0.4);
        Geom::Point pointe = apv.pointAt(float(i));
        Geom::Point pointf = bpv.pointAt(float(initial));
        ASSERT_NEAR(pointa[Geom::X], pointb[Geom::X], precission);
        ASSERT_NEAR(pointa[Geom::Y], pointb[Geom::Y], precission);
        ASSERT_NEAR(pointc[Geom::X], pointd[Geom::X], precission);
        ASSERT_NEAR(pointc[Geom::Y], pointd[Geom::Y], precission);
        ASSERT_NEAR(pointe[Geom::X], pointf[Geom::X], precission);
        ASSERT_NEAR(pointe[Geom::Y], pointf[Geom::Y], precission);
        initial++;
        }
        failed.pop_back();
    }

    void TearDown( ) override
    { 
        Glib::ustring ids = "";
        for (auto fail : failed) {
            if (ids != "") {
                ids += ",";
            }
            ids += fail;
        }
        if (ids != "") {
            FAIL() << "[FAILED IDS] " << ids; 
        }
    }

    // you can override custom threshold from svg file using in 
    // root svg from global and override with per shape "inkscape:test-threshold"
    void testDoc(std::string file) 
    {
        double precission = 0.001;
        SPDocument *doc = nullptr;
        doc = SPDocument::createNewDoc(file.c_str(), false);
        ASSERT_TRUE(doc != nullptr);
        std::vector<SPObject *> objs;
        std::vector<const gchar *> ids;
        std::vector<const gchar *> ds;
        for (auto obj : doc->getObjectsByElement("path")) {
            objs.push_back(obj);
        }
        for (auto obj : doc->getObjectsByElement("ellipse")) {
            objs.push_back(obj);
        }
        for (auto obj : doc->getObjectsByElement("circle")) {
            objs.push_back(obj);
        }
        for (auto obj : doc->getObjectsByElement("rect")) {
            objs.push_back(obj);
        }
        for (auto obj : objs) {
            if (!g_strcmp0(obj->getAttribute("d"), "M 0,0")) {
               if (obj->getAttribute("id")) {
                  std::cout << "[ WARN     ] Item with id:" << obj->getAttribute("id") << " has empty path data" << std::endl;
               }
            } else if (obj->getAttribute("d") && obj->getAttribute("id"))  {
                ds.push_back(obj->getAttribute("d"));
                ids.push_back(obj->getAttribute("id"));
            }
        }
        sp_file_fix_lpe(doc);
        doc->ensureUpToDate();
        SPLPEItem *lpeitem = dynamic_cast<SPLPEItem *>(doc->getRoot());
        sp_lpe_item_update_patheffect(lpeitem, true, true);
        // we need to double update because clippaths or mask
        if (doc->getObjectsByElement("clipPath").size() || doc->getObjectsByElement("mask").size()) {
            sp_lpe_item_update_patheffect(lpeitem, true, true);
        }
        if (lpeitem->getAttribute("inkscape:test-threshold")) {
            precission = helperfns_read_number(lpeitem->getAttribute("inkscape:test-threshold"));
        }
        size_t index = 0;
        for (auto id : ids) {
            SPObject *obj = doc->getObjectById(id);
            if (obj) {
               if (obj->getAttribute("inkscape:test-threshold")) {
                  precission = helperfns_read_number(obj->getAttribute("inkscape:test-threshold"));
               }
               if (!obj->getAttribute("inkscape:test-ignore")) {
                  pathCompare(ds[index], obj->getAttribute("d"), obj->getAttribute("id"), precission);
               } else {
                  std::cout << "[ WARN     ] Item with id:" << obj->getAttribute("id") << " ignored by inkscape:test-ignore" << std::endl;
               }
            } else {
               std::cout << "[ WARN     ] Item with id:" << id << " removed on apply LPE" << std::endl;
            }
            index++;
        }
    }
    std::string svg = ""; 
    std::vector< const gchar *> failed;
};

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
