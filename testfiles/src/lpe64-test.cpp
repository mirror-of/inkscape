// SPDX-License-Identifier: GPL-2.0-or-later
/** @file
 * LPE 64B tests 
 * Because some issues rounding in 32B windows we move this tests only to 64B
 *//*
 * Authors: see git history
 *
 * Copyright (C) 2020 Authors
 *
 * Released under GNU GPL version 2 or later, read the file 'COPYING' for more information
 */

#include <gtest/gtest.h>
#include <testfiles/lpespaths-test.h>
#include <src/inkscape.h>

using namespace Inkscape;
using namespace Inkscape::LivePathEffect;

class LPE64Test : public LPESPathsTest {
public:
    void run() {
        testDoc(svg);
    }
};


// A) FILE BASED TESTS
TEST_F(LPE64Test, Bendpath_mixed_0_92_5)                        { run(); }
TEST_F(LPE64Test, Bendpath_shape_1_0_2)                         { run(); }
TEST_F(LPE64Test, Bendpath_shapeClipPath_1_0_2)                 { run(); }
TEST_F(LPE64Test, Bendpath_multiGroup_1_0_2)                    { run(); }
TEST_F(LPE64Test, Bendpath_stackNested_mm_1_0_2)                { run(); }
TEST_F(LPE64Test, Bendpath_stackNested_px_1_0_2)                { run(); }
TEST_F(LPE64Test, BSpline_mixed_0_92_5)                         { run(); }
TEST_F(LPE64Test, BSpline_mm_1_0_2)                             { run(); }
TEST_F(LPE64Test, BSpline_px_1_0_2)                             { run(); }
TEST_F(LPE64Test, TaperStroke_multi_px_1_0_2)                   { run(); }
TEST_F(LPE64Test, TaperStroke_multi_mm_1_0_2)                   { run(); }
TEST_F(LPE64Test, Sketch_path_0_92_5)                           { run(); }
TEST_F(LPE64Test, Sketch_multi_px_1_0_2)                        { run(); }
TEST_F(LPE64Test, Sketch_multi_mm_1_0_2)                        { run(); }
TEST_F(LPE64Test, PowerStroke_multi_mm_1_0_2)                   { run(); }
TEST_F(LPE64Test, PowerStroke_multi_px_1_0_2)                   { run(); }
TEST_F(LPE64Test, PatternAlongPath_mixed_0_92_5)                { run(); }
TEST_F(LPE64Test, PatternAlongPath_shape_1_0_2)                 { run(); }
TEST_F(LPE64Test, PatternAlongPath_path_1_0_2)                  { run(); }
TEST_F(LPE64Test, PatternAlongPath_multiple_mm_1_0_2)           { run(); }
TEST_F(LPE64Test, PatternAlongPath_multiple_px_1_0_2)           { run(); }
TEST_F(LPE64Test, EnvelopeDeformation_multi_0_92_5)             { run(); }
TEST_F(LPE64Test, EnvelopeDeformation_multi_px_1_0_2)           { run(); }
TEST_F(LPE64Test, EnvelopeDeformation_multi_mm_1_0_2)           { run(); }

// B) CUSTOM TESTS