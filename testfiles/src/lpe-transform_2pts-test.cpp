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

class LPETransform2PointsTest : public LPESPathsTest {};

// TEST fails if multiple simplify on stack

// INKSCAPE 0.92.5

TEST_F(LPETransform2PointsTest, path_0_92_5)
{
   // path10 not pass skiped
   std::string svg = R""""(<?xml version="1.0" encoding="UTF-8" standalone="no"?>
<!-- Created with Inkscape (http://www.inkscape.org/) -->

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
   inkscape:version="0.92.3 (unknown)"
   sodipodi:docname="1.svg">
  <defs
     id="defs2">
    <inkscape:path-effect
       lock_angle="false"
       lock_lenght="false"
       flip_horizontal="false"
       flip_vertical="false"
       from_original_width="false"
       elastic="true"
       offset="0"
       stretch="1"
       helper_size="3"
       last_knot="8"
       first_knot="1"
       is_visible="true"
       id="path-effect826"
       end="-324.28743,-27.076008"
       start="-172.35714,146.56548"
       effect="transform_2pts" />
    <inkscape:path-effect
       effect="transform_2pts"
       start="476.67598,494.03273"
       end="216.17116,-27.489144"
       id="path-effect822"
       is_visible="true"
       first_knot="1"
       last_knot="7"
       helper_size="3"
       stretch="1"
       offset="0"
       elastic="false"
       from_original_width="false"
       flip_vertical="false"
       flip_horizontal="false"
       lock_lenght="false"
       lock_angle="false" />
    <inkscape:path-effect
       effect="transform_2pts"
       start="-83.631556,-14.62824"
       end="130.55267,-29.747288"
       id="path-effect817"
       is_visible="true"
       first_knot="1"
       last_knot="2"
       helper_size="3"
       stretch="1"
       offset="0"
       elastic="false"
       from_original_width="true"
       flip_vertical="false"
       flip_horizontal="false"
       lock_lenght="false"
       lock_angle="false" />
  </defs>
  <sodipodi:namedview
     id="base"
     pagecolor="#ffffff"
     bordercolor="#666666"
     borderopacity="1.0"
     inkscape:pageopacity="0.0"
     inkscape:pageshadow="2"
     inkscape:zoom="0.175"
     inkscape:cx="-1045.2892"
     inkscape:cy="71.886311"
     inkscape:document-units="mm"
     inkscape:current-layer="layer1"
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
        <dc:title />
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
       id="path815"
       sodipodi:sides="5"
       sodipodi:cx="-11.339286"
       sodipodi:cy="-16.720238"
       sodipodi:r1="84.710526"
       sodipodi:r2="42.355263"
       sodipodi:arg1="1.3366398"
       sodipodi:arg2="1.9649583"
       inkscape:flatsided="false"
       inkscape:rounded="0"
       inkscape:randomized="0"
       d="M 49.766767,86.246605 -3.497199,30.551163 -79.573932,42.854149 -43.06389,-25.013731 -78.273729,-93.565172 -2.4453155,-79.814386 51.870541,-134.48449 62.225035,-58.11816 131.00392,-23.354703 61.574935,10.0915 Z"
       inkscape:transform-center-x="-6.0736555"
       inkscape:transform-center-y="2.0919967"
       inkscape:path-effect="#path-effect817"
       transform="translate(1.5119048,-95.25)" />
    <path
       inkscape:connector-curvature="0"
       inkscape:original-d="m -172.35714,146.56548 -63.64938,-21.20931 -53.55552,40.40896 0.50251,-67.08819 -54.98077,-38.44727 63.95994,-20.25348 19.57554,-64.170676 39.02691,54.570856 67.07912,-1.21239 -39.83999,53.98011 z"
       inkscape:path-effect="#path-effect826"
       id="path824"
       d="m -172.35714,146.56548 -91.28473,-27.84133 -29.25591,65.3705 -54.17894,-102.29662 -95.28131,-54.727991 57.80028,-35.3814693 -29.63117,-99.1942597 89.90149,80.429682 76.96824,-6.577433 -2.23812,85.089724 z"
       inkscape:transform-center-y="-4.7206335"
       inkscape:transform-center-x="5.0469249"
       style="fill:none;fill-rule:evenodd;stroke:#000000;stroke-width:10;stroke-miterlimit:4;stroke-dasharray:none" />
    <path
       style="fill:none;fill-rule:evenodd;stroke:#000000;stroke-width:10;stroke-miterlimit:4;stroke-dasharray:none"
       inkscape:transform-center-x="5.0469249"
       inkscape:transform-center-y="-4.7206335"
       d="M 476.67598,494.03273 284.46022,427.1346 120.38112,547.55179 124.60708,344.0711 -40.619703,225.23357 154.20781,166.37368 216.17116,-27.489144 332.35524,139.61417 535.87748,138.63789 412.85565,300.77325 Z"
       id="path819"
       inkscape:path-effect="#path-effect822"
       inkscape:original-d="m 476.67598,494.03273 -63.64938,-21.20931 -53.55552,40.40896 0.50251,-67.08819 -54.98077,-38.44727 63.95994,-20.25348 19.57554,-64.17068 39.02691,54.57086 67.07912,-1.21239 -39.83999,53.98011 z"
       inkscape:connector-curvature="0" />
  </g>
</svg>
)"""";

   testDoc(svg);
}


