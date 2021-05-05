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

class LPEOffsetTest : public LPESPathsTest {};

// TEST fails if multiple simplify on stack


// INKSCAPE 1.0.2

TEST_F(LPEOffsetTest, multi_PX_1_0_2)
{
   std::string svg = R""""(<?xml version="1.0" encoding="UTF-8" standalone="no"?>
<svg
   xmlns:dc="http://purl.org/dc/elements/1.1/"
   xmlns:cc="http://creativecommons.org/ns#"
   xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#"
   xmlns:svg="http://www.w3.org/2000/svg"
   xmlns="http://www.w3.org/2000/svg"
   xmlns:sodipodi="http://sodipodi.sourceforge.net/DTD/sodipodi-0.dtd"
   xmlns:inkscape="http://www.inkscape.org/namespaces/inkscape"
   width="793.70081"
   height="1122.5197"
   viewBox="0 0 793.70081 1122.5197"
   version="1.1"
   id="svg1048"
   inkscape:version="1.0.2 (e86c870879, 2021-01-15)"
   sodipodi:docname="1.svg">
  <defs
     id="defs1042">
    <inkscape:path-effect
       effect="offset"
       id="path-effect3681"
       is_visible="true"
       lpeversion="1"
       linejoin_type="extrp_arc"
       unit="mm"
       offset="24.918991"
       miter_limit="4"
       attempt_force_join="true"
       update_on_knot_move="true" />
    <inkscape:path-effect
       effect="offset"
       id="path-effect1662"
       is_visible="true"
       lpeversion="1"
       linejoin_type="round"
       unit="mm"
       offset="33.655753"
       miter_limit="4"
       attempt_force_join="true"
       update_on_knot_move="true" />
    <inkscape:path-effect
       effect="offset"
       id="path-effect1654"
       is_visible="true"
       lpeversion="1"
       linejoin_type="miter"
       unit="mm"
       offset="24.719792"
       miter_limit="4"
       attempt_force_join="true"
       update_on_knot_move="true" />
    <inkscape:path-effect
       effect="offset"
       id="path-effect1650"
       is_visible="true"
       lpeversion="1"
       linejoin_type="miter"
       unit="mm"
       offset="32.773946"
       miter_limit="4"
       attempt_force_join="true"
       update_on_knot_move="true" />
    <inkscape:path-effect
       effect="offset"
       id="path-effect1646"
       is_visible="true"
       lpeversion="1"
       linejoin_type="miter"
       unit="mm"
       offset="39.135893"
       miter_limit="4"
       attempt_force_join="true"
       update_on_knot_move="true" />
    <inkscape:path-effect
       effect="offset"
       id="path-effect1642"
       is_visible="true"
       lpeversion="1"
       linejoin_type="miter"
       unit="mm"
       offset="-24.333219"
       miter_limit="4"
       attempt_force_join="true"
       update_on_knot_move="true" />
    <inkscape:path-effect
       effect="offset"
       id="path-effect1640"
       is_visible="true"
       lpeversion="1"
       linejoin_type="round"
       unit="mm"
       offset="12.868885"
       miter_limit="4"
       attempt_force_join="true"
       update_on_knot_move="true" />
    <inkscape:path-effect
       effect="offset"
       id="path-effect1638"
       is_visible="true"
       lpeversion="1"
       linejoin_type="miter"
       unit="mm"
       offset="-18.128821"
       miter_limit="4"
       attempt_force_join="true"
       update_on_knot_move="true" />
    <inkscape:path-effect
       effect="offset"
       id="path-effect1636"
       is_visible="true"
       lpeversion="1"
       linejoin_type="miter"
       unit="mm"
       offset="-24.893108"
       miter_limit="4"
       attempt_force_join="true"
       update_on_knot_move="true" />
  </defs>
  <sodipodi:namedview
     id="base"
     pagecolor="#ffffff"
     bordercolor="#666666"
     borderopacity="1.0"
     inkscape:pageopacity="0.0"
     inkscape:pageshadow="2"
     inkscape:zoom="0.0875"
     inkscape:cx="-3289.8269"
     inkscape:cy="614.26258"
     inkscape:document-units="px"
     inkscape:current-layer="layer1"
     inkscape:document-rotation="0"
     showgrid="false"
     inkscape:window-width="1920"
     inkscape:window-height="1016"
     inkscape:window-x="0"
     inkscape:window-y="27"
     inkscape:window-maximized="1"
     units="px" />
  <metadata
     id="metadata1045">
    <rdf:RDF>
      <cc:Work
         rdf:about="">
        <dc:format>image/svg+xml</dc:format>
        <dc:type
           rdf:resource="http://purl.org/dc/dcmitype/StillImage" />
        <dc:title></dc:title>
      </cc:Work>
    </rdf:RDF>
  </metadata>
  <g
     inkscape:label="Layer 1"
     inkscape:groupmode="layer"
     id="layer1">
    <path
       sodipodi:type="star"
       style="fill:none;fill-rule:evenodd;stroke:#000000;stroke-width:27.0512;stroke-miterlimit:4;stroke-dasharray:none"
       id="path1644"
       sodipodi:sides="5"
       sodipodi:cx="-399.80942"
       sodipodi:cy="-180.44173"
       sodipodi:r1="360.73807"
       sodipodi:r2="180.36903"
       sodipodi:arg1="0.57435009"
       sodipodi:arg2="1.2026686"
       inkscape:flatsided="false"
       inkscape:rounded="0"
       inkscape:randomized="0"
       d="m -649.90625,-828.83984 -41.63086,459.61718 -402.22069,226.26954 424.2578,181.621089 90.89844,452.458981 303.83789,-347.36718 458.40429,53.36132 -236.474604,-396.30664 192.408204,-419.47656 -449.98828,102.4375 z"
       inkscape:transform-center-x="914.07392"
       inkscape:transform-center-y="-444.93363"
       inkscape:path-effect="#path-effect1646" />
    <path
       sodipodi:type="star"
       style="fill:none;fill-rule:evenodd;stroke:#000000;stroke-width:10;stroke-miterlimit:4;stroke-dasharray:none"
       id="path1611"
       sodipodi:sides="5"
       sodipodi:cx="-311.88177"
       sodipodi:cy="86.136711"
       sodipodi:r1="133.35405"
       sodipodi:r2="66.677025"
       sodipodi:arg1="0.57435009"
       sodipodi:arg2="1.2026686"
       inkscape:flatsided="false"
       inkscape:rounded="0"
       inkscape:randomized="0"
       d=""
       inkscape:transform-center-x="10.601544"
       inkscape:transform-center-y="2.2228879"
       inkscape:path-effect="#path-effect1638" />
    <path
       id="path1648"
       style="fill:none;fill-rule:evenodd;stroke:#000000;stroke-width:27.0512;stroke-miterlimit:4;stroke-dasharray:none"
       d="m 987.91602,-298.51172 c -180.57249,0 -296.54299,182.60148 -296.54297,358.300782 0,175.699298 115.97206,358.298828 296.54297,358.298828 180.57088,0 296.54488,-182.59838 296.54488,-358.298828 0,-175.700452 -115.9724,-358.300782 -296.54488,-358.300782 z"
       inkscape:path-effect="#path-effect1650"
       inkscape:original-d="m 1055.8342,40.212072 a 72.02242,89.876914 0 0 1 -72.02238,89.876918 72.02242,89.876914 0 0 1 -72.02241,-89.876918 72.02242,89.876914 0 0 1 72.02241,-89.876913 72.02242,89.876914 0 0 1 72.02238,89.876913 z m 104.7572,19.576541 A 172.67497,234.43077 0 0 1 987.91643,294.21937 172.67497,234.43077 0 0 1 815.24146,59.788613 172.67497,234.43077 0 0 1 987.91643,-174.64217 172.67497,234.43077 0 0 1 1160.5914,59.788613 Z" />
    <path
       id="path1616"
       inkscape:test-ignore="1"
       style="fill:none;fill-rule:nonzero;stroke:#000000;stroke-width:27.0512;stroke-miterlimit:4;stroke-dasharray:none"
       d="m 579.83203,1242.3613 c -128.82205,0 -221.31446,135.4441 -221.31445,283.0684 3e-5,147.6242 92.49178,283.0703 221.31445,283.0703 128.82267,0 221.31247,-135.4466 221.3125,-283.0703 10e-6,-147.6238 -92.49045,-283.0684 -221.3125,-283.0684 z m -4.10547,222.2539 c 7.23066,0 23.38476,12.501 23.38477,41.2383 2e-5,28.7374 -16.15406,41.2383 -23.38477,41.2383 -7.23069,0 -23.38283,-12.4996 -23.38281,-41.2383 0,-28.7385 16.15218,-41.2383 23.38281,-41.2383 z"
       inkscape:path-effect="#path-effect1640"
       inkscape:original-d="m 647.74935,1505.8539 a 72.02242,89.876914 0 0 1 -72.02242,89.877 72.02242,89.876914 0 0 1 -72.02241,-89.877 72.02242,89.876914 0 0 1 72.02241,-89.8769 72.02242,89.876914 0 0 1 72.02242,89.8769 z m 104.75716,19.5766 a 172.67497,234.43077 0 0 1 -172.67497,234.4307 172.67497,234.43077 0 0 1 -172.67497,-234.4307 172.67497,234.43077 0 0 1 172.67497,-234.4308 172.67497,234.43077 0 0 1 172.67497,234.4308 z" />
    <path
       id="path1652"
       style="fill:#c83737;fill-rule:evenodd;stroke:#000000;stroke-width:27.0512;stroke-miterlimit:4;stroke-dasharray:none"
       d="m 1764.834,-209.97461 c -221.9177,3e-5 -373.7871,237.89726 -373.7871,490.7168 0,121.73627 34.0309,242.42939 99.209,334.79101 43.8495,62.13773 101.1883,110.21898 167.5742,135.72266 15.5988,31.81693 34.0552,62.11729 55.4824,89.94726 78.2086,101.57897 189.7106,164.54888 312.1719,164.54888 122.4612,0 233.9631,-62.9699 312.1718,-164.54888 76.8408,-99.80231 118.0411,-232.43198 118.0411,-366.30273 0,-133.87075 -41.2003,-266.50037 -118.0411,-366.30273 C 2262.1383,10.513581 2155.545,-51.392792 2038.0508,-55.548828 1973.3853,-146.25589 1878.6887,-209.97465 1764.834,-209.97461 Z m 169.7539,366.6875 c 10.8288,37.52949 17.1738,79.27415 17.1738,124.0293 0,88.99704 -25.045,170.40349 -65.0215,227.05273 -28.7151,40.69117 -62.047,63.53929 -92.5683,72.42578 -7.8479,-33.60362 -12.043,-69.06229 -12.043,-105.32031 0,-98.15148 29.7638,-188.04305 79.2422,-252.30664 22.4056,-29.10089 47.3813,-50.85792 73.2168,-65.88086 z"
       inkscape:path-effect="#path-effect1654"
       inkscape:original-d="M 2362.2679,474.90029 A 336.78439,437.42233 0 0 1 2025.4835,912.3226 336.78439,437.42233 0 0 1 1688.6991,474.90029 336.78439,437.42233 0 0 1 2025.4835,37.477958 336.78439,437.42233 0 0 1 2362.2679,474.90029 Z M 2045.1921,280.74126 a 280.35793,397.28546 0 0 1 -280.358,397.28546 280.35793,397.28546 0 0 1 -280.3579,-397.28546 280.35793,397.28546 0 0 1 280.3579,-397.28545 280.35793,397.28546 0 0 1 280.358,397.28545 z" />
    <g
       id="g1660"
       inkscape:path-effect="#path-effect1662"
       transform="matrix(2.7051153,0,0,2.7051153,-1432.8252,108.90945)">
      <ellipse
         style="fill:none;fill-rule:evenodd;stroke:#000000;stroke-width:10;stroke-miterlimit:4;stroke-dasharray:none"
         id="ellipse1656"
         cx="-54.396549"
         cy="390.94354"
         rx="127.62676"
         ry="101.8336"
         d="m -54.396484,242.08594 c -90.446856,-2e-5 -174.650386,61.26383 -174.650396,148.85742 0,87.59358 84.20355,148.85742 174.650396,148.85742 90.446846,0 174.650394,-61.26384 174.650394,-148.85742 0,-87.59358 -84.203555,-148.85742 -174.650394,-148.85742 z" />
      <path
         id="path1658"
         style="fill:none;fill-rule:evenodd;stroke:#000000;stroke-width:10;stroke-miterlimit:4;stroke-dasharray:none"
         inkscape:transform-center-x="-2.1853814"
         inkscape:transform-center-y="-4.9767892"
         d="m -634.80273,278.22461 a 47.023077,47.023077 0 0 0 -39.70118,22.28906 l -12.91992,20.89258 -24.53515,1.19922 a 47.023077,47.023077 0 0 0 -33.58594,77.35937 l 15.87695,18.74414 -6.44141,23.70899 a 47.023077,47.023077 0 0 0 63.19532,55.8457 l 22.73633,-9.30859 20.55664,13.45117 a 47.023077,47.023077 0 0 0 72.63867,-42.8457 l -1.82617,-24.49414 19.14453,-15.39258 a 47.023077,47.023077 0 0 0 -18.29883,-82.32617 l -23.86523,-5.83399 -8.72266,-22.96484 a 47.023077,47.023077 0 0 0 -44.25195,-30.32422 z"
         inkscape:original-d="m -603.8755,463.05932 -41.24201,-26.98797 -45.61276,18.67429 12.9226,-47.56323 -31.85542,-37.60964 49.22862,-2.40772 25.92503,-41.91833 17.50235,46.07517 47.87798,11.7027 -38.41157,30.88374 z" />
    </g>
    <g
       id="g1634"
       inkscape:path-effect="#path-effect1636"
       transform="matrix(2.7051153,0,0,2.7051153,462.32306,1530.401)">
      <ellipse
         style="fill:none;fill-rule:evenodd;stroke:#000000;stroke-width:10;stroke-miterlimit:4;stroke-dasharray:none"
         id="path1627"
         cx="-54.396549"
         cy="390.94354"
         rx="127.62676"
         ry="101.8336"
         d="m -54.396484,323.88867 c -55.722726,-10e-6 -92.847656,34.00302 -92.847656,67.05469 0,33.05167 37.12493,67.05469 92.847656,67.05469 55.7227241,0 92.847656,-34.00302 92.847656,-67.05469 0,-33.05167 -37.1249319,-67.05469 -92.847656,-67.05469 z" />
      <path
         id="path1629"
         style="fill:none;fill-rule:evenodd;stroke:#000000;stroke-width:10;stroke-miterlimit:4;stroke-dasharray:none"
         inkscape:transform-center-x="-2.1853814"
         inkscape:transform-center-y="-4.9767892"
         d=""
         inkscape:original-d="m 28.693756,340.79243 -41.24201,-26.98797 -45.612767,18.67429 12.922603,-47.56323 -31.855423,-37.60964 49.228618,-2.40772 25.9250314,-41.91833 17.5023566,46.07517 47.877974,11.7027 -38.411567,30.88374 z" />
    </g>
    <path
       style="fill:none;stroke:#000000;stroke-width:1px;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1"
       d="m -1461.7676,-2227.6543 a 401.082,401.082 0 0 0 -180.7383,50.5313 c -333.0238,185.7407 -811.2228,1075.1109 -885.4902,1328.32612 -8.4602,28.84501 -18.8206,74.40294 -5.2793,111.44922 l 11.9688,32.7461 30.4062,17.05859 c 176.1937,98.85023 223.7988,166.16406 223.7988,166.16406 5.4943,6.78069 7.5362,7.65414 8.2149,6.80079 2.0853,190.93769 160.3336,158.58429 775.0195,245.79101 673.27514,95.51889 1251.90788,-179.63063 1527.507825,-1146.43749 135.812225,-476.4304 -240.781525,-493.876 -602.542965,-430.5684 -343.3065,60.0781 -729.20016,167.5763 -887.42966,71.5645 -53.9163,-32.7158 -97.441,-106.1311 -57.9707,-302.0391 a 1047.9395,1047.9395 0 0 1 42.5351,-151.3867 z"
       id="path3679"
       sodipodi:nodetypes="ccsssc"
       inkscape:path-effect="#path-effect3681"
       inkscape:original-d="m -2444.8182,-769.68124 c -30.9069,-84.55485 513.3048,-1138.40856 848.1897,-1325.18726 -221.209,1097.95586 1812.34994,-267.5661 1549.695866,653.8265 -263.619156,924.77814 -796.925516,1167.92916 -1423.705466,1079.00663 -626.7799,-88.92252 -694.1671,-62.07062 -694.1671,-156.31001 0,-94.23939 -280.013,-251.33586 -280.013,-251.33586 z" />
  </g>
</svg>
)"""";

   testDoc(svg);
}

