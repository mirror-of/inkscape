// SPDX-License-Identifier: GPL-2.0-or-later
/** @file
 * LPE Boolean operation test
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

class LPEEllipse5ptsTest : public LPESPathsTest {};

// INKSCAPE 0.92.5

TEST_F(LPEEllipse5ptsTest, path_0_92_5)
{
   std::string svg = R""""(<?xml version="1.0" encoding="UTF-8" standalone="no"?>
<svg
   xmlns:svg="http://www.w3.org/2000/svg"
   xmlns="http://www.w3.org/2000/svg"
   xmlns:sodipodi="http://sodipodi.sourceforge.net/DTD/sodipodi-0.dtd"
   xmlns:inkscape="http://www.inkscape.org/namespaces/inkscape"
   width="210mm"
   height="297mm"
   viewBox="0 0 210 297"
   version="1.1"
   id="svg33"
   inkscape:version="0.92.5 (2060ec1f9f, 2020-04-08)">
  <defs
     id="defs27">
    <inkscape:path-effect
       effect="ellipse_5pts"
       id="path-effect38"
       is_visible="true" />
  </defs>
  <g
     inkscape:label="Layer 1"
     inkscape:groupmode="layer"
     id="layer1">
    <path
       style="fill:none;stroke:#ff0000;stroke-width:0.26458332"
       d="M 119.90087,132.05124 C 101.87877,180.1776 63.968484,210.46627 35.225875,199.7029 6.4832664,188.93952 -2.2074215,141.19995 15.81468,93.073596 33.836782,44.947238 71.747071,14.65856 100.48968,25.421939 c 28.74261,10.763379 37.4333,58.502943 19.41119,106.629301"
       id="rect01"
       inkscape:path-effect="#path-effect38"
       inkscape:original-d="M 43.089287,47.535713 77.107143,24.857142 122.46429,47.535713 v 77.107137 l -111.880957,46.1131 32.505954,-46.1131 z"
       inkscape:connector-curvature="0"
       sodipodi:nodetypes="ccccccc" />
  </g>
</svg>
)"""";
    testDoc(svg);
}


// INKSCAPE 1.0.2

TEST_F(LPEEllipse5ptsTest, ellipse_PX_1_0_2)
{
   std::string svg = R""""(<?xml version="1.0" encoding="UTF-8" standalone="no"?>
<svg
   xmlns:svg="http://www.w3.org/2000/svg"
   xmlns="http://www.w3.org/2000/svg"
   xmlns:sodipodi="http://sodipodi.sourceforge.net/DTD/sodipodi-0.dtd"
   xmlns:inkscape="http://www.inkscape.org/namespaces/inkscape"
   width="250"
   height="250"
   viewBox="0 0 250 250"
   version="1.1"
   id="svg8"
   inkscape:version="1.0.2 (e86c870879, 2021-01-15)">
  <defs
     id="defs2">
    <inkscape:path-effect
       effect="ellipse_5pts"
       id="path-effect26"
       is_visible="true"
       lpeversion="1" />
  </defs>
  <path
     id="rect01"
     style="fill:#0000ff;fill-rule:evenodd"
     d="m 202.66249,139.65753 c -12.45277,22.43137 -56.48142,21.7773 -98.34073,-1.4609 C 62.46245,114.95842 38.623746,77.935918 51.076525,55.504549 c 12.452778,-22.431369 56.481425,-21.7773 98.340735,1.460903 41.85931,23.238204 65.69801,60.260708 53.24523,82.692078"
     sodipodi:nodetypes="ccccccc"
     inkscape:path-effect="#path-effect26"
     inkscape:original-d="m 50.861622,55.90004 52.335718,-15.51685 53.60076,20.939902 36.58352,32.89488 -53.03798,58.444098 H 50.861622 Z" />
</svg>
)"""";

   testDoc(svg);
}



TEST_F(LPEEllipse5ptsTest, ellipse_MM_1_0_2)
{
   std::string svg = R""""(<?xml version="1.0" encoding="UTF-8" standalone="no"?>
<svg
   xmlns:svg="http://www.w3.org/2000/svg"
   xmlns="http://www.w3.org/2000/svg"
   xmlns:sodipodi="http://sodipodi.sourceforge.net/DTD/sodipodi-0.dtd"
   xmlns:inkscape="http://www.inkscape.org/namespaces/inkscape"
   width="250mm"
   height="250mm"
   viewBox="0 0 250 250"
   version="1.1"
   id="svg8"
   inkscape:version="1.0.2 (e86c870879, 2021-01-15)">
  <defs
     id="defs2">
    <inkscape:path-effect
       effect="ellipse_5pts"
       id="path-effect26"
       is_visible="true"
       lpeversion="1" />
  </defs>
  <path
     id="rect01"
     style="fill:#0000ff;fill-rule:evenodd"
     d="m 202.66249,139.65753 c -12.45277,22.43137 -56.48142,21.7773 -98.34073,-1.4609 C 62.46245,114.95842 38.623746,77.935918 51.076525,55.504549 c 12.452778,-22.431369 56.481425,-21.7773 98.340735,1.460903 41.85931,23.238204 65.69801,60.260708 53.24523,82.692078"
     sodipodi:nodetypes="ccccccc"
     inkscape:path-effect="#path-effect26"
     inkscape:original-d="m 50.861622,55.90004 52.335718,-15.51685 53.60076,20.939902 36.58352,32.89488 -53.03798,58.444098 H 50.861622 Z"
     transform="scale(0.5)" />
</svg>
)"""";

   testDoc(svg);
}
