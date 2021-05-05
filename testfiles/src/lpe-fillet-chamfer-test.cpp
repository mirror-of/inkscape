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

class LPEFilletChamferTest : public LPESPathsTest {};
// INKSCAPE 1.0.2

TEST_F(LPEFilletChamferTest, multi_PX_1_0_2)
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
   id="svg8"
   inkscape:version="1.0.2 (e86c870879, 2021-01-15)"
   sodipodi:docname="1.svg">
  <defs
     id="defs2">
    <inkscape:path-effect
       effect="fillet_chamfer"
       id="path-effect847"
       is_visible="true"
       lpeversion="1"
       satellites_param="IF,0,0,1,0,0,0,1 @ IF,0,0,1,0,52.916667,0,1 @ IF,0,0,1,0,52.916667,0,1 @ IF,0,0,1,0,52.916667,0,1 @ IF,0,0,1,0,52.916667,0,1 @ IF,0,0,1,0,52.916667,0,1 @ IF,0,0,1,0,0,0,1"
       unit="px"
       method="auto"
       mode="IF"
       radius="200"
       chamfer_steps="1"
       flexible="false"
       use_knot_distance="true"
       apply_no_radius="true"
       apply_with_radius="true"
       only_selected="false"
       hide_knots="false" />
    <inkscape:path-effect
       effect="fillet_chamfer"
       id="path-effect843"
       is_visible="true"
       lpeversion="1"
       satellites_param="IF,0,0,1,0,0,0,1 @ IF,0,0,1,0,13.229167,0,1 @ F,0,0,1,0,13.229167,0,1 @ IF,0,0,1,0,13.229167,0,1 @ IF,0,0,1,0,13.229167,0,1 @ IF,0,0,1,0,13.229167,0,1 @ IF,0,0,1,0,0,0,1"
       unit="px"
       method="auto"
       mode="IF"
       radius="50"
       chamfer_steps="1"
       flexible="false"
       use_knot_distance="true"
       apply_no_radius="true"
       apply_with_radius="true"
       only_selected="false"
       hide_knots="false" />
    <inkscape:path-effect
       effect="fillet_chamfer"
       id="path-effect839"
       is_visible="true"
       lpeversion="1"
       satellites_param="IC,0,0,1,0,21.960417,0,1 @ IC,0,0,1,0,21.960417,0,1 @ IC,0,0,1,0,21.960417,0,1 @ IC,0,0,1,0,21.960417,0,1 @ IC,0,0,1,0,21.960417,0,1 @ IC,0,0,1,0,21.960417,0,1 @ IC,0,0,1,0,21.960417,0,1 @ IC,0,0,1,0,21.960417,0,1 @ IC,0,0,1,0,21.960417,0,1 @ IC,0,0,1,0,21.960417,0,1"
       unit="px"
       method="auto"
       mode="IC"
       radius="83"
       chamfer_steps="1"
       flexible="false"
       use_knot_distance="true"
       apply_no_radius="true"
       apply_with_radius="true"
       only_selected="false"
       hide_knots="false" />
    <inkscape:path-effect
       effect="fillet_chamfer"
       id="path-effect835"
       is_visible="true"
       lpeversion="1"
       satellites_param="F,0,0,1,0,47.030329,0,4 @ F,0,0,1,0,45.435742,0,4 @ C,0,0,1,0,33.56938,0,4 @ IF,0,0,1,0,40.518268,0,4"
       unit="px"
       method="auto"
       mode="F"
       radius="0"
       chamfer_steps="4"
       flexible="false"
       use_knot_distance="true"
       apply_no_radius="true"
       apply_with_radius="true"
       only_selected="false"
       hide_knots="false" />
  </defs>
  <sodipodi:namedview
     id="base"
     pagecolor="#ffffff"
     bordercolor="#666666"
     borderopacity="1.0"
     inkscape:pageopacity="0.0"
     inkscape:pageshadow="2"
     inkscape:zoom="0.35"
     inkscape:cx="400"
     inkscape:cy="560"
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
     id="metadata5">
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
       style="fill:none;fill-rule:evenodd;stroke:#000000;stroke-width:22.2253;stroke-miterlimit:4;stroke-dasharray:none"
       id="rect833"
       width="562.28693"
       height="334.01868"
       x="-1.3634367"
       y="201.31145"
       inkscape:path-effect="#path-effect835"
       d="m 45.666892,201.31145 469.820858,0 a 45.435742,45.435742 45 0 1 45.43574,45.43574 v 255.01355 l -2.55532,12.84645 -7.27692,10.89069 -10.89069,7.27693 -12.84645,2.55531 H 39.154831 A 40.518268,40.518268 45 0 0 -1.3634367,494.81186 V 248.34178 A 47.030329,47.030329 135 0 1 45.666892,201.31145 Z"
       sodipodi:type="rect" />
    <path
       sodipodi:type="star"
       style="fill:none;fill-rule:evenodd;stroke:#000000;stroke-width:22.2253;stroke-miterlimit:4;stroke-dasharray:none"
       id="path837"
       sodipodi:sides="5"
       sodipodi:cx="42.101727"
       sodipodi:cy="793.948"
       sodipodi:r1="212.19016"
       sodipodi:r2="106.09508"
       sodipodi:arg1="0.7570233"
       sodipodi:arg2="1.3853418"
       inkscape:flatsided="false"
       inkscape:rounded="0"
       inkscape:randomized="0"
       d="m 175.35038,933.21223 -92.696558,-28.52877 -38.209091,7.16842 -76.052415,60.18781 -17.562647,-8.32968 -1.512338,-96.97554 -18.624829,-34.12384 -80.743502,-53.73109 2.49484,-19.27708 91.761881,-31.40541 26.698313,-28.25811 26.1501869,-93.39545 19.1045451,-3.58421 58.224297,77.56592 35.125297,16.65937 96.9052,-3.99047 9.31242,17.06192 -55.77728,79.34378 -4.98969,38.55417 33.74052,90.9292 z"
       inkscape:transform-center-x="15.546215"
       inkscape:transform-center-y="-10.82121"
       inkscape:path-effect="#path-effect839" />
    <path
       style="fill:none;stroke:#000000;stroke-width:0.588044px;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1"
       d="m 376.19127,678.13316 227.43612,321.58166 c 6.07773,-4.05497 14.42571,-2.50497 18.64452,3.46018 75.03925,-50.06513 510.55729,-340.87433 582.78879,-394.54405 5.8411,-4.34005 5.2143,-11.67583 -1.3956,-14.6579 -64.1816,-28.95605 -376.56704,-145.6057 -442.90301,-170.31909 -0.99174,7.23836 -7.34681,11.0426 -14.19338,8.49192 l -43.3565,316.44542 a 7.6639148,7.6639148 37.886125 0 1 10.46757,8.14471 L 1050.9269,620.27852 a 23.24385,23.24385 128.32444 0 1 14.2594,-18.03972 l 62.2276,-407.69667"
       id="path841"
       inkscape:path-effect="#path-effect843"
       inkscape:original-d="m 376.19127,678.13316 235.07496,332.38254 c 0,0 603.94317,-402.60801 603.94317,-410.28794 0,-7.67994 -466.84493,-181.1886 -466.84493,-181.1886 l -46.94802,342.65886 361.77375,-146.38151 64.2237,-420.77438" />
    <path
       style="fill:none;stroke:#000000;stroke-width:0.588044px;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1"
       d="m 629.29243,-26.076168 782.38637,-17.467019 a 30.913407,30.913407 58.428006 0 0 27.9507,45.4829164 L 1229.4913,394.9164 a 45.817982,45.817982 167.24724 0 0 -67.5673,15.29234 L 839.40548,172.77895 c 20.81728,-20.51208 17.78144,-52.00447 -5.75156,-69.32885 88.29628,-87.001828 310.80968,-282.10896 395.06658,-131.264643 -2.3684,29.0529797 9.9075,76.228716 24.0899,101.619458 C 1233.7309,307.85697 650.67002,396.81001 599.83777,196.53785 608.77238,168.92517 617.64596,120.88347 610.46331,92.584803 685.34099,-138.82699 977.83584,-134.2229 977.83584,-134.2229"
       id="path845"
       inkscape:path-effect="#path-effect847"
       inkscape:original-d="M 629.29243,-26.076168 1464.5823,-44.724274 1204.5385,441.5804 796.79102,141.40729 c 0,0 373.70708,-412.2173 451.04898,-120.027056 C 1325.1819,313.57048 552.93026,427.51163 598.22727,144.01321 643.5243,-139.48522 977.83584,-134.2229 977.83584,-134.2229" />
  </g>
