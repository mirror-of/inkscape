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

class LPEConstructGridTest : public LPESPathsTest {};

// INKSCAPE 0.92.5

TEST_F(LPEConstructGridTest, mixed_0_92_5)
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
   id="svg8"
   inkscape:version="0.92.5 (2060ec1f9f, 2020-04-08)"
   inkscape:test-threshold="0.2">
  <defs
     id="defs2">
    <inkscape:path-effect
       effect="construct_grid"
       id="path-effect21"
       is_visible="true"
       nr_x="5"
       nr_y="5" />
    <inkscape:path-effect
       effect="construct_grid"
       id="path-effect19"
       is_visible="true"
       nr_x="5"
       nr_y="5" />
  </defs>
  <g
     inkscape:label="Layer 1"
     inkscape:groupmode="layer"
     id="layer1">
    <path
       sodipodi:type="star"
       style="fill:none;stroke:#ff0000;stroke-width:0.48932359"
       id="path02"
       sodipodi:sides="5"
       sodipodi:cx="117.38959"
       sodipodi:cy="114.68728"
       sodipodi:r1="98.7099"
       sodipodi:r2="49.35495"
       sodipodi:arg1="0.65483207"
       sodipodi:arg2="1.2831506"
       inkscape:flatsided="false"
       inkscape:rounded="0"
       inkscape:randomized="0"
       d="m 131.39136,162.01445 321.45002,63.94864 M 84.408507,207.72433 405.85852,271.67297 M 37.42565,253.4342 358.87566,317.38284 M -9.5572058,299.14408 311.89281,363.09272 M -56.540062,344.85396 264.90995,408.8026 m -368.43287,-18.23877 321.45001,63.94864 M 131.39136,162.01445 -103.52292,390.56383 M 195.68137,174.80418 -39.232916,403.35356 M 259.97137,187.59391 25.057087,416.14329 M 324.26137,200.38364 89.347089,428.93302 M 388.55137,213.17336 153.63709,441.72274 M 452.84138,225.96309 217.92709,454.51247"
       inkscape:transform-center-x="8.6535807"
       inkscape:transform-center-y="-0.80865374"
       inkscape:path-effect="#path-effect21"
       transform="scale(0.5)" />
    <path
       style="fill:none;stroke:#ff0000;stroke-width:0.467675"
       id="path01"
       inkscape:path-effect="#path-effect19"
       sodipodi:type="arc"
       sodipodi:cx="146.65475"
       sodipodi:cy="53.828335"
       sodipodi:rx="46.767502"
       sodipodi:ry="33.405357"
       d="M 146.65475,87.233692 380.49226,-79.793095 M 99.887253,53.828335 333.72476,-113.19845 M 53.119751,20.422977 286.95726,-146.60381 M 6.3522491,-12.98238 240.18976,-180.00917 M -40.415253,-46.387737 193.42226,-213.41452 M -87.182755,-79.793095 146.65475,-246.81988 m 0,334.053572 L -87.182755,-79.793095 M 193.42226,53.828335 -40.415253,-113.19845 M 240.18976,20.422977 6.3522491,-146.60381 M 286.95726,-12.98238 53.119751,-180.00917 M 333.72476,-46.387737 99.887253,-213.41452 M 380.49226,-79.793095 146.65475,-246.81988" />
  </g>
