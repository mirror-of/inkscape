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

class LPEBoundingBoxTest : public LPESPathsTest {};

// INKSCAPE 0.92.5

TEST_F(LPEBoundingBoxTest, mixed_0_92_5)
{
   std::string svg = R""""(<?xml version="1.0" encoding="UTF-8" standalone="no"?>
<svg
   xmlns:svg="http://www.w3.org/2000/svg"
   xmlns="http://www.w3.org/2000/svg"
   xmlns:sodipodi="http://sodipodi.sourceforge.net/DTD/sodipodi-0.dtd"
   xmlns:inkscape="http://www.inkscape.org/namespaces/inkscape"
   width="250mm"
   height="250mm"
   viewBox="0 0 250 250.00001"
   version="1.1"
   id="svg8"
   inkscape:version="0.92.5 (2060ec1f9f, 2020-04-08)">
  <defs
     id="defs2">
    <inkscape:path-effect
       effect="bounding_box"
       id="path-effect39"
       is_visible="true"
       linkedpath="#rect41"
       visualbounds="false" />
  </defs>
  <g id="t" transform="scale(0.445)">
  <path
     style="fill:#00ff00;stroke:#ff0000;stroke-width:0.26458332"
     d="M 89.540131,102.43796 H 211.6403 v 136.481 H 89.540131 Z"
     id="rect01"
     inkscape:path-effect="#path-effect39"
     inkscape:original-d="m 55.50008,49.459705 h 50.32008 V 88.679764 H 55.50008 Z" />
  <path
     style="fill:none;stroke:#ff0000;stroke-width:0.26458332"
     d="m 89.540131,132.33983 c 39.825569,-5.82992 71.760559,-64.263684 122.100169,0 v 86.58012 c -41.85049,31.06173 -82.21035,21.87631 -122.100169,0 z"
     id="rect41"
     inkscape:connector-curvature="0"
     sodipodi:nodetypes="ccccc" />
     </g>
</svg>
)"""";

   testDoc(svg);
}


// INKSCAPE 1.0.2

TEST_F(LPEBoundingBoxTest, bbox_MM_1_0_2)
{
   std::string svg = R""""(<?xml version="1.0" encoding="UTF-8" standalone="no"?>
<svg
   xmlns:svg="http://www.w3.org/2000/svg"
   xmlns="http://www.w3.org/2000/svg"
   xmlns:sodipodi="http://sodipodi.sourceforge.net/DTD/sodipodi-0.dtd"
   xmlns:inkscape="http://www.inkscape.org/namespaces/inkscape"
   width="250mm"
   height="250mm"
   viewBox="0 0 250 250.00001"
   version="1.1"
   id="svg8"
   inkscape:version="1.0.2 (e86c870879, 2021-01-15)">
  <defs
     id="defs2">
    <inkscape:path-effect
       effect="bounding_box"
       id="path-effect39"
       is_visible="true"
       lpeversion="1"
       linkedpath="#rect41"
       visualbounds="false" />
  </defs>
  <g id="t" transform="scale(0.445)">
  <path
     style="fill:#00ff00;stroke:#ff0000;stroke-width:0.26458332"
     d="M 89.540131,102.43796 H 211.6403 v 136.481 H 89.540131 Z"
     id="rect01"
     inkscape:path-effect="#path-effect39"
     inkscape:original-d="m 55.50008,49.459705 h 50.32008 V 88.679764 H 55.50008 Z" />
  <path
     style="fill:none;stroke:#ff0000;stroke-width:0.26458332"
     d="m 89.540131,132.33983 c 39.825569,-5.82992 71.760559,-64.263684 122.100169,0 v 86.58012 c -41.85049,31.06173 -82.21035,21.87631 -122.100169,0 z"
     id="rect41"
     inkscape:connector-curvature="0"
     sodipodi:nodetypes="ccccc" />
     </g>
</svg>
)"""";

   testDoc(svg);
}

// INKSCAPE 1.0.2

TEST_F(LPEBoundingBoxTest, bbox_PX_1_0_2)
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
       effect="bounding_box"
       id="path-effect39"
       is_visible="true"
       linkedpath="#rect41"
       visualbounds="false"
       lpeversion="1" />
  </defs>
  <g id="t" transform="scale(0.445)">
  <path
     style="fill:#00ff00;stroke:#ff0000;stroke-width:0.264583"
     d="M 89.540131,102.43796 H 211.6403 v 136.481 H 89.540131 Z"
     id="rect01"
     inkscape:path-effect="#path-effect39"
     inkscape:original-d="m 55.50008,49.459705 h 50.32008 V 88.679764 H 55.50008 Z" />
  <path
     style="fill:none;stroke:#ff0000;stroke-width:0.264583"
     d="m 89.540131,132.33983 c 39.825569,-5.82992 71.760559,-64.263684 122.100169,0 v 86.58012 c -41.85049,31.06173 -82.21035,21.87631 -122.100169,0 z"
     id="rect41"
     inkscape:connector-curvature="0"
     sodipodi:nodetypes="ccccc" />
     </g>
</svg>
)"""";

   testDoc(svg);
}