</svg>
)"""";

   testDoc(svg);
}

TEST_F(LPEFilletChamferTest, multi_MM_1_0_2)
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
   viewBox="0 0 210 297"
   version="1.1"
   id="svg8"
   inkscape:version="1.0.2 (e86c870879, 2021-01-15)"
   sodipodi:docname="1.svg">
  <defs
     id="defs2">
    <inkscape:path-effect
       effect="fillet_chamfer"
       id="path-effect847"
       is_visible="true"
       lpeversion="1"
       satellites_param="IF,0,0,1,0,0,0,1 @ IF,0,0,1,0,52.916667,0,1 @ IF,0,0,1,0,52.916667,0,1 @ IF,0,0,1,0,52.916667,0,1 @ IF,0,0,1,0,52.916667,0,1 @ IF,0,0,1,0,52.916667,0,1 @ IF,0,0,1,0,0,0,1"
       unit="px"
       method="auto"
       mode="F"
       radius="200"
       chamfer_steps="1"
       flexible="false"
       use_knot_distance="true"
       apply_no_radius="true"
       apply_with_radius="true"
       only_selected="false"
       hide_knots="false" />
    <inkscape:path-effect
       effect="fillet_chamfer"
       id="path-effect843"
       is_visible="true"
       lpeversion="1"
       satellites_param="IF,0,0,1,0,0,0,1 @ IF,0,0,1,0,13.229167,0,1 @ IF,0,0,1,0,13.229167,0,1 @ IF,0,0,1,0,13.229167,0,1 @ IF,0,0,1,0,13.229167,0,1 @ IF,0,0,1,0,13.229167,0,1 @ IF,0,0,1,0,0,0,1"
       unit="px"
       method="auto"
       mode="F"
       radius="50"
       chamfer_steps="1"
       flexible="false"
       use_knot_distance="true"
       apply_no_radius="true"
       apply_with_radius="true"
       only_selected="false"
       hide_knots="false" />
    <inkscape:path-effect
       effect="fillet_chamfer"
       id="path-effect839"
       is_visible="true"
       lpeversion="1"
       satellites_param="IC,0,0,1,0,21.960417,0,1 @ IC,0,0,1,0,21.960417,0,1 @ IC,0,0,1,0,21.960417,0,1 @ IC,0,0,1,0,21.960417,0,1 @ IC,0,0,1,0,21.960417,0,1 @ IC,0,0,1,0,21.960417,0,1 @ IC,0,0,1,0,21.960417,0,1 @ IC,0,0,1,0,21.960417,0,1 @ IC,0,0,1,0,21.960417,0,1 @ IC,0,0,1,0,21.960417,0,1"
       unit="px"
       method="auto"
       mode="F"
       radius="83"
       chamfer_steps="1"
       flexible="false"
       use_knot_distance="true"
       apply_no_radius="true"
       apply_with_radius="true"
       only_selected="false"
       hide_knots="false" />
    <inkscape:path-effect
       effect="fillet_chamfer"
       id="path-effect835"
       is_visible="true"
       lpeversion="1"
       satellites_param="F,0,0,1,0,47.030329,0,4 @ F,0,0,1,0,45.435742,0,4 @ C,0,0,1,0,33.56938,0,4 @ IF,0,0,1,0,40.518268,0,4"
       unit="px"
       method="auto"
       mode="F"
       radius="0"
       chamfer_steps="4"
       flexible="false"
       use_knot_distance="true"
       apply_no_radius="true"
       apply_with_radius="true"
       only_selected="false"
       hide_knots="false" />
  </defs>
  <sodipodi:namedview
     id="base"
     pagecolor="#ffffff"
     bordercolor="#666666"
     borderopacity="1.0"
     inkscape:pageopacity="0.0"
     inkscape:pageshadow="2"
     inkscape:zoom="0.35"
     inkscape:cx="400"
     inkscape:cy="557.14286"
     inkscape:document-units="mm"
     inkscape:current-layer="layer1"
     inkscape:document-rotation="0"
     showgrid="false"
     inkscape:window-width="1920"
     inkscape:window-height="1016"
     inkscape:window-x="0"
     inkscape:window-y="27"
     inkscape:window-maximized="1" />
  <metadata
     id="metadata5">
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
       style="fill:none;fill-rule:evenodd;stroke:#000000;stroke-width:10;stroke-miterlimit:4;stroke-dasharray:none"
       id="rect833"
       width="252.9938"
       height="150.28743"
       x="-77.40638"
       y="16.580982"
       inkscape:path-effect="#path-effect835"
       d="M -30.376051,16.580982 H 130.15168 a 45.435742,45.435742 45 0 1 45.43575,45.435742 v 71.282306 l -2.55532,12.84645 -7.27693,10.89069 -10.89069,7.27693 -12.84644,2.55531 H -36.888112 A 40.518268,40.518268 45 0 0 -77.40638,126.35014 V 63.611311 a 47.030329,47.030329 135 0 1 47.030329,-47.030329 z"
       sodipodi:type="rect" />
    <path
       sodipodi:type="star"
       style="fill:none;fill-rule:evenodd;stroke:#000000;stroke-width:10;stroke-miterlimit:4;stroke-dasharray:none"
       id="path837"
       sodipodi:sides="5"
       sodipodi:cx="-57.849785"
       sodipodi:cy="283.23022"
       sodipodi:r1="95.472244"
       sodipodi:r2="47.736122"
       sodipodi:arg1="0.7570233"
       sodipodi:arg2="1.3853418"
       inkscape:flatsided="false"
       inkscape:rounded="0"
       inkscape:randomized="0"
       d="m -9.4414607,342.33717 -18.6172283,-5.72973 -38.209091,7.16842 -15.274411,12.08815 -17.562647,-8.32968 -0.303739,-19.47662 -18.624833,-34.12384 -16.21657,-10.79139 2.49485,-19.27708 18.4295,-6.30748 26.698316,-28.25811 5.252019,-18.7576 19.104545,-3.58421 11.693801,15.57838 35.125295,16.65937 19.4624972,-0.80145 9.3124148,17.06192 -11.202341,15.93545 -4.9896874,38.55417 6.7764664,18.26228 z"
       inkscape:transform-center-x="5.44007"
       inkscape:transform-center-y="-3.7866475"
       inkscape:path-effect="#path-effect839" />
    <path
       style="fill:none;stroke:#000000;stroke-width:0.264583px;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1"
       d="m 92.46953,231.12081 98.13013,138.75038 c 6.07682,-4.05633 14.42469,-2.5065 18.64351,3.45866 44.03613,-29.39451 207.24585,-138.47638 250.27605,-169.20695 -6.71967,-2.82497 -7.33665,-9.53058 -1.41683,-13.7583 -36.73902,-15.44525 -148.34111,-57.2327 -185.78178,-71.20137 -0.99176,7.23849 -7.34691,11.04219 -14.19227,8.48827 L 240.5962,255.61303 a 7.6639145,7.6639145 37.886124 0 1 10.46757,8.14471 l 138.2488,-55.93847 a 23.243851,23.243851 128.32444 0 1 14.25941,-18.03972 L 430.47253,13.535211"
       id="path841"
       inkscape:path-effect="#path-effect843"
       inkscape:original-d="M 92.46953,231.12081 198.2385,380.67207 c 0,0 271.7365,-181.14831 271.7365,-184.6038 0,-3.45549 -210.0509,-81.52349 -210.0509,-81.52349 L 238.80044,268.71975 401.5759,202.85726 430.47253,13.535211" />
    <path
       style="fill:none;stroke:#000000;stroke-width:0.264583px;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1"
       d="m 206.34916,-85.729169 322.92453,-7.20939 a 30.913407,30.913407 58.428006 0 0 27.95064,45.482916 l -67.0977,125.47856 A 45.817985,45.817985 167.24724 0 0 422.55932,93.315261 L 324.32748,20.99953 c 21.79366,-19.4715398 18.71636,-51.05189 -4.81044,-68.371712 38.78753,-34.654704 98.64238,-78.484008 136.38097,-59.525698 -13.76573,24.784298 -3.00976,80.913492 22.43831,93.697555 -41.17539,74.133594 -215.56109,102.593833 -270.31122,52.137358 14.15064,-24.109478 23.05863,-78.113161 1.69301,-97.803225 45.32541,-77.224238 153.4537,-75.522218 153.4537,-75.522218"
       id="path845"
       inkscape:path-effect="#path-effect847"
       inkscape:original-d="M 206.34916,-85.729169 582.17717,-94.119646 465.17379,124.68692 281.71301,-10.372129 c 0,0 168.14472,-185.471901 202.94372,-54.004639 C 519.45574,67.090496 171.99098,118.35686 192.37179,-9.1996275 212.75261,-136.75612 363.17181,-134.38841 363.17181,-134.38841" />
  </g>
</svg>
)"""";

   testDoc(svg);
}