// INKSCAPE 1.0.2

TEST_F(LPETransform2PointsTest, multi_PX_1_0_2)
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
       lock_angle="false"
       lock_lenght="false"
       flip_horizontal="false"
       flip_vertical="false"
       from_original_width="false"
       elastic="true"
       offset="0"
       stretch="1"
       helper_size="3"
       last_knot="8"
       first_knot="1"
       is_visible="true"
       id="path-effect826"
       end="-200.81584,243.9868"
       start="66.930747,552.16325"
       effect="transform_2pts"
       lpeversion="1"
       lock_length="false" />
    <inkscape:path-effect
       effect="transform_2pts"
       start="1210.7212,1168.8432"
       end="751.63379,243.25357"
       id="path-effect822"
       is_visible="true"
       first_knot="1"
       last_knot="7"
       helper_size="3"
       stretch="1"
       offset="0"
       elastic="false"
       from_original_width="false"
       flip_vertical="false"
       flip_horizontal="false"
       lock_lenght="false"
       lock_angle="false"
       lpeversion="1"
       lock_length="false" />
    <inkscape:path-effect
       effect="transform_2pts"
       start="-265.8744,-128.70673"
       end="-109.14255,-128.70673"
       id="path-effect817"
       is_visible="true"
       first_knot="1"
       last_knot="2"
       helper_size="3"
       stretch="1"
       offset="0"
       elastic="false"
       from_original_width="true"
       flip_vertical="false"
       flip_horizontal="false"
       lock_lenght="false"
       lock_angle="false"
       lpeversion="1"
       lock_length="false" />
  </defs>
  <sodipodi:namedview
     id="base"
     pagecolor="#ffffff"
     bordercolor="#666666"
     borderopacity="1.0"
     inkscape:pageopacity="0.0"
     inkscape:pageshadow="2"
     inkscape:zoom="0.175"
     inkscape:cx="-1031.0035"
     inkscape:cy="780.58029"
     inkscape:document-units="px"
     inkscape:current-layer="layer1"
     showgrid="false"
     inkscape:window-width="1920"
     inkscape:window-height="1016"
     inkscape:window-x="0"
     inkscape:window-y="27"
     inkscape:window-maximized="1"
     inkscape:document-rotation="0"
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
       sodipodi:type="star"
       style="fill:none;fill-rule:evenodd;stroke:#000000;stroke-width:10;stroke-miterlimit:4;stroke-dasharray:none"
       id="path815"
       sodipodi:sides="5"
       sodipodi:cx="-193.58212"
       sodipodi:cy="-130.79874"
       sodipodi:r1="84.710526"
       sodipodi:r2="42.355263"
       sodipodi:arg1="1.3366398"
       sodipodi:arg2="1.9649583"
       inkscape:flatsided="false"
       inkscape:rounded="0"
       inkscape:randomized="0"
       d="m -173.92737,-48.399918 -35.92064,-43.291402 -56.02639,5.04801 30.07248,-47.54034 -22.11405,-51.72434 54.50646,13.90986 42.35915,-37.01541 3.61437,56.1371 48.29344,28.84756 -52.27266,20.78478 z"
       inkscape:transform-center-x="-10.703601"
       inkscape:transform-center-y="3.712852"
       inkscape:path-effect="#path-effect817"
       transform="matrix(1.762299,0,0,1.7747858,694.50637,325.45746)" />
    <path
       inkscape:connector-curvature="0"
       inkscape:original-d="m 66.930747,552.16325 -112.169226,-37.64197 -94.380841,71.71726 0.88557,-119.06719 -96.89255,-68.23566 112.71653,-35.94559 34.497958,-113.88922 68.777078,96.8516 118.213466,-2.15174 -70.209978,95.80314 z"
       inkscape:path-effect="#path-effect826"
       id="path824"
       d="m 66.930747,552.16325 -160.607532,-49.37839 -51.217365,116.06257 -95.61858,-181.57258 -167.72758,-97.10638 101.512,-62.83962 -52.44398,-176.07762 158.35645,142.73557 135.315439,-11.71556 -3.642381,151.055 z"
       inkscape:transform-center-y="-8.33421"
       inkscape:transform-center-x="8.5685778"
       style="fill:none;fill-rule:evenodd;stroke:#000000;stroke-width:17.6853;stroke-miterlimit:4;stroke-dasharray:none" />
    <path
       style="fill:none;fill-rule:evenodd;stroke:#000000;stroke-width:17.6853;stroke-miterlimit:4;stroke-dasharray:none"
       inkscape:transform-center-x="8.9041596"
       inkscape:transform-center-y="-8.327653"
       d="M 1210.7212,1168.8432 871.95795,1050.1551 582.77136,1263.929 590.23315,902.7664 299.03979,691.883 642.41425,587.36043 751.63379,243.25357 956.38932,539.81761 1315.0837,538.0312 1098.2551,825.84035 Z"
       id="path819"
       inkscape:path-effect="#path-effect822"
       inkscape:original-d="m 1210.7212,1168.8432 -112.1693,-37.642 -94.3808,71.7173 0.8856,-119.0672 -96.89259,-68.2357 112.71649,-35.94554 34.498,-113.88923 68.7771,96.85161 118.2134,-2.15174 -70.2099,95.8031 z"
       inkscape:connector-curvature="0" />
  </g>
