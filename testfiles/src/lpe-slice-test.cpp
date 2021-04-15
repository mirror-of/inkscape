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

class LPEBooleanOperationTest : public LPESPathsTest {};

// TEST fails if multiple simplify on stack


// INKSCAPE 1.0.2

TEST_F(LPEBooleanOperationTest, multi_PX_1_1)
{
   std::string svg = R""""(<?xml version="1.0" encoding="UTF-8" standalone="no"?>
<svg
   width="793.70081"
   height="1122.5197"
   viewBox="0 0 793.70081 1122.5197"
   version="1.1"
   id="svg5"
   inkscape:version="1.2-dev (99544120e7, 2021-04-09)"
   sodipodi:docname="1.svg"
   xmlns:inkscape="http://www.inkscape.org/namespaces/inkscape"
   xmlns:sodipodi="http://sodipodi.sourceforge.net/DTD/sodipodi-0.dtd"
   xmlns="http://www.w3.org/2000/svg"
   xmlns:svg="http://www.w3.org/2000/svg">
  <sodipodi:namedview
     id="namedview7"
     pagecolor="#ffffff"
     bordercolor="#666666"
     borderopacity="1.0"
     objecttolerance="10.0"
     gridtolerance="10.0"
     guidetolerance="10.0"
     inkscape:pageshadow="2"
     inkscape:pageopacity="0.0"
     inkscape:pagecheckerboard="0"
     inkscape:document-units="px"
     showgrid="false"
     inkscape:zoom="0.023329212"
     inkscape:cx="-12752.252"
     inkscape:cy="-5208.0627"
     inkscape:window-width="1920"
     inkscape:window-height="1016"
     inkscape:window-x="0"
     inkscape:window-y="27"
     inkscape:window-maximized="1"
     inkscape:current-layer="layer1"
     units="px"
     scale-x="1" />
  <defs
     id="defs2">
    <inkscape:path-effect
       effect="slice"
       start_point="142.62793,379.61328"
       end_point="142.62793,487.35742"
       center_point="142.62793,433.48535"
       id="path-effect1350"
       is_visible="true"
       lpeversion="1"
       allow_transforms="true" />
    <inkscape:path-effect
       effect="slice"
       start_point="53.212416,362.47607"
       end_point="53.212416,481.841"
       center_point="53.212416,422.15854"
       id="path-effect1309"
       is_visible="true"
       lpeversion="1"
       allow_transforms="true" />
    <inkscape:path-effect
       effect="slice"
       start_point="97.920442,362.47591"
       end_point="97.920442,487.35689"
       center_point="97.920442,424.9164"
       id="path-effect1294"
       is_visible="true"
       lpeversion="1"
       allow_transforms="true" />
    <inkscape:path-effect
       effect="slice"
       start_point="75.886874,768.34765"
       end_point="75.886874,1224.3066"
       center_point="75.886874,996.32713"
       id="path-effect1004"
       is_visible="true"
       lpeversion="1"
       allow_transforms="true" />
    <inkscape:path-effect
       effect="slice"
       start_point="193.63703,768.34699"
       end_point="193.63703,1224.3069"
       center_point="193.63703,996.32695"
       id="path-effect1001"
       is_visible="true"
       lpeversion="1"
       allow_transforms="true" />
    <inkscape:path-effect
       effect="slice"
       start_point="193.15391,114.33749"
       end_point="82.156209,136.48133"
       center_point="137.65506,125.40941"
       id="path-effect856"
       is_visible="true"
       lpeversion="1"
       allow_transforms="false" />
    <inkscape:path-effect
       effect="slice"
       start_point="103.76717,94.14931"
       end_point="10.442992,156.70153"
       center_point="57.10508,125.42542"
       id="path-effect851"
       is_visible="true"
       lpeversion="1"
       allow_transforms="false" />
    <inkscape:path-effect
       effect="slice"
       start_point="96.470909,61.288662"
       end_point="96.470909,188.54016"
       center_point="96.470909,124.91441"
       id="path-effect848"
       is_visible="true"
       lpeversion="1"
       allow_transforms="false" />
  </defs>
  <g
     inkscape:label="Layer 1"
     inkscape:groupmode="layer"
     id="layer1">
    <path
       id="path976"
       style="fill:none;fill-rule:evenodd;stroke:#000000;stroke-width:37.7953"
       inkscape:transform-center-x="23.78673"
       inkscape:transform-center-y="-110.89665"
       transform="matrix(5.6466942,0,0,5.6466942,394.39693,-1169.8226)"
       d="M 75.886874,928.62985 -41.863281,952.41211 75.886874,1055.1921 Z m 0,186.84605 -12.554843,108.8307 12.554843,-7.5091 z"
       inkscape:path-effect="#path-effect1001;#path-effect1004"
       inkscape:original-d="M 354.42488,1208.2816 204.6655,1139.7757 63.331136,1224.3069 82.20589,1060.7078 -41.862757,952.41236 119.56186,919.80841 184.21759,768.347 265.10873,911.79576 429.13682,926.48286 317.70568,1047.743 Z" />
    <path
       style="fill:none;fill-rule:evenodd;stroke:#000000;stroke-width:10;stroke-miterlimit:4;stroke-dasharray:none"
       id="path846"
       inkscape:path-effect="#path-effect848;#path-effect851"
       sodipodi:type="arc"
       sodipodi:cx="96.470909"
       sodipodi:cy="124.91441"
       sodipodi:rx="80.346069"
       sodipodi:ry="63.625751"
       d="M 96.470703,99.039898 V 61.289062 c -44.373909,0 -80.345703,28.485471 -80.345703,63.624998 0,8.49573 2.102672,16.60262 5.918392,24.01209 z"
       transform="matrix(21.341837,0,0,21.341837,4004.6518,-1123.6413)" />
    <path
       id="slice-1-path846"
       class="path846-slice UnoptimicedTransforms"
       d="m 22.043392,148.92615 c 11.966634,23.23714 40.781739,39.61487 74.427311,39.61487 V 99.039898 Z"
       style="display:inline;fill:none;fill-rule:evenodd;stroke:#000000;stroke-width:10;stroke-miterlimit:4;stroke-dasharray:none"
       transform="matrix(21.341837,0,0,21.341837,4004.6518,-1123.6413)" />
    <path
       id="slice-1-0-path846"
       class="path846-slice UnoptimicedTransforms"
       d="M 176.30545,117.69873 C 173.47666,97.853497 159.11937,80.773162 138.66406,70.757812 l -42.193357,28.28125 v 34.586548 z"
       style="display:inline;fill:none;fill-rule:evenodd;stroke:#000000;stroke-width:10;stroke-miterlimit:4;stroke-dasharray:none"
       inkscape:path-effect="#path-effect856"
       inkscape:original-d="m 96.470703,99.039898 v 89.501122 c 44.373807,-9e-5 80.345707,-28.48748 80.345707,-63.62696 -1e-5,-22.88349 -15.25519,-42.945104 -38.15148,-54.155607 z"
       transform="matrix(21.341837,0,0,21.341837,4004.6518,-1123.6413)" />
    <path
       id="slice-0-path846"
       class="path846-slice UnoptimicedTransforms"
       d="M 138.66493,70.758453 C 126.4021,64.754314 111.94744,61.289093 96.470703,61.289062 v 37.750836 z"
       style="display:inline;fill:none;fill-rule:evenodd;stroke:#000000;stroke-width:10;stroke-miterlimit:4;stroke-dasharray:none"
       transform="matrix(21.341837,0,0,21.341837,4004.6518,-1123.6413)" />
    <path
       id="slice-0-slice-1-0-path846"
       class="slice-1-0-path846-slice UnoptimicedTransforms"
       d="m 96.470703,133.62561 v 54.91541 c 44.373807,-9e-5 80.345707,-28.48748 80.345707,-63.62696 0,-2.43961 -0.1734,-4.84716 -0.51096,-7.21533 z"
       style="display:inline;fill:none;fill-rule:evenodd;stroke:#000000;stroke-width:10;stroke-miterlimit:4;stroke-dasharray:none"
       transform="matrix(21.341837,0,0,21.341837,4004.6518,-1123.6413)" />
    <path
       transform="matrix(5.6466942,0,0,5.6466942,1526.7835,-1579.9605)"
       id="slice-1-path976"
       class="path976-slice UnoptimicedTransforms"
       d="M 75.886874,1216.7975 193.63672,1146.3711 V 785.05273 l -9.41992,-16.70507 -64.6543,151.46093 -43.675626,8.82126 z m 0,-161.6054 6.318204,5.5149 -6.318204,54.7689 z"
       style="display:inline;fill:none;fill-rule:evenodd;stroke:#000000;stroke-width:37.7953" />
    <path
       transform="matrix(5.6466942,0,0,5.6466942,2889.5756,-945.94322)"
       id="slice-1-0-path976"
       class="path976-slice UnoptimicedTransforms"
       d="m 193.63672,785.05273 v 361.31837 l 11.0293,-6.5957 149.75976,68.5058 -36.7207,-160.539 111.43164,-121.25978 -164.02734,-14.6875 z"
       style="display:inline;fill:none;fill-rule:evenodd;stroke:#000000;stroke-width:37.7953" />
    <path
       transform="scale(0.26458333)"
       id="slice-0-path976"
       class="path976-slice UnoptimicedTransforms"
       d=""
       style="display:inline;fill:none;fill-rule:evenodd;stroke:#000000;stroke-width:37.7953" />
    <g
       id="g1292"
       inkscape:path-effect="#path-effect1294;#path-effect1309"
       transform="matrix(21.341837,0,0,21.341837,222.36533,-1331.2608)">
      <path
         sodipodi:type="star"
         style="fill:none;fill-rule:evenodd;stroke:#000000;stroke-width:37.7953;stroke-miterlimit:4;stroke-dasharray:none"
         id="path1111"
         inkscape:flatsided="false"
         sodipodi:sides="5"
         sodipodi:cx="269.10574"
         sodipodi:cy="1607.3273"
         sodipodi:r1="240.11061"
         sodipodi:r2="120.05531"
         sodipodi:arg1="0.79040597"
         sodipodi:arg2="1.4187245"
         inkscape:rounded="0"
         inkscape:randomized="0"
         d="m 201.1178,1469.9256 -16.47718,52.0861 -152.498042,46.5664 129.720702,92.7168 -2.83594,159.4239 42.09046,-31.0833 z"
         transform="scale(0.26458333)"
         inkscape:transform-center-x="2.9739241"
         inkscape:transform-center-y="-3.1681299" />
      <ellipse
         style="fill:none;fill-rule:evenodd;stroke:#000000;stroke-width:10;stroke-miterlimit:4;stroke-dasharray:none"
         id="path1135"
         cx="123.481"
         cy="453.47375"
         rx="42.992428"
         ry="33.773975"
         d="" />
      <path
         style="fill:none;fill-rule:evenodd;stroke:#000000;stroke-width:10;stroke-miterlimit:4;stroke-dasharray:none"
         id="rect1159"
         width="90.091156"
         height="63.399643"
         x="77.18277"
         y="379.61249"
         d=""
         sodipodi:type="rect" />
      <path
         id="path1259"
         style="fill:none;fill-rule:evenodd;stroke:#000000;stroke-width:37.7953"
         inkscape:transform-center-x="2.9739241"
         inkscape:transform-center-y="-3.1681299"
         transform="matrix(0.26458333,0,0,0.26458333,20.062621,0.10915402)"
         d="m 125.29057,1540.1347 -93.147992,28.4434 93.147992,66.5768 z"
         inkscape:original-d="m 438.03721,1777.9592 -150.74472,-51.9621 -128.26468,94.7217 2.83626,-159.4239 -129.721703,-92.7164 152.497633,-46.5672 48.09226,-152.0236 91.41245,130.6438 159.44436,-1.2394 -96.00163,127.3095 z" />
      <path
         id="ellipse1261"
         style="fill:none;fill-rule:evenodd;stroke:#000000;stroke-width:10"
         d=""
         inkscape:original-d="m 186.53605,453.58292 a 42.992428,33.773975 0 0 1 -42.99243,33.77397 42.992428,33.773975 0 0 1 -42.99242,-33.77397 42.992428,33.773975 0 0 1 42.99242,-33.77398 42.992428,33.773975 0 0 1 42.99243,33.77398 z" />
      <path
         id="rect1263"
         style="fill:none;fill-rule:evenodd;stroke:#000000;stroke-width:10"
         d=""
         inkscape:original-d="m 97.245392,379.72165 h 90.091158 v 63.39964 H 97.245392 Z" />
    </g>
    <g
       transform="matrix(21.341837,0,0,21.341837,222.36533,-1331.2608)"
       id="slice-1-g1292"
       class="g1292-slice UnoptimicedTransforms"
       style="display:inline">
      <path
         transform="scale(0.26458333)"
         id="path1311"
         d="m 201.1178,1789.6355 86.17517,-63.6394 82.80078,28.541 v -254.2637 l -45.94922,0.3575 -91.41211,-130.6426 -31.61462,99.9373 z"
         style="fill:none;fill-rule:evenodd;stroke:#000000;stroke-width:37.7953;stroke-miterlimit:4;stroke-dasharray:none" />
      <path
         id="path1313"
         d="m 97.919922,426.31445 c -10.576079,6.15312 -17.431641,16.02776 -17.431641,27.16016 0,11.1324 6.855562,21.0076 17.431641,27.16016 z"
         style="fill:none;fill-rule:evenodd;stroke:#000000;stroke-width:10;stroke-miterlimit:4;stroke-dasharray:none" />
      <path
         id="path1315"
         d="m 77.183594,379.61328 v 63.39844 h 20.736328 v -63.39844 z"
         style="fill:none;fill-rule:evenodd;stroke:#000000;stroke-width:10;stroke-miterlimit:4;stroke-dasharray:none" />
      <path
         transform="matrix(0.26458333,0,0,0.26458333,20.062621,0.10915402)"
         id="path1317"
         d="m 125.29057,1635.1549 36.57271,26.14 -2.83594,159.4239 128.26563,-94.7208 6.97265,2.4043 v -270.4726 l -61.5332,-87.9414 -48.0918,152.0234 -59.35005,18.123 z"
         style="fill:none;fill-rule:evenodd;stroke:#000000;stroke-width:37.7953" />
      <path
         id="path1319"
         d="" />
      <path
         id="path1321"
         d="m 97.246094,379.7207 v 63.40039 h 0.673828 V 379.7207 Z"
         style="fill:none;fill-rule:evenodd;stroke:#000000;stroke-width:10" />
    </g>
    <g
       id="slice-1-0-g1292"
       class="g1292-slice UnoptimicedTransforms"
       style="display:inline"
       inkscape:path-effect="#path-effect1350"
       transform="matrix(21.341837,0,0,21.341837,563.18096,-2123.0562)">
      <path
         transform="scale(0.26458333)"
         id="path1324"
         d="m 483.58984,1499.3926 -113.49609,0.8808 v 254.2657 l 67.94336,23.4199 -50.44922,-151.2578 z"
         style="fill:none;fill-rule:evenodd;stroke:#000000;stroke-width:37.7953;stroke-miterlimit:4;stroke-dasharray:none"
         inkscape:original-d="m 483.58984,1499.3926 -113.49609,0.8808 v 254.2657 l 67.94336,23.4199 -50.44922,-151.2578 z" />
      <path
         id="path1326"
         d="m 142.62793,423.22574 c -5.76586,-2.25703 -12.26795,-3.52652 -19.14746,-3.52652 -9.57312,0 -18.41593,2.45851 -25.560548,6.61523 v 54.32032 c 7.144618,4.15633 15.987428,6.61328 25.560548,6.61328 6.87951,0 13.3816,-1.26933 19.14746,-3.52613 z"
         style="fill:none;fill-rule:evenodd;stroke:#000000;stroke-width:10;stroke-miterlimit:4;stroke-dasharray:none"
         inkscape:original-d="m 123.48047,419.69922 c -9.57312,0 -18.41593,2.45851 -25.560548,6.61523 v 54.32032 c 7.144618,4.15633 15.987428,6.61328 25.560548,6.61328 23.74406,0 42.99219,-15.12059 42.99219,-33.77344 -1e-5,-18.65285 -19.24813,-33.77539 -42.99219,-33.77539 z" />
      <path
         id="path1328"
         d="M 142.62793,379.61328 H 97.919922 v 63.39844 h 44.708008 z"
         style="fill:none;fill-rule:evenodd;stroke:#000000;stroke-width:10;stroke-miterlimit:4;stroke-dasharray:none"
         inkscape:original-d="m 97.919922,379.61328 v 63.39844 h 69.353518 v -63.39844 z" />
      <path
         transform="matrix(0.26458333,0,0,0.26458333,20.062621,0.10915402)"
         id="path1330"
         d="m 463.23897,1499.5507 -139.09444,1.0802 -29.87891,-42.7012 v 270.4726 l 143.77149,49.5567 -50.44922,-151.2578 75.65108,-100.3212 z"
         style="fill:none;fill-rule:evenodd;stroke:#000000;stroke-width:37.7953"
         inkscape:original-d="m 294.26562,1457.9297 v 270.4726 l 143.77149,49.5567 -50.44922,-151.2578 96.00195,-127.3086 -159.44531,1.2383 z" />
      <path
         id="path1332"
         d="m 142.62793,419.81609 c -23.32157,0.38253 -42.07715,15.3533 -42.07715,33.76594 0,18.41264 18.75558,33.38532 42.07715,33.76789 z"
         style="fill:none;fill-rule:evenodd;stroke:#000000;stroke-width:10"
         inkscape:original-d="m 143.54297,419.80859 c -23.74406,1e-5 -42.99219,15.12059 -42.99219,33.77344 0,18.65285 19.24813,33.77539 42.99219,33.77539 23.74406,0 42.99218,-15.12254 42.99219,-33.77539 0,-18.65285 -19.24813,-33.77344 -42.99219,-33.77344 z" />
      <path
         id="path1334"
         d="M 142.62793,379.7207 H 97.919922 v 63.40039 h 44.708008 z"
         style="fill:none;fill-rule:evenodd;stroke:#000000;stroke-width:10"
         inkscape:original-d="m 97.919922,379.7207 v 63.40039 H 187.33594 V 379.7207 Z" />
    </g>
    <g
       transform="translate(-61.120238,11.653154)"
       id="slice-0-g1292"
       class="g1292-slice UnoptimicedTransforms"
       style="display:inline">
      <path
         transform="scale(0.26458333)"
         id="path1337"
         d=""
         style="fill:none;fill-rule:evenodd;stroke:#000000;stroke-width:37.7953;stroke-miterlimit:4;stroke-dasharray:none" />
      <path
         id="path1339"
         d=""
         style="fill:none;fill-rule:evenodd;stroke:#000000;stroke-width:10;stroke-miterlimit:4;stroke-dasharray:none" />
      <path
         id="path1341"
         d=""
         style="fill:none;fill-rule:evenodd;stroke:#000000;stroke-width:10;stroke-miterlimit:4;stroke-dasharray:none" />
      <path
         transform="matrix(0.26458333,0,0,0.26458333,20.062621,0.10915402)"
         id="path1343"
         d=""
         style="fill:none;fill-rule:evenodd;stroke:#000000;stroke-width:37.7953" />
      <path
         id="path1345"
         d=""
         style="fill:none;fill-rule:evenodd;stroke:#000000;stroke-width:10" />
      <path
         id="path1347"
         d=""
         style="fill:none;fill-rule:evenodd;stroke:#000000;stroke-width:10" />
    </g>
    <g
       id="slice-0-slice-1-0-g1292"
       class="slice-1-0-g1292-slice UnoptimicedTransforms"
       style="display:inline"
       transform="matrix(21.341837,0,0,21.341837,2751.5104,-2447.089)">
      <path
         transform="scale(0.26458333)"
         id="path1352"
         d=""
         style="fill:none;fill-rule:evenodd;stroke:#000000;stroke-width:37.7953;stroke-miterlimit:4;stroke-dasharray:none" />
      <path
         id="path1354"
         d="m 142.62793,483.72192 c 14.13453,-5.53235 23.84473,-16.99886 23.84473,-30.24731 -1e-5,-13.24845 -9.7102,-24.71594 -23.84473,-30.24887 z"
         style="fill:none;fill-rule:evenodd;stroke:#000000;stroke-width:10;stroke-miterlimit:4;stroke-dasharray:none" />
      <path
         id="path1356"
         d="m 142.62793,443.01172 h 24.64551 v -63.39844 h -24.64551 z"
         style="fill:none;fill-rule:evenodd;stroke:#000000;stroke-width:10;stroke-miterlimit:4;stroke-dasharray:none" />
      <path
         transform="matrix(0.26458333,0,0,0.26458333,20.062621,0.10915402)"
         id="path1358"
         d="m 463.23897,1526.38 20.35087,-26.9874 -20.35087,0.1581 z"
         style="fill:none;fill-rule:evenodd;stroke:#000000;stroke-width:37.7953" />
      <path
         id="path1360"
         d="m 142.62793,487.34992 c 0.30425,0.005 0.60927,0.007 0.91504,0.007 23.74406,0 42.99218,-15.12254 42.99219,-33.77539 0,-18.65285 -19.24813,-33.77344 -42.99219,-33.77344 -0.30577,0 -0.61079,0.003 -0.91504,0.007 z"
         style="fill:none;fill-rule:evenodd;stroke:#000000;stroke-width:10" />
      <path
         id="path1362"
         d="m 142.62793,443.12109 h 44.70801 V 379.7207 h -44.70801 z"
         style="fill:none;fill-rule:evenodd;stroke:#000000;stroke-width:10" />
    </g>
  </g>
</svg>
)"""";

   testDoc(svg);
}