TEST_F(LPEOffsetTest, multi_MM_1_0_2)
{
   std::string svg = R""""(<?xml version="1.0" encoding="UTF-8" standalone="no"?>
<svg
   xmlns:dc="http://purl.org/dc/elements/1.1/"
   xmlns:cc="http://creativecommons.org/ns#"
   xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#"
   xmlns:svg="http://www.w3.org/2000/svg"
   xmlns="http://www.w3.org/2000/svg"
   xmlns:sodipodi="http://sodipodi.sourceforge.net/DTD/sodipodi-0.dtd"
   xmlns:inkscape="http://www.inkscape.org/namespaces/inkscape"
   width="210mm"
   height="297mm"
   viewBox="0 0 210 296.99999"
   version="1.1"
   id="svg1048"
   inkscape:version="1.0.2 (e86c870879, 2021-01-15)"
   sodipodi:docname="1.svg">
  <defs
     id="defs1042">
    <inkscape:path-effect
       effect="offset"
       id="path-effect3681"
       is_visible="true"
       lpeversion="1"
       linejoin_type="extrp_arc"
       unit="mm"
       offset="10.265411"
       miter_limit="4"
       attempt_force_join="true"
       update_on_knot_move="true" />
    <inkscape:path-effect
       effect="offset"
       id="path-effect1662"
       is_visible="true"
       lpeversion="1"
       linejoin_type="round"
       unit="mm"
       offset="33.655753"
       miter_limit="4"
       attempt_force_join="true"
       update_on_knot_move="true" />
    <inkscape:path-effect
       effect="offset"
       id="path-effect1654"
       is_visible="true"
       lpeversion="1"
       linejoin_type="miter"
       unit="mm"
       offset="10.18335"
       miter_limit="4"
       attempt_force_join="true"
       update_on_knot_move="true" />
    <inkscape:path-effect
       effect="offset"
       id="path-effect1650"
       is_visible="true"
       lpeversion="1"
       linejoin_type="miter"
       unit="mm"
       offset="13.501269"
       miter_limit="4"
       attempt_force_join="true"
       update_on_knot_move="true" />
    <inkscape:path-effect
       effect="offset"
       id="path-effect1646"
       is_visible="true"
       lpeversion="1"
       linejoin_type="miter"
       unit="mm"
       offset="39.135893"
       miter_limit="4"
       attempt_force_join="true"
       update_on_knot_move="true" />
    <inkscape:path-effect
       effect="offset"
       id="path-effect1642"
       is_visible="true"
       lpeversion="1"
       linejoin_type="miter"
       unit="mm"
       offset="-24.333219"
       miter_limit="4"
       attempt_force_join="true"
       update_on_knot_move="true" />
    <inkscape:path-effect
       effect="offset"
       id="path-effect1640"
       is_visible="true"
       lpeversion="1"
       linejoin_type="round"
       unit="mm"
       offset="5.3013538"
       miter_limit="4"
       attempt_force_join="true"
       update_on_knot_move="true" />
    <inkscape:path-effect
       effect="offset"
       id="path-effect1638"
       is_visible="true"
       lpeversion="1"
       linejoin_type="miter"
       unit="mm"
       offset="-18.128821"
       miter_limit="4"
       attempt_force_join="true"
       update_on_knot_move="true" />
    <inkscape:path-effect
       effect="offset"
       id="path-effect1636"
       is_visible="true"
       lpeversion="1"
       linejoin_type="miter"
       unit="mm"
       offset="-24.893108"
       miter_limit="4"
       attempt_force_join="true"
       update_on_knot_move="true" />
  </defs>
  <sodipodi:namedview
     id="base"
     pagecolor="#ffffff"
     bordercolor="#666666"
     borderopacity="1.0"
     inkscape:pageopacity="0.0"
     inkscape:pageshadow="2"
     inkscape:zoom="0.01"
     inkscape:cx="-29221.75"
     inkscape:cy="-7336.0358"
     inkscape:document-units="mm"
     inkscape:current-layer="layer1"
     inkscape:document-rotation="0"
     showgrid="false"
     inkscape:window-width="1920"
     inkscape:window-height="1016"
     inkscape:window-x="0"
     inkscape:window-y="27"
     inkscape:window-maximized="1"
     units="mm" />
  <metadata
     id="metadata1045">
    <rdf:RDF>
      <cc:Work
         rdf:about="">
        <dc:format>image/svg+xml</dc:format>
        <dc:type
           rdf:resource="http://purl.org/dc/dcmitype/StillImage" />
        <dc:title></dc:title>
      </cc:Work>
    </rdf:RDF>
  </metadata>
  <g
     inkscape:label="Layer 1"
     inkscape:groupmode="layer"
     id="layer1">
    <path
       sodipodi:type="star"
       style="fill:none;fill-rule:evenodd;stroke:#000000;stroke-width:27.0512;stroke-miterlimit:4;stroke-dasharray:none"
       id="path1644"
       sodipodi:sides="5"
       sodipodi:cx="-399.80942"
       sodipodi:cy="-180.44173"
       sodipodi:r1="360.73807"
       sodipodi:r2="180.36903"
       sodipodi:arg1="0.57435009"
       sodipodi:arg2="1.2026686"
       inkscape:flatsided="false"
       inkscape:rounded="0"
       inkscape:randomized="0"
       d="m -606.87891,-717.28906 -34.46679,380.54492 -333.02344,187.3418 L -623.10156,0.97265625 -547.8418,375.58984 -296.27344,87.982422 83.263672,132.16406 -112.5293,-195.96094 46.777344,-543.26953 l -372.570314,84.8125 z"
       inkscape:transform-center-x="379.29723"
       inkscape:transform-center-y="-183.09287"
       inkscape:path-effect="#path-effect1646"
       transform="matrix(0.41044644,0,0,0.41346166,-2004.1061,1672.8518)" />
    <path
       sodipodi:type="star"
       style="fill:none;fill-rule:evenodd;stroke:#000000;stroke-width:10;stroke-miterlimit:4;stroke-dasharray:none"
       id="path1611"
       sodipodi:sides="5"
       sodipodi:cx="-311.88177"
       sodipodi:cy="86.136711"
       sodipodi:r1="133.35405"
       sodipodi:r2="66.677025"
       sodipodi:arg1="0.57435009"
       sodipodi:arg2="1.2026686"
       inkscape:flatsided="false"
       inkscape:rounded="0"
       inkscape:randomized="0"
       d="m -324.08789,54.492188 -2.03125,22.43164 -19.63086,11.042969 20.70508,8.863281 4.43554,22.082032 14.83008,-16.95313 22.3711,2.60352 -11.54102,-19.341797 9.39063,-20.472656 -21.96094,5 z"
       inkscape:transform-center-x="2.4433845"
       inkscape:transform-center-y="0.51611801"
       inkscape:path-effect="#path-effect1638"
       transform="matrix(0.41044644,0,0,0.41346166,-2004.1061,1672.8518)" />
    <path
       id="path1648"
       style="fill:none;fill-rule:evenodd;stroke:#000000;stroke-width:11.1438;stroke-miterlimit:4;stroke-dasharray:none"
       d="m -1598.6191,1587.1426 c -48.4978,1e-4 -84.377,51.8958 -84.377,110.4297 0,58.5339 35.8792,110.4297 84.377,110.4297 48.4977,10e-5 84.375,-51.8961 84.375,-110.4297 0,-58.5337 -35.8773,-110.4297 -84.375,-110.4297 z m -1.6856,78.6758 c 7.2965,0 16.0606,8.8512 16.0606,23.6601 0,14.8091 -8.7631,23.6582 -16.0606,23.6582 -7.2976,0 -16.0587,-8.8483 -16.0586,-23.6582 0,-14.8097 8.7623,-23.6601 16.0586,-23.6601 z"
       inkscape:path-effect="#path-effect1650"
       inkscape:original-d="m -1570.7428,1689.4779 a 29.561346,37.160658 0 0 1 -29.5613,37.1607 29.561346,37.160658 0 0 1 -29.5613,-37.1607 29.561346,37.160658 0 0 1 29.5613,-37.1606 29.561346,37.160658 0 0 1 29.5613,37.1606 z m 42.9973,8.0942 a 70.873827,96.928135 0 0 1 -70.8739,96.9281 70.873827,96.928135 0 0 1 -70.8738,-96.9281 70.873827,96.928135 0 0 1 70.8738,-96.9281 70.873827,96.928135 0 0 1 70.8739,96.9281 z" />
    <path
       id="path1616"
       inkscape:test-ignore="1"
       style="fill:none;fill-rule:nonzero;stroke:#000000;stroke-width:11.1438;stroke-miterlimit:4;stroke-dasharray:none"
       d="m -1766.1172,2201.3301 c -42.8159,0 -76.1738,46.7326 -76.1738,102.2285 0,55.4958 33.3579,102.2285 76.1738,102.2285 42.816,1e-4 76.1758,-46.7325 76.1758,-102.2285 0,-55.4961 -33.3598,-102.2286 -76.1758,-102.2285 z"
       inkscape:path-effect="#path-effect1640"
       inkscape:original-d="m -1738.2397,2295.4647 a 29.561346,37.160658 0 0 1 -29.5614,37.1606 29.561346,37.160658 0 0 1 -29.5613,-37.1606 29.561346,37.160658 0 0 1 29.5613,-37.1607 29.561346,37.160658 0 0 1 29.5614,37.1607 z m 42.9972,8.0941 a 70.873827,96.928135 0 0 1 -70.8739,96.9281 70.873827,96.928135 0 0 1 -70.8738,-96.9281 70.873827,96.928135 0 0 1 70.8738,-96.9281 70.873827,96.928135 0 0 1 70.8739,96.9281 z" />
    <path
       id="path1652"
       style="fill:#c83737;fill-rule:evenodd;stroke:#000000;stroke-width:11.1438;stroke-miterlimit:4;stroke-dasharray:none"
       d="m -1279.7363,1614.4824 c -70.9176,0 -125.2559,80.1114 -125.2559,174.4453 0,83.4541 42.6903,155.5094 101.7871,171.0274 24.7126,58.8574 73.1764,100.291 130.4512,100.291 83.3189,0 148.416,-87.0813 148.416,-191.041 0,-103.9598 -65.0971,-191.0411 -148.416,-191.041 -3.456,0 -6.8283,0.2202 -10.1563,0.5332 -22.4642,-38.3341 -56.816,-64.2149 -96.8261,-64.2149 z m 85.8242,86.6426 c 11.8892,24.8653 19.0644,54.9751 19.0644,87.8027 10e-5,87.1053 -48.7012,154.0782 -104.8886,154.0782 -2.7543,0 -5.5402,-0.273 -8.3633,-0.6602 -7.9819,-22.1922 -12.7031,-46.858 -12.7031,-73.1406 0,-85.9267 47.3181,-154.746 106.8906,-168.0801 z"
       inkscape:path-effect="#path-effect1654"
       inkscape:original-d="m -1034.5217,1869.2049 a 138.23195,180.85736 0 0 1 -138.232,180.8573 138.23195,180.85736 0 0 1 -138.2319,-180.8573 138.23195,180.85736 0 0 1 138.2319,-180.8574 138.23195,180.85736 0 0 1 138.232,180.8574 z m -130.1426,-80.2774 a 115.07191,164.26231 0 0 1 -115.072,164.2624 115.07191,164.26231 0 0 1 -115.0719,-164.2624 115.07191,164.26231 0 0 1 115.0719,-164.2623 115.07191,164.26231 0 0 1 115.072,164.2623 z" />
    <g
       id="g1660"
       inkscape:path-effect="#path-effect1662"
       transform="matrix(1.1103049,0,0,1.1184615,-2592.2041,1717.8817)">
      <ellipse
         style="fill:none;fill-rule:evenodd;stroke:#000000;stroke-width:10;stroke-miterlimit:4;stroke-dasharray:none"
         id="ellipse1656"
         cx="-54.396549"
         cy="390.94354"
         rx="127.62676"
         ry="101.8336" />
      <path
         id="path1658"
         style="fill:none;fill-rule:evenodd;stroke:#000000;stroke-width:10;stroke-miterlimit:4;stroke-dasharray:none"
         inkscape:transform-center-x="-2.1853814"
         inkscape:transform-center-y="-4.9767892"
         d="m -634.60938,312.80469 a 12.441523,12.441523 0 0 0 -10.48242,5.89843 l -22.48242,36.35743 -42.69726,2.08593 a 12.441523,12.441523 0 0 0 -8.88672,20.4668 l 27.6289,32.61914 -11.20703,41.25196 a 12.441523,12.441523 0 0 0 16.71875,14.77539 l 39.56055,-16.19532 35.76953,23.4043 a 12.441523,12.441523 0 0 0 19.21875,-11.33594 l -3.17969,-42.62695 33.31446,-26.7832 a 12.441523,12.441523 0 0 0 -4.83985,-21.78321 l -41.52539,-10.15039 -15.17969,-39.96094 a 12.441523,12.441523 0 0 0 -11.73047,-8.02343 z"
         inkscape:original-d="m -603.8755,463.05932 -41.24201,-26.98797 -45.61276,18.67429 12.9226,-47.56323 -31.85542,-37.60964 49.22862,-2.40772 25.92503,-41.91833 17.50235,46.07517 47.87798,11.7027 -38.41157,30.88374 z" />
    </g>
    <g
       id="g1634"
       inkscape:path-effect="#path-effect1636"
       transform="matrix(1.1103049,0,0,1.1184615,-1814.3473,2305.6139)">
      <ellipse
         style="fill:none;fill-rule:evenodd;stroke:#000000;stroke-width:10;stroke-miterlimit:4;stroke-dasharray:none"
         id="path1627"
         cx="-54.396549"
         cy="390.94354"
         rx="127.62676"
         ry="101.8336" />
      <path
         id="path1629"
         style="fill:none;fill-rule:evenodd;stroke:#000000;stroke-width:10;stroke-miterlimit:4;stroke-dasharray:none"
         inkscape:transform-center-x="-2.1853814"
         inkscape:transform-center-y="-4.9767892"
         d="m -3.9199219,223.67773 -18.6640621,30.17579 -35.4375,1.73437 22.93164,27.07227 -9.300781,34.23828 32.832031,-13.44336 29.6875,19.42773 L 15.492188,287.5 43.140625,265.26953 8.6777344,256.8457 Z"
         inkscape:original-d="m 28.693756,340.79243 -41.24201,-26.98797 -45.612767,18.67429 12.922603,-47.56323 -31.855423,-37.60964 49.228618,-2.40772 25.9250314,-41.91833 17.5023566,46.07517 47.877974,11.7027 -38.411567,30.88374 z" />
    </g>
    <path
       style="fill:none;stroke:#000000;stroke-width:0.411951px;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1"
       d="m -2644.8633,788.5957 a 144.72395,144.72395 0 0 0 -19.6015,9.15821 c -127.1615,71.44407 -321.23,433.39549 -350.0098,532.24219 -3.2202,11.0599 -5.142,21.5255 -2.7481,28.123 a 13.744083,13.744083 0 0 0 2.4668,4.2344 735.44165,735.44165 0 0 1 2.1329,1.2012 c 41.5345,23.4736 59.539,38.9805 59.539,38.9805 7.2813,5.4436 14.4233,11.1282 20.9766,16.8769 3.6362,3.1898 7.0625,6.3736 10.2168,9.5313 3.5113,3.515 6.6224,6.9309 9.2968,10.2558 6.82,8.4787 9.6856,14.571 9.6856,19.336 0,50.0605 37.3583,38.1517 293.7305,74.791 262.3623,37.4952 486.195,-66.5789 595.6855,-453.4942 46.4676,-164.20611 -73.6264,-166.84132 -215.1328,-141.89645 -139.4529,24.58286 -306.4554,73.16455 -383.8242,25.87304 -35.9341,-21.96461 -54.5678,-66.86399 -36.9219,-155.09179 a 448.44293,448.44293 0 0 1 4.5078,-20.1211 z"
       id="path3679"
       sodipodi:nodetypes="ccsssc"
       inkscape:path-effect="#path-effect3681"
       inkscape:original-d="m -3007.5731,1354.6181 c -12.6856,-34.9602 210.6842,-470.68828 348.1365,-547.91411 -90.7945,453.96261 743.8726,-110.62832 636.0671,270.33221 -108.2015,382.3603 -327.0952,482.8939 -584.3548,446.1279 -257.2596,-36.7661 -284.9184,-25.6639 -284.9184,-64.6282 0,-38.9644 -114.9304,-103.9178 -114.9304,-103.9178 z" />
  </g>
</svg>
)"""";

   testDoc(svg);
}

