// SPDX-License-Identifier: GPL-2.0-or-later
/** @file
 * LPE tests
 *//*
 * Authors: see git history
 *
 * Copyright (C) 2020 Authors
 *
 * Released under GNU GPL version 2 or later, read the file 'COPYING' for more information
 */

#include <gtest/gtest.h>
#include <testfiles/lpespaths-test.h>
#include <src/document.h>
#include <src/inkscape.h>
#include <src/live_effects/lpe-bool.h>
#include <src/object/sp-ellipse.h>
#include <src/object/sp-lpe-item.h>

using namespace Inkscape;
using namespace Inkscape::LivePathEffect;

class LPETest : public LPESPathsTest {
public:
    void run() {
        testDoc(svg);
    }
};

// A) FILE BASED TESTS
TEST_F(LPETest, Bool_multi_px_1_1)                          { run(); }
TEST_F(LPETest, Bool_multi_mm_1_1)                          { run(); }
TEST_F(LPETest, AttachPath_0_92_5_mixed)                    { run(); }
TEST_F(LPETest, AttachPath_mm_1_0_2)                        { run(); }
TEST_F(LPETest, AttachPath_px_1_0_2)                        { run(); }
TEST_F(LPETest, BoundingBox_mixed_0_92_5)                   { run(); }
TEST_F(LPETest, BoundingBox_mm_1_0_2)                       { run(); }
TEST_F(LPETest, BoundingBox_px_1_0_2)                       { run(); }
TEST_F(LPETest, CloneOriginal_mixed_0_92_5)                 { run(); }
// linked item is broken in 1.0.2 because group cliboard items, use same version of 1.1 but resaved in 1.2 to get comapat in 1.0.1 or before the group clipboard is added
TEST_F(LPETest, CloneOriginal_boken_1_0_2)                  { run(); }
TEST_F(LPETest, CloneOriginal_mixed_px_1_1)                 { run(); }
TEST_F(LPETest, CloneOriginal_mixed_mm_1_1)                 { run(); }
TEST_F(LPETest, ConstructGrid_mixed_0_92_5)                 { run(); }
TEST_F(LPETest, ConstructGrid_mm_1_0_2)                     { run(); }
TEST_F(LPETest, ConstructGrid_px_1_0_2)                     { run(); }
TEST_F(LPETest, Transform2Points_path_0_92_5)               { run(); }
TEST_F(LPETest, Transform2Points_multi_px_1_0_2)            { run(); }
TEST_F(LPETest, Transform2Points_multi_mm_1_0_2)            { run(); }
TEST_F(LPETest, VonCoch_path_0_92_5)                        { run(); }
TEST_F(LPETest, VonCoch_multi_px_1_0_2)                     { run(); }
TEST_F(LPETest, VonCoch_multi_mm_1_0_2)                     { run(); }
TEST_F(LPETest, StitchSubPaths_path_0_92_5)                 { run(); }
TEST_F(LPETest, StitchSubPaths_multi_px_1_0_2)              { run(); }
TEST_F(LPETest, StitchSubPaths_multi_mm_1_0_2)              { run(); }
TEST_F(LPETest, Spiro_mixed_0_92_5)                         { run(); }
TEST_F(LPETest, Spiro_mm_1_0_2)                             { run(); }
TEST_F(LPETest, Spiro_px_1_0_2)                             { run(); }
TEST_F(LPETest, Slice_multi_px_1_1)                         { run(); }
TEST_F(LPETest, Slice_multi_mm_1_1)                         { run(); }
TEST_F(LPETest, Simplify_path_0_92_5)                       { run(); }
TEST_F(LPETest, Simplify_multi_px_1_0_2)                    { run(); }
TEST_F(LPETest, Simplify_multi_mm_1_0_2)                    { run(); }
TEST_F(LPETest, ShowHandles_path_0_92_5)                    { run(); }
TEST_F(LPETest, ShowHandles_multi_px_1_0_2)                 { run(); }
TEST_F(LPETest, ShowHandles_multi_mm_1_0_2)                 { run(); }
TEST_F(LPETest, Ruler_path_0_92_5)                          { run(); }
TEST_F(LPETest, Ruler_multi_px_1_0_2)                       { run(); }
TEST_F(LPETest, Ruler_multi_mm_1_0_2)                       { run(); }
TEST_F(LPETest, RoughHatches_path_0_92_5)                   { run(); }
TEST_F(LPETest, RoughHatches_multi_px_1_0_2)                { run(); }
TEST_F(LPETest, RoughHatches_multi_mm_1_0_2)                { run(); }
// Rougen Test till 1.1 fail because wrong implementation of rand on the LPE
TEST_F(LPETest, Roughen_path_1_1)                           { run(); }
TEST_F(LPETest, EllipseFromPoints_multi_px_1_0_2)           { run(); }
TEST_F(LPETest, EllipseFromPoints_multi_mm_1_0_2)           { run(); }
TEST_F(LPETest, PowerMask_multi_px_1_0_2)                   { run(); }
TEST_F(LPETest, PowerMask_multi_mm_1_0_2)                   { run(); }
TEST_F(LPETest, PowerClip_multi_px_1_0_2)                   { run(); }
TEST_F(LPETest, PowerClip_multi_mm_1_0_2)                   { run(); }
TEST_F(LPETest, PerspectiveEnvelope_mixed_0_92_5)           { run(); }
TEST_F(LPETest, PerspectiveEnvelope_multi_mm_1_0_2)         { run(); }
TEST_F(LPETest, PerspectiveEnvelope_multi_px_1_0_2)         { run(); }
TEST_F(LPETest, Offset_multi_px_1_0_2)                      { run(); }
TEST_F(LPETest, Offset_multi_mm_1_0_2)                      { run(); }
TEST_F(LPETest, Offset_multi_px_1_1)                        { run(); }
TEST_F(LPETest, MirrorSymmetry_path_0_92_5)                 { run(); }
TEST_F(LPETest, MirrorSymmetry_multi_px_1_0_2)              { run(); }
TEST_F(LPETest, MirrorSymmetry_multi_mm_1_0_2)              { run(); }
TEST_F(LPETest, MeasureSegments_multi_px_1_0_2)             { run(); }
TEST_F(LPETest, MeasureSegments_multi_mm_1_0_2)             { run(); }
TEST_F(LPETest, Lattice2_path_0_92_5)                       { run(); }
TEST_F(LPETest, Lattice2_multi_px_1_0_2)                    { run(); }
TEST_F(LPETest, Lattice2_multi_mm_1_0_2)                    { run(); }
TEST_F(LPETest, Knot_path_0_92_5)                           { run(); }
TEST_F(LPETest, Knot_multi_px_1_0_2)                        { run(); }
TEST_F(LPETest, Knot_multi_mm_1_0_2)                        { run(); }
TEST_F(LPETest, JoinType_multi_px_1_0_2)                    { run(); }
TEST_F(LPETest, JoinType_multi_mm_1_0_2)                    { run(); }
TEST_F(LPETest, Interpolate_path_0_92_5)                    { run(); }
TEST_F(LPETest, Interpolate_multi_px_1_0_2)                 { run(); }
TEST_F(LPETest, Interpolate_multi_mm_1_0_2)                 { run(); }
TEST_F(LPETest, InterpolatePoints_path_0_92_5)              { run(); }
TEST_F(LPETest, InterpolatePoints_multi_px_1_0_2)           { run(); }
TEST_F(LPETest, InterpolatePoints_multi_mm_1_0_2)           { run(); }
TEST_F(LPETest, Gears_path_0_92_5)                          { run(); }
TEST_F(LPETest, Gears_multi_px_1_0_2)                       { run(); }
TEST_F(LPETest, Gears_multi_mm_1_0_2)                       { run(); }
TEST_F(LPETest, FilletChamfer_multi_px_1_0_2)               { run(); }
TEST_F(LPETest, FilletChamfer_multi_mm_1_0_2)               { run(); }
// NEED to test on 0.92 no working one here (gnome 40)
// TEST_F(LPETest, FillBetweenStrokes_path_0_92_5)          { run(); }
TEST_F(LPETest, FillBetweenStrokes_path_multi_px_1_0_2)     { run(); }
TEST_F(LPETest, FillBetweenStrokes_path_multi_mm_1_0_2)     { run(); }
// NEED to test on 0.92 no working one here (gnome 40)
// TEST_F(LPETest, FillBetweenMany_multi_0_92_5)               { run(); }
TEST_F(LPETest, FillBetweenMany_multi_px_1_0_2)             { run(); }
TEST_F(LPETest, FillBetweenMany_multi_mm_1_0_2)             { run(); }
TEST_F(LPETest, Ellipse5pts_path_0_92_5)                    { run(); }
TEST_F(LPETest, Ellipse5pts_ellipse_px_1_0_2)               { run(); }
TEST_F(LPETest, Ellipse5pts_ellipse_mm_1_0_2)               { run(); }
TEST_F(LPETest, DashedStroke_multi_px_1_0_2)                { run(); }
TEST_F(LPETest, DashedStroke_multi_mm_1_0_2)                { run(); }
TEST_F(LPETest, RotateCopies_multi_mm_1_0_2)                { run(); }
TEST_F(LPETest, RotateCopies_multi_px_1_0_2)                { run(); }
// B) CUSTOM TESTS
// BOOL LPE
TEST_F(LPETest, Bool_canBeApplyedToNonSiblingPaths)
{
    std::string svg("\
<svg width='100' height='100'\
  xmlns:sodipodi='http://sodipodi.sourceforge.net/DTD/sodipodi-0.dtd'\
  xmlns:inkscape='http://www.inkscape.org/namespaces/inkscape'>\
  <defs>\
    <inkscape:path-effect\
      id='path-effect1'\
      effect='bool_op'\
      operation='diff'\
      operand-path='#circle1'\
      lpeversion='1'\
      hide-linked='true' />\
  </defs>\
  <path id='rect1'\
    inkscape:path-effect='#path-effect1'\
    sodipodi:type='rect'\
    width='100' height='100' fill='#ff0000' />\
  <g id='group1'>\
    <circle id='circle1'\
      r='40' cy='50' cx='50' fill='#ffffff' style='display:inline'/>\
  </g>\
</svg>");

    SPDocument *doc = SPDocument::createNewDocFromMem(svg.c_str(), svg.size(), true);
    doc->ensureUpToDate();

    auto lpe_item = dynamic_cast<SPLPEItem *>(doc->getObjectById("rect1"));
    ASSERT_TRUE(lpe_item != nullptr);

    auto lpe_bool_op_effect = dynamic_cast<LPEBool *>(lpe_item->getPathEffectOfType(EffectType::BOOL_OP));
    ASSERT_TRUE(lpe_bool_op_effect != nullptr);

    auto operand_path = lpe_bool_op_effect->getParameter("operand-path")->param_getSVGValue();
    auto circle = dynamic_cast<SPGenericEllipse *>(doc->getObjectById(operand_path.substr(1)));
    ASSERT_TRUE(circle != nullptr);
}