TEST_F(LPEBooleanOperationTest, multi_MM_1_1)
{
   std::string svg = R""""(<?xml version="1.0" encoding="UTF-8" standalone="no"?>
<svg
   width="210mm"
   height="297mm"
   viewBox="0 0 210 297"
   version="1.1"
   id="svg5"
   inkscape:version="1.2-dev (99544120e7, 2021-04-09)"
   sodipodi:docname="1.svg"
   xmlns:inkscape="http://www.inkscape.org/namespaces/inkscape"
   xmlns:sodipodi="http://sodipodi.sourceforge.net/DTD/sodipodi-0.dtd"
   xmlns="http://www.w3.org/2000/svg"
   xmlns:svg="http://www.w3.org/2000/svg">
  <sodipodi:namedview
     id="namedview7"
     pagecolor="#ffffff"
     bordercolor="#666666"
     borderopacity="1.0"
     objecttolerance="10.0"
     gridtolerance="10.0"
     guidetolerance="10.0"
     inkscape:pageshadow="2"
     inkscape:pageopacity="0.0"
     inkscape:pagecheckerboard="0"
     inkscape:document-units="mm"
     showgrid="false"
     inkscape:zoom="0.74653479"
     inkscape:cx="397.16836"
     inkscape:cy="1344.2106"
     inkscape:window-width="1920"
     inkscape:window-height="1016"
     inkscape:window-x="0"
     inkscape:window-y="27"
     inkscape:window-maximized="1"
     inkscape:current-layer="layer1" />
  <defs
     id="defs2">
    <inkscape:path-effect
       effect="slice"
       start_point="142.62793,379.61328"
       end_point="142.62793,487.35742"
       center_point="142.62793,433.48535"
       id="path-effect1350"
       is_visible="true"
       lpeversion="1"
       allow_transforms="true" />
    <inkscape:path-effect
       effect="slice"
       start_point="53.212416,362.47607"
       end_point="53.212416,481.841"
       center_point="53.212416,422.15853"
       id="path-effect1309"
       is_visible="true"
       lpeversion="1"
       allow_transforms="true" />
    <inkscape:path-effect
       effect="slice"
       start_point="97.920442,362.47591"
       end_point="97.920442,487.35689"
       center_point="97.920442,424.9164"
       id="path-effect1294"
       is_visible="true"
       lpeversion="1"
       allow_transforms="true" />
    <inkscape:path-effect
       effect="slice"
       start_point="75.886874,768.34765"
       end_point="75.886874,1224.3066"
       center_point="75.886874,996.32712"
       id="path-effect1004"
       is_visible="true"
       lpeversion="1"
       allow_transforms="true" />
    <inkscape:path-effect
       effect="slice"
       start_point="193.63703,768.34699"
       end_point="193.63703,1224.3069"
       center_point="193.63703,996.32694"
       id="path-effect1001"
       is_visible="true"
       lpeversion="1"
       allow_transforms="true" />
    <inkscape:path-effect
       effect="slice"
       start_point="193.286,115.04085"
       end_point="82.288292,137.1847"
       center_point="137.65506,125.40941"
       id="path-effect856"
       is_visible="true"
       lpeversion="1"
       allow_transforms="false" />
    <inkscape:path-effect
       effect="slice"
       start_point="103.76717,94.14931"
       end_point="10.442993,156.70153"
       center_point="57.10508,125.42542"
       id="path-effect851"
       is_visible="true"
       lpeversion="1"
       allow_transforms="false" />
    <inkscape:path-effect
       effect="slice"
       start_point="96.470909,61.288662"
       end_point="96.470909,188.54016"
       center_point="96.470909,124.91441"
       id="path-effect848"
       is_visible="true"
       lpeversion="1"
       allow_transforms="true" />
  </defs>
  <g
     inkscape:label="Layer 1"
     inkscape:groupmode="layer"
     id="layer1">
    <path
       id="path976"
       style="fill:none;fill-rule:evenodd;stroke:#000000;stroke-width:37.7953"
       inkscape:transform-center-x="1.1145587"
       inkscape:transform-center-y="-5.196207"
       transform="matrix(0.26458333,0,0,0.26458333,-53.05947,19.217554)"
       d="M 75.886874,928.62985 -41.863281,952.41211 75.886874,1055.1921 Z m 0,186.84605 -12.554843,108.8307 12.554843,-7.5091 z"
       inkscape:path-effect="#path-effect1001;#path-effect1004"
       inkscape:original-d="M 354.42488,1208.2816 204.6655,1139.7757 63.331136,1224.3069 82.20589,1060.7078 -41.862757,952.41236 119.56186,919.80841 184.21759,768.347 265.10873,911.79576 429.13682,926.48286 317.70568,1047.743 Z" />
    <path
       style="fill:none;fill-rule:evenodd;stroke:#000000;stroke-width:10;stroke-miterlimit:4;stroke-dasharray:none"
       id="path846"
       inkscape:path-effect="#path-effect848;#path-effect851"
       sodipodi:type="arc"
       sodipodi:cx="96.470909"
       sodipodi:cy="124.91441"
       sodipodi:rx="80.346069"
       sodipodi:ry="63.625751"
       d="m 96.470703,99.039898 0,-37.750836 c -44.373909,0 -80.345703,28.485471 -80.345703,63.624998 0,8.49573 2.102672,16.60262 5.918392,24.01209 z"
       transform="translate(116.1038,21.381441)" />
    <path
       id="slice-1-path846"
       class="path846-slice UnoptimicedTransforms"
       d="m 22.043392,148.92615 c 11.966634,23.23714 40.781739,39.61487 74.427311,39.61487 l 0,-89.501122 z"
       style="display:inline;fill:none;fill-rule:evenodd;stroke:#000000;stroke-width:10;stroke-miterlimit:4;stroke-dasharray:none"
       transform="translate(116.1038,21.381441)" />
    <path
       id="slice-1-0-path846"
       class="path846-slice UnoptimicedTransforms"
       d="M 176.30545,117.69873 C 173.47666,97.853496 159.11937,80.773161 138.66406,70.757812 l -42.193357,28.28125 v 34.586558 z"
       style="display:inline;fill:none;fill-rule:evenodd;stroke:#000000;stroke-width:10;stroke-miterlimit:4;stroke-dasharray:none"
       inkscape:path-effect="#path-effect856"
       inkscape:original-d="m 96.470703,99.039898 v 89.501122 c 44.373807,-9e-5 80.345707,-28.48748 80.345707,-63.62696 -1e-5,-22.88349 -15.25519,-42.945104 -38.15148,-54.155607 z"
       transform="translate(116.1038,21.381441)" />
    <path
       id="slice-0-path846"
       class="path846-slice UnoptimicedTransforms"
       d="M 138.66493,70.758453 C 126.4021,64.754314 111.94744,61.289093 96.470703,61.289062 v 37.750836 z"
       style="display:inline;fill:none;fill-rule:evenodd;stroke:#000000;stroke-width:10;stroke-miterlimit:4;stroke-dasharray:none"
       transform="translate(116.1038,21.381441)" />
    <path
       id="slice-0-slice-1-0-path846"
       class="slice-1-0-path846-slice UnoptimicedTransforms"
       d="m 96.470703,133.62562 v 54.9154 c 44.373807,-9e-5 80.345707,-28.48748 80.345707,-63.62696 0,-2.43961 -0.1734,-4.84716 -0.51096,-7.21533 z"
       style="display:inline;fill:none;fill-rule:evenodd;stroke:#000000;stroke-width:10;stroke-miterlimit:4;stroke-dasharray:none"
       transform="translate(116.1038,21.381441)" />
    <path
       transform="scale(0.26458333)"
       id="slice-1-path976"
       class="path976-slice UnoptimicedTransforms"
       d="M 75.886874,1216.7975 193.63672,1146.3711 V 785.05273 l -9.41992,-16.70507 -64.6543,151.46093 -43.675626,8.82126 z m 0,-161.6054 6.318204,5.5149 -6.318204,54.7689 z"
       style="display:inline;fill:none;fill-rule:evenodd;stroke:#000000;stroke-width:37.7953" />
    <path
       transform="matrix(0.26458333,0,0,0.26458333,63.855427,29.707718)"
       id="slice-1-0-path976"
       class="path976-slice UnoptimicedTransforms"
       d="m 193.63672,785.05273 v 361.31837 l 11.0293,-6.5957 149.75976,68.5058 -36.7207,-160.539 111.43164,-121.25978 -164.02734,-14.6875 z"
       style="display:inline;fill:none;fill-rule:evenodd;stroke:#000000;stroke-width:37.7953" />
    <path
       transform="scale(0.26458333)"
       id="slice-0-path976"
       class="path976-slice UnoptimicedTransforms"
       d=""
       style="display:inline;fill:none;fill-rule:evenodd;stroke:#000000;stroke-width:37.7953" />
    <g
       id="g1292"
       inkscape:path-effect="#path-effect1294;#path-effect1309"
       transform="translate(-61.120238,11.653154)">
      <path
         sodipodi:type="star"
         style="fill:none;fill-rule:evenodd;stroke:#000000;stroke-width:37.7953;stroke-miterlimit:4;stroke-dasharray:none"
         id="path1111"
         inkscape:flatsided="false"
         sodipodi:sides="5"
         sodipodi:cx="269.10574"
         sodipodi:cy="1607.3273"
         sodipodi:r1="240.11061"
         sodipodi:r2="120.05531"
         sodipodi:arg1="0.79040597"
         sodipodi:arg2="1.4187245"
         inkscape:rounded="0"
         inkscape:randomized="0"
         d="m 201.1178,1469.9256 -16.47718,52.0861 -152.498042,46.5664 129.720702,92.7168 -2.83594,159.4239 42.09046,-31.0833 z"
         transform="scale(0.26458333)"
         inkscape:transform-center-x="2.9739241"
         inkscape:transform-center-y="-3.1681299" />
      <ellipse
         style="fill:none;fill-rule:evenodd;stroke:#000000;stroke-width:10;stroke-miterlimit:4;stroke-dasharray:none"
         id="path1135"
         cx="123.481"
         cy="453.47375"
         rx="42.992428"
         ry="33.773975"
         d="" />
      <path
         style="fill:none;fill-rule:evenodd;stroke:#000000;stroke-width:10;stroke-miterlimit:4;stroke-dasharray:none"
         id="rect1159"
         width="90.091156"
         height="63.399643"
         x="77.18277"
         y="379.61249"
         d=""
         sodipodi:type="rect" />
      <path
         id="path1259"
         style="fill:none;fill-rule:evenodd;stroke:#000000;stroke-width:37.7953"
         inkscape:transform-center-x="2.9739241"
         inkscape:transform-center-y="-3.1681299"
         transform="matrix(0.26458333,0,0,0.26458333,20.062621,0.10915402)"
         d="m 125.29057,1540.1347 -93.147992,28.4434 93.147992,66.5768 z"
         inkscape:original-d="m 438.03721,1777.9592 -150.74472,-51.9621 -128.26468,94.7217 2.83626,-159.4239 -129.721703,-92.7164 152.497633,-46.5672 48.09226,-152.0236 91.41245,130.6438 159.44436,-1.2394 -96.00163,127.3095 z" />
      <path
         id="ellipse1261"
         style="fill:none;fill-rule:evenodd;stroke:#000000;stroke-width:10"
         d=""
         inkscape:original-d="m 186.53605,453.58292 a 42.992428,33.773975 0 0 1 -42.99243,33.77397 42.992428,33.773975 0 0 1 -42.99242,-33.77397 42.992428,33.773975 0 0 1 42.99242,-33.77398 42.992428,33.773975 0 0 1 42.99243,33.77398 z" />
      <path
         id="rect1263"
         style="fill:none;fill-rule:evenodd;stroke:#000000;stroke-width:10"
         d=""
         inkscape:original-d="m 97.245392,379.72165 h 90.091158 v 63.39964 H 97.245392 Z" />
    </g>
    <g
       transform="translate(-61.120238,11.653154)"
       id="slice-1-g1292"
       class="g1292-slice UnoptimicedTransforms"
       style="display:inline">
      <path
         transform="scale(0.26458333)"
         id="path1311"
         d="m 201.1178,1789.6355 86.17517,-63.6394 82.80078,28.541 v -254.2637 l -45.94922,0.3575 -91.41211,-130.6426 -31.61462,99.9373 z"
         style="fill:none;fill-rule:evenodd;stroke:#000000;stroke-width:37.7953;stroke-miterlimit:4;stroke-dasharray:none" />
      <path
         id="path1313"
         d="m 97.919922,426.31445 c -10.576079,6.15312 -17.431641,16.02776 -17.431641,27.16016 0,11.1324 6.855562,21.0076 17.431641,27.16016 z"
         style="fill:none;fill-rule:evenodd;stroke:#000000;stroke-width:10;stroke-miterlimit:4;stroke-dasharray:none" />
      <path
         id="path1315"
         d="m 77.183594,379.61328 v 63.39844 h 20.736328 v -63.39844 z"
         style="fill:none;fill-rule:evenodd;stroke:#000000;stroke-width:10;stroke-miterlimit:4;stroke-dasharray:none" />
      <path
         transform="matrix(0.26458333,0,0,0.26458333,20.062621,0.10915402)"
         id="path1317"
         d="m 125.29057,1635.1549 36.57271,26.14 -2.83594,159.4239 128.26563,-94.7208 6.97265,2.4043 v -270.4726 l -61.5332,-87.9414 -48.0918,152.0234 -59.35005,18.123 z"
         style="fill:none;fill-rule:evenodd;stroke:#000000;stroke-width:37.7953" />
      <path
         id="path1319"
         d="" />
      <path
         id="path1321"
         d="m 97.246094,379.7207 v 63.40039 h 0.673828 V 379.7207 Z"
         style="fill:none;fill-rule:evenodd;stroke:#000000;stroke-width:10" />
    </g>
    <g
       id="slice-1-0-g1292"
       class="g1292-slice UnoptimicedTransforms"
       style="display:inline"
       inkscape:path-effect="#path-effect1350"
       transform="translate(-45.150871,-25.447467)">
      <path
         transform="scale(0.26458333)"
         id="path1324"
         d="m 483.58984,1499.3926 -113.49609,0.8808 v 254.2657 l 67.94336,23.4199 -50.44922,-151.2578 z"
         style="fill:none;fill-rule:evenodd;stroke:#000000;stroke-width:37.7953;stroke-miterlimit:4;stroke-dasharray:none"
         inkscape:original-d="m 483.58984,1499.3926 -113.49609,0.8808 v 254.2657 l 67.94336,23.4199 -50.44922,-151.2578 z" />
      <path
         id="path1326"
         d="m 142.62793,423.22574 c -5.76586,-2.25703 -12.26795,-3.52652 -19.14746,-3.52652 -9.57312,0 -18.41593,2.45851 -25.560548,6.61523 v 54.32032 c 7.144618,4.15633 15.987428,6.61328 25.560548,6.61328 6.87951,0 13.3816,-1.26933 19.14746,-3.52613 z"
         style="fill:none;fill-rule:evenodd;stroke:#000000;stroke-width:10;stroke-miterlimit:4;stroke-dasharray:none"
         inkscape:original-d="m 123.48047,419.69922 c -9.57312,0 -18.41593,2.45851 -25.560548,6.61523 v 54.32032 c 7.144618,4.15633 15.987428,6.61328 25.560548,6.61328 23.74406,0 42.99219,-15.12059 42.99219,-33.77344 -1e-5,-18.65285 -19.24813,-33.77539 -42.99219,-33.77539 z" />
      <path
         id="path1328"
         d="M 142.62793,379.61328 H 97.919922 v 63.39844 h 44.708008 z"
         style="fill:none;fill-rule:evenodd;stroke:#000000;stroke-width:10;stroke-miterlimit:4;stroke-dasharray:none"
         inkscape:original-d="m 97.919922,379.61328 v 63.39844 h 69.353518 v -63.39844 z" />
      <path
         transform="matrix(0.26458333,0,0,0.26458333,20.062621,0.10915402)"
         id="path1330"
         d="m 463.23897,1499.5507 -139.09444,1.0802 -29.87891,-42.7012 v 270.4726 l 143.77149,49.5567 -50.44922,-151.2578 75.65108,-100.3212 z"
         style="fill:none;fill-rule:evenodd;stroke:#000000;stroke-width:37.7953"
         inkscape:original-d="m 294.26562,1457.9297 v 270.4726 l 143.77149,49.5567 -50.44922,-151.2578 96.00195,-127.3086 -159.44531,1.2383 z" />
      <path
         id="path1332"
         d="m 142.62793,419.81609 c -23.32157,0.38253 -42.07715,15.3533 -42.07715,33.76594 0,18.41264 18.75558,33.38532 42.07715,33.76789 z"
         style="fill:none;fill-rule:evenodd;stroke:#000000;stroke-width:10"
         inkscape:original-d="m 143.54297,419.80859 c -23.74406,1e-5 -42.99219,15.12059 -42.99219,33.77344 0,18.65285 19.24813,33.77539 42.99219,33.77539 23.74406,0 42.99218,-15.12254 42.99219,-33.77539 0,-18.65285 -19.24813,-33.77344 -42.99219,-33.77344 z" />
      <path
         id="path1334"
         d="M 142.62793,379.7207 H 97.919922 v 63.40039 h 44.708008 z"
         style="fill:none;fill-rule:evenodd;stroke:#000000;stroke-width:10"
         inkscape:original-d="m 97.919922,379.7207 v 63.40039 H 187.33594 V 379.7207 Z" />
    </g>
    <g
       transform="translate(-61.120238,11.653154)"
       id="slice-0-g1292"
       class="g1292-slice UnoptimicedTransforms"
       style="display:inline">
      <path
         transform="scale(0.26458333)"
         id="path1337"
         d=""
         style="fill:none;fill-rule:evenodd;stroke:#000000;stroke-width:37.7953;stroke-miterlimit:4;stroke-dasharray:none" />
      <path
         id="path1339"
         d=""
         style="fill:none;fill-rule:evenodd;stroke:#000000;stroke-width:10;stroke-miterlimit:4;stroke-dasharray:none" />
      <path
         id="path1341"
         d=""
         style="fill:none;fill-rule:evenodd;stroke:#000000;stroke-width:10;stroke-miterlimit:4;stroke-dasharray:none" />
      <path
         transform="matrix(0.26458333,0,0,0.26458333,20.062621,0.10915402)"
         id="path1343"
         d=""
         style="fill:none;fill-rule:evenodd;stroke:#000000;stroke-width:37.7953" />
      <path
         id="path1345"
         d=""
         style="fill:none;fill-rule:evenodd;stroke:#000000;stroke-width:10" />
      <path
         id="path1347"
         d=""
         style="fill:none;fill-rule:evenodd;stroke:#000000;stroke-width:10" />
    </g>
    <g
       id="slice-0-slice-1-0-g1292"
       class="slice-1-0-g1292-slice UnoptimicedTransforms"
       style="display:inline"
       transform="translate(57.386201,-40.630453)">
      <path
         transform="scale(0.26458333)"
         id="path1352"
         d=""
         style="fill:none;fill-rule:evenodd;stroke:#000000;stroke-width:37.7953;stroke-miterlimit:4;stroke-dasharray:none" />
      <path
         id="path1354"
         d="m 142.62793,483.72192 c 14.13453,-5.53235 23.84473,-16.99886 23.84473,-30.24731 -1e-5,-13.24845 -9.7102,-24.71594 -23.84473,-30.24887 z"
         style="fill:none;fill-rule:evenodd;stroke:#000000;stroke-width:10;stroke-miterlimit:4;stroke-dasharray:none" />
      <path
         id="path1356"
         d="m 142.62793,443.01172 h 24.64551 v -63.39844 h -24.64551 z"
         style="fill:none;fill-rule:evenodd;stroke:#000000;stroke-width:10;stroke-miterlimit:4;stroke-dasharray:none" />
      <path
         transform="matrix(0.26458333,0,0,0.26458333,20.062621,0.10915402)"
         id="path1358"
         d="m 463.23897,1526.38 20.35087,-26.9874 -20.35087,0.1581 z"
         style="fill:none;fill-rule:evenodd;stroke:#000000;stroke-width:37.7953" />
      <path
         id="path1360"
         d="m 142.62793,487.34992 c 0.30425,0.005 0.60927,0.007 0.91504,0.007 23.74406,0 42.99218,-15.12254 42.99219,-33.77539 0,-18.65285 -19.24813,-33.77344 -42.99219,-33.77344 -0.30577,0 -0.61079,0.003 -0.91504,0.007 z"
         style="fill:none;fill-rule:evenodd;stroke:#000000;stroke-width:10" />
      <path
         id="path1362"
         d="m 142.62793,443.12109 h 44.70801 V 379.7207 h -44.70801 z"
         style="fill:none;fill-rule:evenodd;stroke:#000000;stroke-width:10" />
    </g>
  </g>
</svg>
)"""";

   testDoc(svg);
}