</svg>
)"""";

   testDoc(svg);
}

// INKSCAPE 1.0.2

TEST_F(LPEConstructGridTest, constructgrid_MM_1_0_2)
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
   inkscape:version="1.0.2 (e86c870879, 2021-01-15)"
   inkscape:test-threshold="0.2">
  <defs
     id="defs2">
    <inkscape:path-effect
       effect="construct_grid"
       id="path-effect12"
       is_visible="true"
       lpeversion="1"
       nr_x="5"
       nr_y="5" />
    <inkscape:path-effect
       effect="construct_grid"
       id="path-effect16"
       is_visible="true"
       lpeversion="1"
       nr_x="5"
       nr_y="5" />
  </defs>
  <path
     style="fill:#ff0000;fill-rule:evenodd;stroke:#ff8080;stroke-width:0.264583"
     id="path02"
     inkscape:path-effect="#path-effect12"
     sodipodi:type="arc"
     sodipodi:cx="186.84631"
     sodipodi:cy="191.16818"
     sodipodi:rx="46.695065"
     sodipodi:ry="35.03508"
     d="M 186.84631,226.20326 420.32164,51.027863 M 140.15125,191.16818 373.62657,15.992783 M 93.456184,156.1331 326.93151,-19.042297 M 46.76112,121.09802 280.23644,-54.077377 M 0.0660553,86.062943 233.54138,-89.112457 M -46.629009,51.027863 186.84631,-124.14754 m 0,350.3508 L -46.629009,51.027863 M 233.54138,191.16818 0.0660553,15.992783 M 280.23644,156.1331 46.76112,-19.042297 M 326.93151,121.09802 93.456184,-54.077377 M 373.62657,86.062943 140.15125,-89.112457 M 420.32164,51.027863 186.84631,-124.14754" />
  <path
     sodipodi:type="star"
     style="stroke:#ff8080;stroke-width:0.264583"
     id="path01"
     sodipodi:sides="5"
     sodipodi:cx="36.039883"
     sodipodi:cy="161.52631"
     sodipodi:r1="36.572998"
     sodipodi:r2="12.763976"
     sodipodi:arg1="0.78714885"
     sodipodi:arg2="0.59272991"
     inkscape:flatsided="false"
     inkscape:rounded="0"
     inkscape:randomized="0"
     inkscape:transform-center-x="2.8191462"
     inkscape:transform-center-y="-2.9357597"
     transform="matrix(1.6123889,0,0,1.6423718,27.88509,-262.05498)"
     inkscape:path-effect="#path-effect16"
     d="m 46.626564,168.65661 76.145096,93.87972 M 19.379065,194.08397 95.524162,287.96369 M -7.8684335,219.51132 68.276664,313.39104 M -35.115932,244.93868 41.029165,338.8184 M -62.363431,270.36604 13.781666,364.24575 M -89.61093,295.79339 -13.465833,389.67311 M 46.626564,168.65661 -89.61093,295.79339 M 61.855583,187.43256 -74.38191,314.56934 M 77.084603,206.2085 -59.152891,333.34528 M 92.313622,224.98444 -43.923872,352.12122 M 107.54264,243.76039 -28.694852,370.89717 M 122.77166,262.53633 -13.465833,389.67311" />
</svg>
)"""";

   testDoc(svg);
}

// INKSCAPE 1.0.2

TEST_F(LPEConstructGridTest, constructgrid_PX_1_0_2)
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
   inkscape:version="1.0.2 (e86c870879, 2021-01-15)"
   inkscape:test-threshold="0.2">
  <defs
     id="defs2">
    <inkscape:path-effect
       effect="construct_grid"
       id="path-effect12"
       is_visible="true"
       lpeversion="1"
       nr_x="5"
       nr_y="5" />
    <inkscape:path-effect
       effect="construct_grid"
       id="path-effect16"
       is_visible="true"
       lpeversion="1"
       nr_x="5"
       nr_y="5" />
  </defs>
  <path
     style="fill:#ff0000;fill-rule:evenodd;stroke:#ff8080;stroke-width:0.264583"
     id="path02"
     inkscape:path-effect="#path-effect12"
     sodipodi:type="arc"
     sodipodi:cx="186.84631"
     sodipodi:cy="191.16818"
     sodipodi:rx="46.695065"
     sodipodi:ry="35.03508"
     d="M 186.84631,226.20326 420.32164,51.027863 M 140.15125,191.16818 373.62657,15.992783 M 93.456184,156.1331 326.93151,-19.042297 M 46.76112,121.09802 280.23644,-54.077377 M 0.0660553,86.062943 233.54138,-89.112457 M -46.629009,51.027863 186.84631,-124.14754 m 0,350.3508 L -46.629009,51.027863 M 233.54138,191.16818 0.0660553,15.992783 M 280.23644,156.1331 46.76112,-19.042297 M 326.93151,121.09802 93.456184,-54.077377 M 373.62657,86.062943 140.15125,-89.112457 M 420.32164,51.027863 186.84631,-124.14754" />
  <path
     sodipodi:type="star"
     style="stroke:#ff8080;stroke-width:0.264583"
     id="path01"
     sodipodi:sides="5"
     sodipodi:cx="36.039883"
     sodipodi:cy="161.52631"
     sodipodi:r1="36.572998"
     sodipodi:r2="12.763976"
     sodipodi:arg1="0.78714885"
     sodipodi:arg2="0.59272991"
     inkscape:flatsided="false"
     inkscape:rounded="0"
     inkscape:randomized="0"
     inkscape:transform-center-x="2.8191462"
     inkscape:transform-center-y="-2.9357597"
     transform="matrix(1.6123889,0,0,1.6423718,27.88509,-262.05498)"
     inkscape:path-effect="#path-effect16"
     d="m 46.626564,168.65661 76.145096,93.87972 M 19.379065,194.08397 95.524162,287.96369 M -7.8684335,219.51132 68.276664,313.39104 M -35.115932,244.93868 41.029165,338.8184 M -62.363431,270.36604 13.781666,364.24575 M -89.61093,295.79339 -13.465833,389.67311 M 46.626564,168.65661 -89.61093,295.79339 M 61.855583,187.43256 -74.38191,314.56934 M 77.084603,206.2085 -59.152891,333.34528 M 92.313622,224.98444 -43.923872,352.12122 M 107.54264,243.76039 -28.694852,370.89717 M 122.77166,262.53633 -13.465833,389.67311" />
</svg>
)"""";

   testDoc(svg);
}