</svg>
)"""";

   testDoc(svg);
}

TEST_F(LPETransform2PointsTest, multi_MM_1_0_2)
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
       lock_angle="false"
       lock_lenght="false"
       flip_horizontal="false"
       flip_vertical="false"
       from_original_width="false"
       elastic="true"
       offset="0"
       stretch="1"
       helper_size="3"
       last_knot="8"
       first_knot="1"
       is_visible="true"
       id="path-effect826"
       end="-324.82341,73.95964"
       start="-204.884,212.00996"
       lpeversion="1"
       effect="transform_2pts" />
    <inkscape:path-effect
       effect="transform_2pts"
       start="307.48687,488.25711"
       end="101.83468,73.631184"
       id="path-effect822"
       is_visible="true"
       first_knot="1"
       last_knot="7"
       helper_size="3"
       stretch="1"
       offset="0"
       elastic="false"
       from_original_width="false"
       flip_vertical="false"
       flip_horizontal="false"
       lock_lenght="false"
       lpeversion="1"
       lock_angle="false" />
    <inkscape:path-effect
       effect="transform_2pts"
       start="-265.8744,-128.70673"
       end="-109.14255,-128.70673"
       id="path-effect817"
       is_visible="true"
       first_knot="1"
       last_knot="2"
       helper_size="3"
       stretch="1"
       offset="0"
       elastic="false"
       from_original_width="true"
       flip_vertical="false"
       flip_horizontal="false"
       lock_lenght="false"
       lpeversion="1"
       lock_angle="false" />
  </defs>
  <sodipodi:namedview
     id="base"
     pagecolor="#ffffff"
     bordercolor="#666666"
     borderopacity="1.0"
     inkscape:pageopacity="0.0"
     inkscape:pageshadow="2"
     inkscape:zoom="0.175"
     inkscape:cx="-1031.0035"
     inkscape:cy="780.58029"
     inkscape:document-units="mm"
     inkscape:current-layer="layer1"
     showgrid="false"
     inkscape:window-width="1920"
     inkscape:window-height="1016"
     inkscape:window-x="0"
     inkscape:window-y="27"
     inkscape:window-maximized="1"
     inkscape:document-rotation="0" />
  <metadata
     id="metadata5">
    <rdf:RDF>
      <cc:Work
         rdf:about="">
        <dc:format>image/svg+xml</dc:format>
        <dc:type
           rdf:resource="http://purl.org/dc/dcmitype/StillImage" />
        <dc:title />
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
       id="path815"
       sodipodi:sides="5"
       sodipodi:cx="-193.58212"
       sodipodi:cy="-130.79874"
       sodipodi:r1="84.710526"
       sodipodi:r2="42.355263"
       sodipodi:arg1="1.3366398"
       sodipodi:arg2="1.9649583"
       inkscape:flatsided="false"
       inkscape:rounded="0"
       inkscape:randomized="0"
       d="m -173.92737,-48.399918 -35.92064,-43.291402 -56.02639,5.04801 30.07248,-47.54034 -22.11405,-51.72434 54.50646,13.90986 42.35915,-37.01541 3.61437,56.1371 48.29344,28.84756 -52.27266,20.78478 z"
       inkscape:transform-center-x="-4.7947696"
       inkscape:transform-center-y="1.6632018"
       inkscape:path-effect="#path-effect817"
       transform="matrix(0.78943716,0,0,0.79503075,76.243955,110.45513)" />
    <path
       inkscape:connector-curvature="0"
       inkscape:original-d="m -204.884,212.00996 -50.24718,-16.86205 -42.27872,32.12637 0.3967,-53.33718 -43.40386,-30.56676 50.49235,-16.10214 15.45366,-51.017662 30.80929,43.385512 52.95475,-0.96389 -31.45117,42.91585 z"
       inkscape:path-effect="#path-effect826"
       id="path824"
       d="m -204.884,212.00996 -71.94554,-22.11948 -22.94327,51.99124 -42.83316,-81.33701 -75.13503,-43.49965 45.47318,-28.149557 -23.49273,-78.875495 70.93714,63.939632 60.61573,-5.248088 -1.63164,67.666398 z"
       inkscape:transform-center-y="-3.7333778"
       inkscape:transform-center-x="3.8383673"
       style="fill:none;fill-rule:evenodd;stroke:#000000;stroke-width:7.92229;stroke-miterlimit:4;stroke-dasharray:none" />
    <path
       style="fill:none;fill-rule:evenodd;stroke:#000000;stroke-width:7.92229;stroke-miterlimit:4;stroke-dasharray:none"
       inkscape:transform-center-x="3.9886577"
       inkscape:transform-center-y="-3.7304598"
       d="M 307.48687,488.25711 155.735,435.08979 26.191294,530.85159 29.533824,369.06571 -100.90872,274.59871 52.908902,227.77684 101.83468,73.631184 l 91.722,132.848436 160.68035,-0.80025 -97.13031,128.92667 z"
       id="path819"
       inkscape:path-effect="#path-effect822"
       inkscape:original-d="m 307.48687,488.25711 -50.24719,-16.86205 -42.27872,32.12636 0.3967,-53.33717 -43.40386,-30.56676 50.49235,-16.10214 15.45366,-51.01767 30.80929,43.38552 52.95475,-0.96389 -31.45116,42.91585 z"
       inkscape:connector-curvature="0" />
  </g>
</svg>
)"""";

   testDoc(svg);
}