TEST_F(LPEOffsetTest, multi_MM_1_1)
{
   std::string svg = R""""(<?xml version="1.0" encoding="UTF-8" standalone="no"?>
<svg
   width="210mm"
   height="297mm"
   viewBox="0 0 210 297"
   version="1.1"
   id="svg1048"
   inkscape:version="1.1-dev (85844a075a, 2021-04-29)"
   sodipodi:docname="1.svg"
   xmlns:inkscape="http://www.inkscape.org/namespaces/inkscape"
   xmlns:sodipodi="http://sodipodi.sourceforge.net/DTD/sodipodi-0.dtd"
   xmlns="http://www.w3.org/2000/svg"
   xmlns:svg="http://www.w3.org/2000/svg"
   xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#"
   xmlns:cc="http://creativecommons.org/ns#"
   xmlns:dc="http://purl.org/dc/elements/1.1/">
  <defs
     id="defs1042">
    <inkscape:path-effect
       effect="offset"
       id="path-effect1662"
       is_visible="true"
       lpeversion="1"
       linejoin_type="miter"
       unit="mm"
       offset="33.655753"
       miter_limit="4"
       attempt_force_join="true"
       update_on_knot_move="true" />
    <inkscape:path-effect
       effect="offset"
       id="path-effect1654"
       is_visible="true"
       lpeversion="1"
       linejoin_type="miter"
       unit="mm"
       offset="35.530225"
       miter_limit="4"
       attempt_force_join="true"
       update_on_knot_move="true" />
    <inkscape:path-effect
       effect="offset"
       id="path-effect1650"
       is_visible="true"
       lpeversion="1"
       linejoin_type="miter"
       unit="mm"
       offset="12.115545"
       miter_limit="4"
       attempt_force_join="true"
       update_on_knot_move="true" />
    <inkscape:path-effect
       effect="offset"
       id="path-effect1646"
       is_visible="true"
       lpeversion="1"
       linejoin_type="miter"
       unit="mm"
       offset="14.467366"
       miter_limit="4"
       attempt_force_join="true"
       update_on_knot_move="true" />
    <inkscape:path-effect
       effect="offset"
       id="path-effect1642"
       is_visible="true"
       lpeversion="1"
       linejoin_type="miter"
       unit="mm"
       offset="-25.387334"
       miter_limit="4"
       attempt_force_join="true"
       update_on_knot_move="true" />
    <inkscape:path-effect
       effect="offset"
       id="path-effect1640"
       is_visible="true"
       lpeversion="1"
       linejoin_type="miter"
       unit="mm"
       offset="-6.9244833"
       miter_limit="4"
       attempt_force_join="true"
       update_on_knot_move="true" />
    <inkscape:path-effect
       effect="offset"
       id="path-effect1638"
       is_visible="true"
       lpeversion="1"
       linejoin_type="miter"
       unit="mm"
       offset="-18.128821"
       miter_limit="4"
       attempt_force_join="true"
       update_on_knot_move="true" />
    <inkscape:path-effect
       effect="offset"
       id="path-effect1636"
       is_visible="true"
       lpeversion="1"
       linejoin_type="miter"
       unit="mm"
       offset="-24.893108"
       miter_limit="4"
       attempt_force_join="true"
       update_on_knot_move="true" />
  </defs>
  <sodipodi:namedview
     id="base"
     pagecolor="#ffffff"
     bordercolor="#666666"
     borderopacity="1.0"
     inkscape:pageopacity="0.0"
     inkscape:pageshadow="2"
     inkscape:zoom="0.12374369"
     inkscape:cx="-884.89361"
     inkscape:cy="-60.609151"
     inkscape:document-units="mm"
     inkscape:current-layer="layer1"
     inkscape:document-rotation="0"
     showgrid="false"
     inkscape:window-width="1920"
     inkscape:window-height="1016"
     inkscape:window-x="0"
     inkscape:window-y="27"
     inkscape:window-maximized="1"
     objecttolerance="10.0"
     gridtolerance="10.0"
     guidetolerance="10.0"
     inkscape:pagecheckerboard="0" />
  <metadata
     id="metadata1045">
    <rdf:RDF>
      <cc:Work
         rdf:about="">
        <dc:format>image/svg+xml</dc:format>
        <dc:type
           rdf:resource="http://purl.org/dc/dcmitype/StillImage" />
      </cc:Work>
    </rdf:RDF>
  </metadata>
  <g
     inkscape:label="Layer 1"
     inkscape:groupmode="layer"
     id="layer1">
    <path
       sodipodi:type="star"
       style="fill:none;fill-rule:evenodd;stroke:#000000;stroke-width:10;stroke-miterlimit:4;stroke-dasharray:none"
       id="path1644"
       sodipodi:sides="5"
       sodipodi:cx="-278.59417"
       sodipodi:cy="-605.4344"
       sodipodi:r1="133.35405"
       sodipodi:r2="66.677025"
       sodipodi:arg1="0.57435009"
       sodipodi:arg2="1.2026686"
       inkscape:flatsided="false"
       inkscape:rounded="0"
       inkscape:randomized="0"
       d="m -338.34766,-760.35352 -9.94726,109.8125 -96.10156,54.06446 101.36718,43.39453 21.7168,108.10351 72.59375,-82.99609 109.52734,12.75 -56.5,-94.68945 45.96875,-100.22266 -107.51367,24.47461 z"
       inkscape:transform-center-x="10.601544"
       inkscape:transform-center-y="2.2228879"
       inkscape:path-effect="#path-effect1646" />
    <path
       sodipodi:type="star"
       style="fill:none;fill-rule:evenodd;stroke:#000000;stroke-width:10;stroke-miterlimit:4;stroke-dasharray:none"
       id="path1611"
       sodipodi:sides="5"
       sodipodi:cx="-311.88177"
       sodipodi:cy="86.136711"
       sodipodi:r1="133.35405"
       sodipodi:r2="66.677025"
       sodipodi:arg1="0.57435009"
       sodipodi:arg2="1.2026686"
       inkscape:flatsided="false"
       inkscape:rounded="0"
       inkscape:randomized="0"
       d="m -345.13086,-0.06445312 c -1.8444,20.36783812 -3.6888,40.73567712 -5.5332,61.10351512 -17.82487,10.027344 -35.64974,20.054688 -53.47461,30.082032 18.80078,8.048829 37.60156,16.097656 56.40234,24.146486 4.028,20.05013 8.05599,40.10026 12.08399,60.15039 13.46484,-15.39388 26.92969,-30.78776 40.39453,-46.18164 20.3138,2.36458 40.6276,4.72917 60.9414,7.09375 -10.47917,-17.56185 -20.95833,-35.1237 -31.4375,-52.685549 8.52669,-18.589193 17.05339,-37.178385 25.58008,-55.767578 -19.94141,4.539714 -39.88281,9.079427 -59.82422,13.619141 -15.04427,-13.853516 -30.08854,-27.707031 -45.13281,-41.56054712 z"
       inkscape:transform-center-x="10.601544"
       inkscape:transform-center-y="2.2228879"
       inkscape:path-effect="#path-effect1638" />
    <path
       id="path1648"
       style="fill:none;fill-rule:evenodd;stroke:#000000;stroke-width:10;stroke-miterlimit:4;stroke-dasharray:none"
       d="m -0.68554688,-632.60547 c -43.58775312,0 -75.94726812,46.3939 -75.94726512,98.77735 2e-6,52.38343 32.359518,98.77734 75.94726512,98.77734 43.58775688,10e-6 75.94726388,-46.39391 75.94726588,-98.77734 2e-6,-52.38345 -32.359511,-98.77736 -75.94726588,-98.77735 z m -1.51757812,70.42969 c 6.5973407,0 14.509766,7.96603 14.509766,21.10937 0,13.14335 -7.9124253,21.10938 -14.509766,21.10938 -6.5973376,0 -14.509766,-7.96603 -14.509766,-21.10938 0,-13.14334 7.9124267,-21.10937 14.509766,-21.10937 z"
       inkscape:path-effect="#path-effect1650"
       inkscape:original-d="m 24.42155,-541.06574 a 26.624529,33.2248 0 0 1 -26.6245296,33.2248 26.624529,33.2248 0 0 1 -26.6245264,-33.2248 26.624529,33.2248 0 0 1 26.6245264,-33.2248 26.624529,33.2248 0 0 1 26.6245296,33.2248 z m 38.725583,7.23686 a 63.83276,86.662025 0 0 1 -63.83276263,86.66202 63.83276,86.662025 0 0 1 -63.83275637,-86.66202 63.83276,86.662025 0 0 1 63.83275637,-86.66203 63.83276,86.662025 0 0 1 63.83276263,86.66203 z" />
    <path
       id="path1616"
       inkscape:test-ignore="1"
       style="fill:none;fill-rule:evenodd;stroke:#000000;stroke-width:10;stroke-miterlimit:4;stroke-dasharray:none"
       d="m -33.972656,78.003906 c -5.448084,-0.0067 -10.879166,1.126733 -15.911534,3.200592 -6.434278,2.640102 -12.203101,6.746747 -17.112432,11.646375 -5.615727,5.606754 -10.155693,12.234247 -13.698617,19.319547 -5.025637,10.07139 -8.075823,21.0698 -9.404137,32.22982 -0.929057,7.84329 -1.028994,15.78377 -0.318528,23.64941 0.949874,10.43013 3.373453,20.75419 7.477269,30.40181 3.147723,7.39261 7.291819,14.39427 12.530953,20.50107 4.609017,5.36885 10.09446,10.04216 16.358941,13.36913 4.992823,2.65991 10.488914,4.41802 16.127704,4.96522 5.401312,0.5374 10.902307,-0.0494 16.08534,-1.65207 6.288727,-1.93251 12.0804339,-5.30822 17.1026144,-9.53253 6.064976,-5.09947 11.0564829,-11.39537 15.0277086,-18.2327 4.799013,-8.2768 8.130738,-17.3605 10.168206,-26.69612 2.118236,-9.73165 2.840671,-19.75239 2.304177,-29.69265 C 22.160684,140.44793 19.945118,129.47219 15.878226,119.18494 12.84973,111.538 8.7902785,104.26673 3.5902453,97.87971 -0.94233546,92.318409 -6.3678931,87.429138 -12.615394,83.86693 c -4.987546,-2.851123 -10.510897,-4.811512 -16.219249,-5.535287 -1.70344,-0.217795 -3.420686,-0.327822 -5.138013,-0.327737 z m -1.517578,32.353514 c 4.550672,-0.0101 9.094595,1.08692 13.14694,3.15536 4.702214,2.38193 8.725725,5.98849 11.857042,10.20877 4.0837553,5.50159 6.710819,12.02521 7.8652338,18.76398 1.5657554,9.13429 0.4261756,18.74427 -3.4026815,27.19648 -2.3513356,5.18393 -5.7347853,9.9412 -10.0798093,13.64163 -3.563154,3.04205 -7.795765,5.33251 -12.351895,6.46599 -5.707319,1.43674 -11.853271,1.07808 -17.348531,-1.0333 -4.51594,-1.72078 -8.557357,-4.5744 -11.860694,-8.08549 -4.240834,-4.50063 -7.313413,-10.03709 -9.181653,-15.91595 -3.370435,-10.59683 -2.853975,-22.40133 1.537148,-32.62729 2.368672,-5.51241 5.8866,-10.57857 10.472304,-14.47504 3.636508,-3.09467 7.970078,-5.40501 12.626743,-6.50765 2.19857,-0.52632 4.459389,-0.78826 6.719853,-0.78749 z"
       inkscape:path-effect="#path-effect1640"
       inkscape:original-d="m -8.8660351,150.50538 a 26.624529,33.2248 0 0 1 -26.6245299,33.2248 26.624529,33.2248 0 0 1 -26.624526,-33.2248 26.624529,33.2248 0 0 1 26.624526,-33.2248 26.624529,33.2248 0 0 1 26.6245299,33.2248 z m 38.7255801,7.23686 a 63.83276,86.662025 0 0 1 -63.83276,86.66202 63.83276,86.662025 0 0 1 -63.832757,-86.66202 63.83276,86.662025 0 0 1 63.832757,-86.662029 63.83276,86.662025 0 0 1 63.83276,86.662029 z" />
    <path
       id="path1652"
       style="fill:#c83737;fill-rule:evenodd;stroke:#000000;stroke-width:10;stroke-miterlimit:4;stroke-dasharray:none"
       d="m 286.51758,-634.54297 c -82.7488,0 -139.16992,88.5797 -139.16992,182.39453 0,77.17355 38.56381,150.90133 98.89648,174.40235 26.92684,54.51299 76.24006,94.60352 136.62695,94.60351 92.91988,2e-5 160.0293,-93.60453 160.0293,-197.23242 0,-101.80178 -64.78448,-193.86283 -155.16016,-197.07812 -24.00138,-33.4152 -58.94309,-57.08985 -101.22265,-57.08985 z m 62.12304,138.19922 c 3.77116,13.43084 5.98829,28.27401 5.98829,44.19531 0,60.06079 -28.04491,100.45697 -56.36329,109.42578 -2.71748,-11.87751 -4.36328,-24.43301 -4.36328,-37.65234 0,-55.38244 24.38167,-97.61537 54.73828,-115.96875 z"
       inkscape:path-effect="#path-effect1654"
       inkscape:original-d="M 507.37089,-380.37454 A 124.49909,161.70192 0 0 1 382.8718,-218.67263 124.49909,161.70192 0 0 1 258.37271,-380.37454 124.49909,161.70192 0 0 1 382.8718,-542.07646 124.49909,161.70192 0 0 1 507.37089,-380.37454 Z m -117.21342,-71.77477 a 103.63992,146.86452 0 0 1 -103.63992,146.86452 103.63992,146.86452 0 0 1 -103.63992,-146.86452 103.63992,146.86452 0 0 1 103.63992,-146.86452 103.63992,146.86452 0 0 1 103.63992,146.86452 z" />
    <path
       id="path1622"
       style="fill:#c83737;fill-rule:evenodd;stroke:#000000;stroke-width:10;stroke-miterlimit:4;stroke-dasharray:none"
       d="m 253.23047,117.94336 c -8.21232,-0.008 -16.33207,2.23922 -23.61486,5.9722 -8.75629,4.46593 -16.37504,10.92827 -22.82397,18.30305 -8.59298,9.85166 -15.1784,21.34547 -20.08886,33.42983 -7.02407,17.34467 -10.62397,35.95864 -11.51127,54.61561 -0.36242,7.73238 -0.27317,15.48881 0.32663,23.20701 0.5023,4.81173 0.80941,9.65254 1.67413,14.41885 0.67446,4.15296 1.36299,8.30895 2.38576,12.39338 1.04419,4.55095 2.28032,9.05773 3.74722,13.49102 1.7968,5.51545 3.92941,10.92065 6.39438,16.17184 1.69541,3.7229 3.68393,7.30342 5.71733,10.84968 1.55602,2.61333 3.20028,5.17523 4.96148,7.65534 -0.5987,-8.02476 -0.87506,-16.08157 -0.57674,-24.12629 0.21755,-6.23728 0.61386,-12.4704 1.3115,-18.67345 1.91473,-17.00585 5.65763,-33.82136 11.37506,-49.95802 8.31286,-23.49785 20.74458,-45.67966 37.40799,-64.28469 12.57458,-14.04299 27.63466,-25.97925 44.60133,-34.27395 -6.89324,-6.82611 -15.00394,-12.58264 -24.13572,-15.98406 -3.77806,-1.42066 -7.72594,-2.37476 -11.7327,-2.85775 -1.7969,-0.22036 -3.60797,-0.34548 -5.41869,-0.3496 z m 120.9043,61.45703 c 2.57801,9.30501 4.52631,18.78251 5.86005,28.34504 0.72139,4.80451 1.16231,9.64642 1.55078,14.48767 0.32635,3.23407 0.4413,6.4852 0.57591,9.73162 0.27148,7.14632 0.11285,14.30307 -0.32905,21.43925 -0.35072,6.19986 -1.02777,12.37887 -1.90349,18.5256 -0.68574,4.80995 -1.53374,9.59632 -2.55304,14.34683 -1.39849,6.74577 -3.13452,13.42126 -5.2396,19.98148 -1.12679,3.56333 -2.316,7.10755 -3.6481,10.59997 -2.41751,6.37406 -5.09228,12.65694 -8.16055,18.74643 -3.80732,7.58738 -8.11384,14.92796 -12.96587,21.89568 -3.08798,4.47895 -6.45957,8.7597 -10.00814,12.88186 -2.71992,3.1534 -5.57783,6.1876 -8.5818,9.07205 -3.91112,3.78905 -8.04379,7.35472 -12.40728,10.6139 -2.30491,1.64777 -4.58411,3.33633 -7.0007,4.82003 -3.9246,2.5367 -8.02474,4.80065 -12.24154,6.81298 -2.67488,1.24716 -5.3742,2.44764 -8.15262,3.44807 -2.16069,0.86645 -4.37865,1.57548 -6.61118,2.23096 -0.6656,0.20679 -1.32442,0.43587 -1.99628,0.62214 6.5525,8.85771 14.27738,16.8993 23.17405,23.4257 2.79299,2.06018 5.72141,3.93457 8.71168,5.6943 5.29109,2.96355 10.85007,5.48456 16.66701,7.22585 2.51846,0.82503 5.10847,1.42098 7.71172,1.90808 5.595,1.01236 11.30437,1.49215 16.98677,1.14146 10.62171,-0.59637 21.03516,-3.6854 30.46145,-8.57379 11.0467,-5.71387 20.77061,-13.78945 28.95145,-23.11842 10.40371,-11.85903 18.37699,-25.73171 24.11442,-40.39551 6.82925,-17.49712 10.49073,-36.16009 11.37655,-54.90088 0.32683,-6.64366 0.28328,-13.3037 -0.11142,-19.94342 -0.37581,-7.42057 -1.27973,-14.80803 -2.53951,-22.12833 -0.98986,-5.63094 -2.32201,-11.1983 -3.8643,-16.70214 -4.87002,-17.09959 -12.52115,-33.48957 -23.22208,-47.72877 -9.10071,-12.0918 -20.49793,-22.63935 -33.91524,-29.76322 -3.44652,-1.82817 -7.01838,-3.42235 -10.69005,-4.74247 z"
       inkscape:original-d="M 474.0833,311.19658 A 124.49909,161.70192 0 0 1 349.58421,472.89849 124.49909,161.70192 0 0 1 225.08512,311.19658 124.49909,161.70192 0 0 1 349.58421,149.49465 124.49909,161.70192 0 0 1 474.0833,311.19658 Z M 356.86988,239.42181 A 103.63992,146.86452 0 0 1 253.22996,386.28633 103.63992,146.86452 0 0 1 149.59004,239.42181 103.63992,146.86452 0 0 1 253.22996,92.557292 103.63992,146.86452 0 0 1 356.86988,239.42181 Z"
       inkscape:path-effect="#path-effect1642" />
    <g
       id="g1660"
       inkscape:path-effect="#path-effect1662"
       transform="translate(-103.6134,-636.91437)">
      <ellipse
         style="fill:none;fill-rule:evenodd;stroke:#000000;stroke-width:10;stroke-miterlimit:4;stroke-dasharray:none"
         id="ellipse1656"
         cx="-54.396549"
         cy="390.94354"
         rx="127.62676"
         ry="101.8336"
         d="m -54.396484,255.45312 c -84.772646,-1e-5 -161.283206,56.80926 -161.283206,135.49024 0,78.68098 76.51057,135.49023 161.283206,135.49023 84.772634,0 161.283204,-56.80925 161.283204,-135.49023 0,-78.68098 -76.510575,-135.49024 -161.283204,-135.49024 z" />
      <path
         id="path1658"
         style="fill:none;fill-rule:evenodd;stroke:#000000;stroke-width:10;stroke-miterlimit:4;stroke-dasharray:none"
         inkscape:transform-center-x="-2.1853814"
         inkscape:transform-center-y="-4.9767892"
         d="m 5.3066406,127.28125 -52.4882816,84.86719 -99.667969,4.875 64.494141,76.14258 -26.162111,96.29687 92.345705,-37.80664 83.498047,54.63867 L 59.908203,306.7832 137.67383,244.25781 40.742188,220.5625 Z"
         inkscape:original-d="m 28.693756,340.79243 -41.24201,-26.98797 -45.612767,18.67429 12.922603,-47.56323 -31.855423,-37.60964 49.228618,-2.40772 25.9250314,-41.91833 17.5023566,46.07517 47.877974,11.7027 -38.411567,30.88374 z" />
    </g>
    <g
       id="g1634"
       inkscape:path-effect="#path-effect1636"
       transform="translate(-136.90099,54.656749)">
      <ellipse
         style="fill:none;fill-rule:evenodd;stroke:#000000;stroke-width:10;stroke-miterlimit:4;stroke-dasharray:none"
         id="path1627"
         cx="-54.396549"
         cy="390.94354"
         rx="127.62676"
         ry="101.8336"
         d="m -54.396484,314.00195 c -14.461769,-0.002 -28.963005,2.18361 -42.65465,6.894 -12.910866,4.44143 -25.135806,11.14456 -35.382786,20.21243 -7.31327,6.47563 -13.60662,14.18642 -18.03401,22.9175 -3.57122,7.0288 -5.88459,14.7293 -6.49385,22.60181 -0.5723,7.17418 0.31289,14.44303 2.4539,21.30802 2.63293,8.48866 7.12506,16.33797 12.75188,23.19245 6.68632,8.14824 14.95183,14.93387 23.99204,20.31724 13.93752,8.29079 29.696453,13.28391 45.748793,15.33203 9.897767,1.25871 19.930507,1.43736 29.869777,0.57702 14.144523,-1.23449 28.136913,-4.68607 41.0528687,-10.62578 10.901156,-5.0161 21.0529853,-11.82122 29.4386983,-20.43806 6.697934,-6.88352 12.255103,-14.9593 15.800034,-23.91019 2.816116,-7.09165 4.32134,-14.72705 4.183849,-22.36576 -0.125193,-7.90236 -1.995774,-15.73793 -5.208249,-22.94624 -3.930311,-8.84073 -9.800395,-16.74369 -16.752943,-23.44034 -8.388333,-8.0776 -18.3423941,-14.45054 -28.9750716,-19.15276 -14.6888184,-6.49315 -30.6765114,-9.81437 -46.6978884,-10.38279 -1.696742,-0.0606 -3.394571,-0.0906 -5.092392,-0.0906 z" />
      <path
         id="path1629"
         style="fill:none;fill-rule:evenodd;stroke:#000000;stroke-width:10;stroke-miterlimit:4;stroke-dasharray:none"
         inkscape:transform-center-x="-2.1853814"
         inkscape:transform-center-y="-4.9767892"
         d="m -7.2988281,258.9707 c -2.093099,3.38412 -4.1861979,6.76823 -6.2792969,10.15235 -3.973307,0.19401 -7.946615,0.38802 -11.919922,0.58203 2.570964,3.03581 5.141927,6.07161 7.712891,9.10742 -1.042969,3.83919 -2.085937,7.67839 -3.128906,11.51758 3.68164,-1.50716 7.363281,-3.01433 11.0449214,-4.52149 3.3294271,2.17904 6.6588541,4.35807 9.98828122,6.53711 -0.29557291,-3.9681 -0.59114583,-7.93619 -0.88671874,-11.90429 3.10026042,-2.49284 6.20052082,-4.98568 9.30078122,-7.47852 -3.8645833,-0.94466 -7.72916667,-1.88932 -11.59375,-2.83398 -1.4127604,-3.7194 -2.8255208,-7.43881 -4.2382812,-11.15821 z"
         inkscape:original-d="m 28.693756,340.79243 -41.24201,-26.98797 -45.612767,18.67429 12.922603,-47.56323 -31.855423,-37.60964 49.228618,-2.40772 25.9250314,-41.91833 17.5023566,46.07517 47.877974,11.7027 -38.411567,30.88374 z" />
    </g>
  </g>
</svg>
)"""";

   testDoc(svg);
}