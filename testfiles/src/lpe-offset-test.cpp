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
   inkscape:version="1.2-dev (99544120e7, 2021-04-09)"
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
       sodipodi:cx="-220.51343"
       sodipodi:cy="-560.59009"
       sodipodi:r1="133.35405"
       sodipodi:r2="66.677025"
       sodipodi:arg1="0.57435009"
       sodipodi:arg2="1.2026686"
       inkscape:flatsided="false"
       inkscape:rounded="0"
       inkscape:randomized="0"
       d="m -280.26758,-715.50977 -9.94726,109.81446 -96.10157,54.0625 101.36719,43.39453 21.71875,108.10351 72.59375,-82.99609 109.525392,12.75 -56.500002,-94.68945 45.968752,-100.22266 -107.511722,24.47461 z"
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
       d="m -345.13086,-0.06445312 -5.5332,61.10351512 -53.47461,30.082032 56.40234,24.146486 12.08399,60.15039 40.39453,-46.18164 60.9414,7.09375 -31.4375,-52.685549 25.58008,-55.767578 -59.82422,13.619141 z"
       inkscape:transform-center-x="10.601544"
       inkscape:transform-center-y="2.2228879"
       inkscape:path-effect="#path-effect1638" />
    <path
       id="path1648"
       style="fill:none;fill-rule:evenodd;stroke:#000000;stroke-width:10;stroke-miterlimit:4;stroke-dasharray:none"
       d="m 57.394531,-587.76172 c -43.587753,0 -75.947268,46.39391 -75.947265,98.77734 2e-6,52.38345 32.359518,98.77735 75.947265,98.77735 43.587759,10e-6 75.949209,-46.39359 75.949219,-98.77735 0,-52.38374 -32.36146,-98.77735 -75.949219,-98.77734 z m -1.517578,70.43164 c 6.597341,0 14.509766,7.96603 14.509766,21.10938 0,13.14335 -7.912433,21.10937 -14.509766,21.10937 -6.597347,0 -14.507812,-7.96517 -14.507812,-21.10937 0,-13.1442 7.910474,-21.10938 14.507812,-21.10938 z"
       inkscape:path-effect="#path-effect1650"
       inkscape:original-d="m 82.502297,-496.22143 a 26.624529,33.2248 0 0 1 -26.62453,33.2248 26.624529,33.2248 0 0 1 -26.624526,-33.2248 26.624529,33.2248 0 0 1 26.624526,-33.2248 26.624529,33.2248 0 0 1 26.62453,33.2248 z m 38.725583,7.23686 a 63.83276,86.662025 0 0 1 -63.832763,86.66202 63.83276,86.662025 0 0 1 -63.8327566,-86.66202 63.83276,86.662025 0 0 1 63.8327566,-86.66203 63.83276,86.662025 0 0 1 63.832763,86.66203 z" />
    <path
       id="path1616"
       inkscape:test-ignore="1"
       style="fill:none;fill-rule:evenodd;stroke:#000000;stroke-width:10;stroke-miterlimit:4;stroke-dasharray:none"
       d="m -33.972656,78.003906 c -30.490731,2e-6 -56.910157,34.460444 -56.910156,79.738284 0,45.27783 26.419426,79.73827 56.910156,79.73828 30.4907249,0 56.908201,-34.46028 56.908203,-79.73828 2e-6,-45.27802 -26.4174721,-79.738284 -56.908203,-79.738284 z m -1.517578,32.353514 c 19.301278,0 33.5488277,18.84678 33.5488278,40.14844 -10e-8,21.30166 -14.2475498,40.14844 -33.5488278,40.14844 -19.301278,0 -33.548828,-18.84678 -33.548828,-40.14844 0,-21.30166 14.24755,-40.14844 33.548828,-40.14844 z"
       inkscape:path-effect="#path-effect1640"
       inkscape:original-d="m -8.8660351,150.50538 a 26.624529,33.2248 0 0 1 -26.6245299,33.2248 26.624529,33.2248 0 0 1 -26.624526,-33.2248 26.624529,33.2248 0 0 1 26.624526,-33.2248 26.624529,33.2248 0 0 1 26.6245299,33.2248 z m 38.7255801,7.23686 a 63.83276,86.662025 0 0 1 -63.83276,86.66202 63.83276,86.662025 0 0 1 -63.832757,-86.66202 63.83276,86.662025 0 0 1 63.832757,-86.662029 63.83276,86.662025 0 0 1 63.83276,86.662029 z" />
    <path
       id="path1652"
       style="fill:#c83737;fill-rule:evenodd;stroke:#000000;stroke-width:10;stroke-miterlimit:4;stroke-dasharray:none"
       d="m 344.59766,-589.69922 c -82.74881,0 -139.16797,88.58024 -139.16797,182.39453 0,77.1741 38.56225,150.90404 98.89648,174.4043 26.92717,54.5123 76.24085,94.60156 136.62695,94.60156 92.91923,0 160.02735,-93.60308 160.02735,-197.23047 0,-101.79982 -64.78013,-193.86219 -155.1543,-197.08008 -24.00272,-33.41742 -58.94807,-57.08984 -101.22851,-57.08984 z m 62.125,138.19922 c 3.7701,13.43056 5.98632,28.27314 5.98632,44.19531 0,60.06079 -28.04491,100.45697 -56.36328,109.42578 -2.71778,-11.87711 -4.36327,-24.43193 -4.36328,-37.65039 1e-5,-55.38351 24.38298,-97.6175 54.74024,-115.9707 z"
       inkscape:path-effect="#path-effect1654"
       inkscape:original-d="M 565.45164,-335.53023 A 124.49909,161.70192 0 0 1 440.95255,-173.82832 124.49909,161.70192 0 0 1 316.45346,-335.53023 124.49909,161.70192 0 0 1 440.95255,-497.23215 124.49909,161.70192 0 0 1 565.45164,-335.53023 Z M 448.23822,-407.305 A 103.63992,146.86452 0 0 1 344.5983,-260.44048 103.63992,146.86452 0 0 1 240.95838,-407.305 103.63992,146.86452 0 0 1 344.5983,-554.16952 103.63992,146.86452 0 0 1 448.23822,-407.305 Z" />
    <path
       id="path1622"
       style="fill:#c83737;fill-rule:evenodd;stroke:#000000;stroke-width:10;stroke-miterlimit:4;stroke-dasharray:none"
       d="m 253.23047,117.94336 c -8.1994,-0.0123 -16.30529,2.23276 -23.57983,5.95126 -8.9613,4.57145 -16.73582,11.22136 -23.28275,18.81355 -8.11355,9.42689 -14.41457,20.32092 -19.18909,31.78094 -6.69899,16.1233 -10.403,33.40354 -11.68637,50.78866 -0.91991,12.62752 -0.61385,25.3484 0.96499,37.91166 1.38553,10.90021 3.77177,21.68371 7.30597,32.09229 3.97628,11.70978 9.41807,22.9694 16.57059,33.07766 -0.51833,-7.19445 -0.73001,-14.4131 -0.58906,-21.62525 0.50269,-25.78789 5.27494,-51.52153 14.42675,-75.65215 8.87503,-23.37293 21.91233,-45.32086 39.19761,-63.46183 11.82811,-12.41376 25.68992,-22.97584 41.1151,-30.52054 -7.01852,-6.94257 -15.29857,-12.78414 -24.62814,-16.15444 -5.3202,-1.93057 -10.96215,-2.98811 -16.62577,-3.00181 z m 121.01953,61.5 c 5.78299,20.92561 8.36747,42.70335 7.96005,64.39979 -0.27785,13.79222 -1.77797,27.56358 -4.56492,41.076 -2.39732,11.70055 -5.75985,23.20506 -10.08751,34.33809 -2.77356,7.10975 -5.94839,14.0638 -9.54257,20.79672 -5.63227,10.54306 -12.29994,20.55283 -20.07312,29.64184 -7.87939,9.21469 -16.9112,17.48634 -27.02574,24.1938 -8.40153,5.58111 -17.55262,10.055 -27.17587,13.09365 -1.13389,0.3581 -2.27246,0.70184 -3.41805,1.02066 8.51988,11.51974 19.07109,21.68819 31.50332,28.90471 7.33604,4.26696 15.32938,7.44117 23.65024,9.14899 5.4984,1.13504 11.13412,1.60887 16.74475,1.40414 10.43233,-0.38248 20.71458,-3.17811 30.07281,-7.76352 11.31817,-5.52655 21.30878,-13.55028 29.71328,-22.88861 10.1378,-11.26839 18.03576,-24.44432 23.85045,-38.41425 7.79631,-18.76331 11.85099,-38.98148 12.678,-59.25357 0.37893,-9.3159 0.0834,-18.6599 -0.88001,-27.93351 -2.03181,-19.21866 -7.07945,-38.18571 -15.61336,-55.55921 -7.56722,-15.39546 -17.9087,-29.59095 -31.11694,-40.62554 -7.94013,-6.62628 -16.93085,-12.06146 -26.67481,-15.58018 z"
       inkscape:original-d="M 474.0833,311.19658 A 124.49909,161.70192 0 0 1 349.58421,472.89849 124.49909,161.70192 0 0 1 225.08512,311.19658 124.49909,161.70192 0 0 1 349.58421,149.49465 124.49909,161.70192 0 0 1 474.0833,311.19658 Z M 356.86988,239.42181 A 103.63992,146.86452 0 0 1 253.22996,386.28633 103.63992,146.86452 0 0 1 149.59004,239.42181 103.63992,146.86452 0 0 1 253.22996,92.557292 103.63992,146.86452 0 0 1 356.86988,239.42181 Z"
       inkscape:path-effect="#path-effect1642" />
    <g
       id="g1660"
       inkscape:path-effect="#path-effect1662"
       transform="translate(-45.532653,-592.07006)">
      <ellipse
         style="fill:none;fill-rule:evenodd;stroke:#000000;stroke-width:10;stroke-miterlimit:4;stroke-dasharray:none"
         id="ellipse1656"
         cx="-54.396549"
         cy="390.94354"
         rx="127.62676"
         ry="101.8336"
         d="m -54.396484,255.45312 c -84.772636,0 -161.283206,56.80926 -161.283206,135.49024 0,78.68098 76.51057,135.49023 161.283206,135.49023 84.772634,0 161.283204,-56.80925 161.283204,-135.49023 0,-78.68098 -76.510569,-135.49025 -161.283204,-135.49024 z" />
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
         d="m -54.396484,314.00195 c -14.096794,-0.003 -28.226434,2.07482 -41.613185,6.54179 -12.363201,4.12361 -24.112211,10.30311 -34.167271,18.6292 -7.50222,6.22385 -14.05884,13.66773 -18.85643,22.17579 -3.97362,7.02954 -6.70566,14.79988 -7.68324,22.82957 -0.85571,6.89132 -0.37391,13.93505 1.337,20.66187 2.22467,8.81746 6.48653,17.04923 12.0069,24.24893 6.08757,7.94045 13.67126,14.67003 22.03665,20.129 12.03688,7.85156 25.674103,13.10254 39.729626,15.97198 11.663878,2.38075 23.641708,3.13803 35.520496,2.4525 14.291379,-0.83599 28.492134,-3.89481 41.6997731,-9.4628 11.1571444,-4.70717 21.6209659,-11.2198 30.4050439,-19.58609 6.594909,-6.28347 12.234937,-13.6341 16.208268,-21.8506 3.566446,-7.34795 5.745585,-15.40625 6.066137,-23.58122 0.295226,-7.1372 -0.844076,-14.31115 -3.182956,-21.05378 -3.014072,-8.71803 -7.960616,-16.69807 -14.057883,-23.59231 -7.294377,-8.25466 -16.209062,-15.00244 -25.8819988,-20.2308 -15.2473032,-8.2313 -32.3528282,-12.71794 -49.5970572,-13.93353 -3.317759,-0.23465 -6.643875,-0.34918 -9.969873,-0.3495 z" />
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