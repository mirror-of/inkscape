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

class LPEPowerMaskTest : public LPESPathsTest {};
// INKSCAPE 1.0.2

TEST_F(LPEPowerMaskTest, multi_PX_1_0_2)
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
   id="svg953"
   inkscape:version="1.0.2 (e86c870879, 2021-01-15)"
   sodipodi:docname="1.svg">
  <defs
     id="defs947">
    <inkscape:path-effect
       effect="powermask"
       id="path-effect1714"
       is_visible="true"
       lpeversion="1"
       uri="#mask-powermask-path-effect1714"
       invert="false"
       hide_mask="false"
       background="true"
       background_color="#05ff00ff"
       background_color_opacity_LPE="1" />
    <inkscape:path-effect
       effect="powermask"
       id="path-effect1574"
       is_visible="true"
       lpeversion="1"
       uri="#mask-powermask-path-effect1574"
       invert="false"
       hide_mask="false"
       background="true"
       background_color="#ffffffff" />
    <inkscape:path-effect
       effect="powermask"
       id="path-effect1556"
       is_visible="true"
       lpeversion="1"
       uri="#mask-powermask-path-effect1556"
       invert="false"
       hide_mask="false"
       background="true"
       background_color="#ffffffff" />
    <inkscape:path-effect
       effect="powermask"
       id="path-effect1542"
       is_visible="true"
       lpeversion="1"
       uri="#mask-powermask-path-effect1542"
       invert="false"
       hide_mask="false"
       background="true"
       background_color="#ffffffff" />
    <inkscape:path-effect
       effect="powermask"
       id="path-effect1524"
       is_visible="true"
       lpeversion="1"
       uri="#mask-powermask-path-effect1524"
       invert="false"
       hide_mask="false"
       background="true"
       background_color="#ff0084ff"
       background_color_opacity_LPE="1" />
    <mask
       maskUnits="userSpaceOnUse"
       id="mask-powermask-path-effect1524">
      <path
         id="mask-powermask-path-effect1524_box"
         style="fill:#ff0084;fill-opacity:1"
         d="M -224.56375,-2.5846939 H 139.06147 V 233.58665 h -363.62522 z" />
      <ellipse
         style="fill:#008080;fill-rule:evenodd;stroke:#000000;stroke-width:10"
         id="ellipse1522"
         cx="-54.03064"
         cy="107.55451"
         rx="66.384262"
         ry="49.355526"
         d="M 12.353622,107.55451 A 66.384262,49.355526 0 0 1 -54.03064,156.91004 66.384262,49.355526 0 0 1 -120.4149,107.55451 66.384262,49.355526 0 0 1 -54.03064,58.198986 66.384262,49.355526 0 0 1 12.353622,107.55451 Z" />
    </mask>
    <filter
       id="mask-powermask-path-effect1524_inverse"
       inkscape:label="filtermask-powermask-path-effect1524"
       style="color-interpolation-filters:sRGB"
       height="100"
       width="100"
       x="-50"
       y="-50">
      <feColorMatrix
         id="mask-powermask-path-effect1524_primitive1"
         values="1"
         type="saturate"
         result="fbSourceGraphic" />
      <feColorMatrix
         id="mask-powermask-path-effect1524_primitive2"
         values="-1 0 0 0 1 0 -1 0 0 1 0 0 -1 0 1 0 0 0 1 0 "
         in="fbSourceGraphic" />
    </filter>
    <mask
       maskUnits="userSpaceOnUse"
       id="mask-powermask-path-effect1542">
      <path
         id="mask-powermask-path-effect1542_box"
         style="fill:#ffffff;fill-opacity:1"
         d="M -250.92118,170.85218 H 13.671501 V 370.34521 H -250.92118 Z" />
      <path
         id="path1540"
         style="fill:#ffff00;fill-rule:evenodd;stroke:#000000;stroke-width:10"
         inkscape:transform-center-x="-0.39075553"
         inkscape:transform-center-y="-5.2654283"
         d="m -97.509163,317.8194 -34.307387,-18.98512 -35.08889,17.49862 7.45438,-38.495 -27.48525,-27.96414 38.91445,-4.80607 18.10207,-34.78142 16.59607,35.52468 38.672943,6.46804 -28.657513,26.76152 z" />
    </mask>
    <filter
       id="mask-powermask-path-effect1542_inverse"
       inkscape:label="filtermask-powermask-path-effect1542"
       style="color-interpolation-filters:sRGB"
       height="100"
       width="100"
       x="-50"
       y="-50">
      <feColorMatrix
         id="mask-powermask-path-effect1542_primitive1"
         values="1"
         type="saturate"
         result="fbSourceGraphic" />
      <feColorMatrix
         id="mask-powermask-path-effect1542_primitive2"
         values="-1 0 0 0 1 0 -1 0 0 1 0 0 -1 0 1 0 0 0 1 0 "
         in="fbSourceGraphic" />
    </filter>
    <mask
       maskUnits="userSpaceOnUse"
       id="mask-powermask-path-effect1556">
      <path
         id="mask-powermask-path-effect1556_box"
         style="fill:#ffffff;fill-opacity:1"
         d="M -250.92118,170.85218 H 13.671501 V 370.34521 H -250.92118 Z" />
      <path
         id="path1558"
         style="fill:#ffffff;fill-opacity:1"
         d="M -250.92118,170.85218 H 13.671501 V 370.34521 H -250.92118 Z" />
      <path
         id="path1560"
         style="fill:#ffff00;fill-rule:evenodd;stroke:#000000;stroke-width:10"
         inkscape:transform-center-x="-0.39075553"
         inkscape:transform-center-y="-5.2654283"
         d="m -97.509163,317.8194 -34.307387,-18.98512 -35.08889,17.49862 7.45438,-38.495 -27.48525,-27.96414 38.91445,-4.80607 18.10207,-34.78142 16.59607,35.52468 38.672943,6.46804 -28.657513,26.76152 z" />
    </mask>
    <filter
       id="mask-powermask-path-effect1556_inverse"
       inkscape:label="filtermask-powermask-path-effect1556"
       style="color-interpolation-filters:sRGB"
       height="100"
       width="100"
       x="-50"
       y="-50">
      <feColorMatrix
         id="mask-powermask-path-effect1556_primitive1"
         values="1"
         type="saturate"
         result="fbSourceGraphic" />
      <feColorMatrix
         id="mask-powermask-path-effect1556_primitive2"
         values="-1 0 0 0 1 0 -1 0 0 1 0 0 -1 0 1 0 0 0 1 0 "
         in="fbSourceGraphic" />
    </filter>
    <mask
       maskUnits="userSpaceOnUse"
       id="mask-powermask-path-effect1574">
      <path
         id="mask-powermask-path-effect1574_box"
         style="fill:#ffffff;fill-opacity:1"
         d="M -224.56375,-2.5846939 H 139.06147 V 233.58665 h -363.62522 z" />
      <path
         id="path1576"
         style="fill:#ffffff;fill-opacity:1"
         d="M -224.56375,-2.5846939 H 139.06147 V 233.58665 h -363.62522 z" />
      <ellipse
         style="fill:#008080;fill-rule:evenodd;stroke:#000000;stroke-width:10"
         id="ellipse1578"
         cx="-54.03064"
         cy="107.55451"
         rx="66.384262"
         ry="49.355526"
         d="M 12.353622,107.55451 A 66.384262,49.355526 0 0 1 -54.03064,156.91004 66.384262,49.355526 0 0 1 -120.4149,107.55451 66.384262,49.355526 0 0 1 -54.03064,58.198986 66.384262,49.355526 0 0 1 12.353622,107.55451 Z" />
    </mask>
    <filter
       id="mask-powermask-path-effect1574_inverse"
       inkscape:label="filtermask-powermask-path-effect1574"
       style="color-interpolation-filters:sRGB"
       height="100"
       width="100"
       x="-50"
       y="-50">
      <feColorMatrix
         id="mask-powermask-path-effect1574_primitive1"
         values="1"
         type="saturate"
         result="fbSourceGraphic" />
      <feColorMatrix
         id="mask-powermask-path-effect1574_primitive2"
         values="-1 0 0 0 1 0 -1 0 0 1 0 0 -1 0 1 0 0 0 1 0 "
         in="fbSourceGraphic" />
    </filter>
    <mask
       maskUnits="userSpaceOnUse"
       id="mask-powermask-path-effect1714">
      <path
         id="mask-powermask-path-effect1714_box"
         style="fill:#05ff00;fill-opacity:1"
         d="m -180.95891,308.69259 h 390.06043 v 271.0059 h -390.06043 z" />
      <ellipse
         style="fill:#ffff00;fill-rule:evenodd;stroke:#000000;stroke-width:10"
         id="ellipse1712"
         cx="32.252182"
         cy="438.96045"
         rx="109.56506"
         ry="47.203445" />
    </mask>
    <filter
       id="mask-powermask-path-effect1714_inverse"
       inkscape:label="filtermask-powermask-path-effect1714"
       style="color-interpolation-filters:sRGB"
       height="100"
       width="100"
       x="-50"
       y="-50">
      <feColorMatrix
         id="mask-powermask-path-effect1714_primitive1"
         values="1"
         type="saturate"
         result="fbSourceGraphic" />
      <feColorMatrix
         id="mask-powermask-path-effect1714_primitive2"
         values="-1 0 0 0 1 0 -1 0 0 1 0 0 -1 0 1 0 0 0 1 0 "
         in="fbSourceGraphic" />
    </filter>
  </defs>
  <sodipodi:namedview
     id="base"
     pagecolor="#ffffff"
     bordercolor="#666666"
     borderopacity="1.0"
     inkscape:pageopacity="0.0"
     inkscape:pageshadow="2"
     inkscape:zoom="0.175"
     inkscape:cx="413.38095"
     inkscape:cy="1096.5347"
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
     id="metadata950">
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
       style="fill:#008080;fill-rule:evenodd;stroke:#000000;stroke-width:10"
       id="path1571"
       mask="url(#mask-powermask-path-effect1574)"
       inkscape:path-effect="#path-effect1574"
       sodipodi:type="arc"
       sodipodi:cx="-42.751141"
       sodipodi:cy="115.50098"
       sodipodi:rx="175.81261"
       sodipodi:ry="112.08567"
       transform="matrix(2.5180623,0,0,2.5180623,323.20196,-85.135818)"
       d="M 133.06147,115.50098 A 175.81261,112.08567 0 0 1 -42.751141,227.58665 175.81261,112.08567 0 0 1 -218.56375,115.50098 175.81261,112.08567 0 0 1 -42.751141,3.4153061 175.81261,112.08567 0 0 1 133.06147,115.50098 Z" />
    <path
       style="fill:#008080;fill-rule:evenodd;stroke:#000000;stroke-width:10"
       id="path1516"
       mask="url(#mask-powermask-path-effect1524)"
       inkscape:path-effect="#path-effect1524"
       sodipodi:type="arc"
       sodipodi:cx="-42.751141"
       sodipodi:cy="115.50098"
       sodipodi:rx="175.81261"
       sodipodi:ry="112.08567"
       d="M 133.06147,115.50098 A 175.81261,112.08567 0 0 1 -42.751141,227.58665 175.81261,112.08567 0 0 1 -218.56375,115.50098 175.81261,112.08567 0 0 1 -42.751141,3.4153061 175.81261,112.08567 0 0 1 133.06147,115.50098 Z"
       transform="matrix(2.5180623,0,0,2.5180623,1283.4059,-152.1192)" />
    <path
       id="path1553"
       style="fill:#008080;fill-rule:evenodd;stroke:#000000;stroke-width:10"
       d="M 7.6715012,270.59869 A 126.29634,93.746513 0 0 1 -118.62484,364.34521 126.29634,93.746513 0 0 1 -244.92118,270.59869 126.29634,93.746513 0 0 1 -118.62484,176.85218 126.29634,93.746513 0 0 1 7.6715012,270.59869 Z"
       mask="url(#mask-powermask-path-effect1556)"
       inkscape:path-effect="#path-effect1556"
       inkscape:original-d="M 7.6715012,270.59869 A 126.29634,93.746513 0 0 1 -118.62484,364.34521 126.29634,93.746513 0 0 1 -244.92118,270.59869 126.29634,93.746513 0 0 1 -118.62484,176.85218 126.29634,93.746513 0 0 1 7.6715012,270.59869 Z"
       transform="matrix(2.5180623,0,0,2.5180623,478.91959,173.072)" />
    <path
       id="path1532"
       style="fill:#008080;fill-rule:evenodd;stroke:#000000;stroke-width:10"
       d="M 7.6715012,270.59869 A 126.29634,93.746513 0 0 1 -118.62484,364.34521 126.29634,93.746513 0 0 1 -244.92118,270.59869 126.29634,93.746513 0 0 1 -118.62484,176.85218 126.29634,93.746513 0 0 1 7.6715012,270.59869 Z"
       mask="url(#mask-powermask-path-effect1542)"
       inkscape:path-effect="#path-effect1542"
       inkscape:original-d="M 7.6715012,270.59869 A 126.29634,93.746513 0 0 1 -118.62484,364.34521 126.29634,93.746513 0 0 1 -244.92118,270.59869 126.29634,93.746513 0 0 1 -118.62484,176.85218 126.29634,93.746513 0 0 1 7.6715012,270.59869 Z"
       transform="matrix(2.5180623,0,0,2.5180623,1543.3922,158.58438)" />
    <g
       id="g1706"
       mask="url(#mask-powermask-path-effect1714)"
       inkscape:path-effect="#path-effect1714"
       transform="matrix(2.5180623,0,0,2.5180623,363.94858,236.98403)">
      <ellipse
         style="fill:#ffff00;fill-rule:evenodd;stroke:#000000;stroke-width:10"
         id="path1696"
         cx="120.02859"
         cy="484.28946"
         rx="83.072929"
         ry="89.409035" />
      <path
         sodipodi:type="star"
         style="fill:#ffff00;fill-rule:evenodd;stroke:#000000;stroke-width:10"
         id="path1698"
         sodipodi:sides="5"
         sodipodi:cx="-59.314068"
         sodipodi:cy="433.00345"
         sodipodi:r1="112.49419"
         sodipodi:r2="56.247089"
         sodipodi:arg1="0.85428738"
         sodipodi:arg2="1.4826059"
         inkscape:flatsided="false"
         inkscape:rounded="0"
         inkscape:randomized="0"
         d="m 14.567171,517.83566 -68.927212,-28.80371 -62.803699,40.45133 6.09428,-74.45451 -57.87891,-47.22972 72.693685,-17.21171 27.032563,-69.6409 38.832887,63.8171 74.585955,4.18927 -48.6936403,56.65284 z"
         inkscape:transform-center-x="3.0617565"
         inkscape:transform-center-y="-7.7885933" />
      <path
         id="path1700"
         style="fill:#ffff00;fill-rule:evenodd;stroke:#000000;stroke-width:10"
         d="m 171.73651,428.27258 a 75.414436,77.203995 0 0 1 -75.414435,77.204 75.414436,77.203995 0 0 1 -75.414436,-77.204 75.414436,77.203995 0 0 1 75.414436,-77.20399 75.414436,77.203995 0 0 1 75.414435,77.20399 z"
         inkscape:original-d="m 171.73651,428.27258 a 75.414436,77.203995 0 0 1 -75.414435,77.204 75.414436,77.203995 0 0 1 -75.414436,-77.204 75.414436,77.203995 0 0 1 75.414436,-77.20399 75.414436,77.203995 0 0 1 75.414435,77.20399 z" />
    </g>
  </g>
</svg>
)"""";

   testDoc(svg);
}

TEST_F(LPEPowerMaskTest, multi_MM_1_0_2)
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
   id="svg953"
   inkscape:version="1.0.2 (e86c870879, 2021-01-15)"
   sodipodi:docname="1.svg">
  <defs
     id="defs947">
    <inkscape:path-effect
       effect="powermask"
       id="path-effect1714"
       is_visible="true"
       lpeversion="1"
       uri="#mask-powermask-path-effect1714"
       invert="false"
       hide_mask="false"
       background="true"
       background_color="#05ff00ff"
       background_color_opacity_LPE="1" />
    <inkscape:path-effect
       effect="powermask"
       id="path-effect1574"
       is_visible="true"
       lpeversion="1"
       uri="#mask-powermask-path-effect1574"
       invert="false"
       hide_mask="false"
       background="true"
       background_color="#ffffffff" />
    <inkscape:path-effect
       effect="powermask"
       id="path-effect1556"
       is_visible="true"
       lpeversion="1"
       uri="#mask-powermask-path-effect1556"
       invert="false"
       hide_mask="false"
       background="true"
       background_color="#ffffffff" />
    <inkscape:path-effect
       effect="powermask"
       id="path-effect1542"
       is_visible="true"
       lpeversion="1"
       uri="#mask-powermask-path-effect1542"
       invert="false"
       hide_mask="false"
       background="true"
       background_color="#ffffffff" />
    <inkscape:path-effect
       effect="powermask"
       id="path-effect1524"
       is_visible="true"
       lpeversion="1"
       uri="#mask-powermask-path-effect1524"
       invert="false"
       hide_mask="false"
       background="true"
       background_color="#ff0084ff"
       background_color_opacity_LPE="1" />
    <mask
       maskUnits="userSpaceOnUse"
       id="mask-powermask-path-effect1524">
      <path
         id="mask-powermask-path-effect1524_box"
         style="fill:#ff0084;fill-opacity:1"
         d="M -224.56375,-2.5846939 H 139.06147 V 233.58665 h -363.62522 z" />
      <ellipse
         style="fill:#008080;fill-rule:evenodd;stroke:#000000;stroke-width:10"
         id="ellipse1522"
         cx="-54.03064"
         cy="107.55451"
         rx="66.384262"
         ry="49.355526"
         d="M 12.353622,107.55451 A 66.384262,49.355526 0 0 1 -54.03064,156.91004 66.384262,49.355526 0 0 1 -120.4149,107.55451 66.384262,49.355526 0 0 1 -54.03064,58.198986 66.384262,49.355526 0 0 1 12.353622,107.55451 Z" />
    </mask>
    <filter
       id="mask-powermask-path-effect1524_inverse"
       inkscape:label="filtermask-powermask-path-effect1524"
       style="color-interpolation-filters:sRGB"
       height="100"
       width="100"
       x="-50"
       y="-50">
      <feColorMatrix
         id="mask-powermask-path-effect1524_primitive1"
         values="1"
         type="saturate"
         result="fbSourceGraphic" />
      <feColorMatrix
         id="mask-powermask-path-effect1524_primitive2"
         values="-1 0 0 0 1 0 -1 0 0 1 0 0 -1 0 1 0 0 0 1 0 "
         in="fbSourceGraphic" />
    </filter>
    <mask
       maskUnits="userSpaceOnUse"
       id="mask-powermask-path-effect1542">
      <path
         id="mask-powermask-path-effect1542_box"
         style="fill:#ffffff;fill-opacity:1"
         d="M -250.92118,170.85218 H 13.671501 V 370.34521 H -250.92118 Z" />
      <path
         id="path1540"
         style="fill:#ffff00;fill-rule:evenodd;stroke:#000000;stroke-width:10"
         inkscape:transform-center-x="-0.39075553"
         inkscape:transform-center-y="-5.2654283"
         d="m -97.509163,317.8194 -34.307387,-18.98512 -35.08889,17.49862 7.45438,-38.495 -27.48525,-27.96414 38.91445,-4.80607 18.10207,-34.78142 16.59607,35.52468 38.672943,6.46804 -28.657513,26.76152 z" />
    </mask>
    <filter
       id="mask-powermask-path-effect1542_inverse"
       inkscape:label="filtermask-powermask-path-effect1542"
       style="color-interpolation-filters:sRGB"
       height="100"
       width="100"
       x="-50"
       y="-50">
      <feColorMatrix
         id="mask-powermask-path-effect1542_primitive1"
         values="1"
         type="saturate"
         result="fbSourceGraphic" />
      <feColorMatrix
         id="mask-powermask-path-effect1542_primitive2"
         values="-1 0 0 0 1 0 -1 0 0 1 0 0 -1 0 1 0 0 0 1 0 "
         in="fbSourceGraphic" />
    </filter>
    <mask
       maskUnits="userSpaceOnUse"
       id="mask-powermask-path-effect1556">
      <path
         id="mask-powermask-path-effect1556_box"
         style="fill:#ffffff;fill-opacity:1"
         d="M -250.92118,170.85218 H 13.671501 V 370.34521 H -250.92118 Z" />
      <path
         id="path1558"
         style="fill:#ffffff;fill-opacity:1"
         d="M -250.92118,170.85218 H 13.671501 V 370.34521 H -250.92118 Z" />
      <path
         id="path1560"
         style="fill:#ffff00;fill-rule:evenodd;stroke:#000000;stroke-width:10"
         inkscape:transform-center-x="-0.39075553"
         inkscape:transform-center-y="-5.2654283"
         d="m -97.509163,317.8194 -34.307387,-18.98512 -35.08889,17.49862 7.45438,-38.495 -27.48525,-27.96414 38.91445,-4.80607 18.10207,-34.78142 16.59607,35.52468 38.672943,6.46804 -28.657513,26.76152 z" />
    </mask>
    <filter
       id="mask-powermask-path-effect1556_inverse"
       inkscape:label="filtermask-powermask-path-effect1556"
       style="color-interpolation-filters:sRGB"
       height="100"
       width="100"
       x="-50"
       y="-50">
      <feColorMatrix
         id="mask-powermask-path-effect1556_primitive1"
         values="1"
         type="saturate"
         result="fbSourceGraphic" />
      <feColorMatrix
         id="mask-powermask-path-effect1556_primitive2"
         values="-1 0 0 0 1 0 -1 0 0 1 0 0 -1 0 1 0 0 0 1 0 "
         in="fbSourceGraphic" />
    </filter>
    <mask
       maskUnits="userSpaceOnUse"
       id="mask-powermask-path-effect1574">
      <path
         id="mask-powermask-path-effect1574_box"
         style="fill:#ffffff;fill-opacity:1"
         d="M -224.56375,-2.5846939 H 139.06147 V 233.58665 h -363.62522 z" />
      <path
         id="path1576"
         style="fill:#ffffff;fill-opacity:1"
         d="M -224.56375,-2.5846939 H 139.06147 V 233.58665 h -363.62522 z" />
      <ellipse
         style="fill:#008080;fill-rule:evenodd;stroke:#000000;stroke-width:10"
         id="ellipse1578"
         cx="-54.03064"
         cy="107.55451"
         rx="66.384262"
         ry="49.355526"
         d="M 12.353622,107.55451 A 66.384262,49.355526 0 0 1 -54.03064,156.91004 66.384262,49.355526 0 0 1 -120.4149,107.55451 66.384262,49.355526 0 0 1 -54.03064,58.198986 66.384262,49.355526 0 0 1 12.353622,107.55451 Z" />
    </mask>
    <filter
       id="mask-powermask-path-effect1574_inverse"
       inkscape:label="filtermask-powermask-path-effect1574"
       style="color-interpolation-filters:sRGB"
       height="100"
       width="100"
       x="-50"
       y="-50">
      <feColorMatrix
         id="mask-powermask-path-effect1574_primitive1"
         values="1"
         type="saturate"
         result="fbSourceGraphic" />
      <feColorMatrix
         id="mask-powermask-path-effect1574_primitive2"
         values="-1 0 0 0 1 0 -1 0 0 1 0 0 -1 0 1 0 0 0 1 0 "
         in="fbSourceGraphic" />
    </filter>
    <mask
       maskUnits="userSpaceOnUse"
       id="mask-powermask-path-effect1714">
      <path
         id="mask-powermask-path-effect1714_box"
         style="fill:#05ff00;fill-opacity:1"
         d="m -180.95891,308.69259 h 390.06043 v 271.0059 h -390.06043 z" />
      <ellipse
         style="fill:#ffff00;fill-rule:evenodd;stroke:#000000;stroke-width:10"
         id="ellipse1712"
         cx="32.252182"
         cy="438.96045"
         rx="109.56506"
         ry="47.203445" />
    </mask>
    <filter
       id="mask-powermask-path-effect1714_inverse"
       inkscape:label="filtermask-powermask-path-effect1714"
       style="color-interpolation-filters:sRGB"
       height="100"
       width="100"
       x="-50"
       y="-50">
      <feColorMatrix
         id="mask-powermask-path-effect1714_primitive1"
         values="1"
         type="saturate"
         result="fbSourceGraphic" />
      <feColorMatrix
         id="mask-powermask-path-effect1714_primitive2"
         values="-1 0 0 0 1 0 -1 0 0 1 0 0 -1 0 1 0 0 0 1 0 "
         in="fbSourceGraphic" />
    </filter>
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
     inkscape:cy="1360"
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
     id="metadata950">
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
       style="fill:#008080;fill-rule:evenodd;stroke:#000000;stroke-width:10"
       id="path1571"
       mask="url(#mask-powermask-path-effect1574)"
       inkscape:path-effect="#path-effect1574"
       sodipodi:type="arc"
       sodipodi:cx="-42.751141"
       sodipodi:cy="115.50098"
       sodipodi:rx="175.81261"
       sodipodi:ry="112.08567"
       transform="translate(-16.181737,-127.9237)"
       d="M 133.06147,115.50098 A 175.81261,112.08567 0 0 1 -42.751141,227.58665 175.81261,112.08567 0 0 1 -218.56375,115.50098 175.81261,112.08567 0 0 1 -42.751141,3.4153061 175.81261,112.08567 0 0 1 133.06147,115.50098 Z" />
    <path
       style="fill:#008080;fill-rule:evenodd;stroke:#000000;stroke-width:10"
       id="path1516"
       mask="url(#mask-powermask-path-effect1524)"
       inkscape:path-effect="#path-effect1524"
       sodipodi:type="arc"
       sodipodi:cx="-42.751141"
       sodipodi:cy="115.50098"
       sodipodi:rx="175.81261"
       sodipodi:ry="112.08567"
       d="M 133.06147,115.50098 A 175.81261,112.08567 0 0 1 -42.751141,227.58665 175.81261,112.08567 0 0 1 -218.56375,115.50098 175.81261,112.08567 0 0 1 -42.751141,3.4153061 175.81261,112.08567 0 0 1 133.06147,115.50098 Z"
       transform="translate(365.14479,-154.52486)" />
    <path
       id="path1553"
       style="fill:#008080;fill-rule:evenodd;stroke:#000000;stroke-width:10"
       d="M 7.6715012,270.59869 A 126.29634,93.746513 0 0 1 -118.62484,364.34521 126.29634,93.746513 0 0 1 -244.92118,270.59869 126.29634,93.746513 0 0 1 -118.62484,176.85218 126.29634,93.746513 0 0 1 7.6715012,270.59869 Z"
       mask="url(#mask-powermask-path-effect1556)"
       inkscape:path-effect="#path-effect1556"
       inkscape:original-d="M 7.6715012,270.59869 A 126.29634,93.746513 0 0 1 -118.62484,364.34521 126.29634,93.746513 0 0 1 -244.92118,270.59869 126.29634,93.746513 0 0 1 -118.62484,176.85218 126.29634,93.746513 0 0 1 7.6715012,270.59869 Z"
       transform="translate(45.658523,-25.381434)" />
    <path
       id="path1532"
       style="fill:#008080;fill-rule:evenodd;stroke:#000000;stroke-width:10"
       d="M 7.6715012,270.59869 A 126.29634,93.746513 0 0 1 -118.62484,364.34521 126.29634,93.746513 0 0 1 -244.92118,270.59869 126.29634,93.746513 0 0 1 -118.62484,176.85218 126.29634,93.746513 0 0 1 7.6715012,270.59869 Z"
       mask="url(#mask-powermask-path-effect1542)"
       inkscape:path-effect="#path-effect1542"
       inkscape:original-d="M 7.6715012,270.59869 A 126.29634,93.746513 0 0 1 -118.62484,364.34521 126.29634,93.746513 0 0 1 -244.92118,270.59869 126.29634,93.746513 0 0 1 -118.62484,176.85218 126.29634,93.746513 0 0 1 7.6715012,270.59869 Z"
       transform="translate(468.39333,-31.134912)" />
    <g
       id="g1706"
       mask="url(#mask-powermask-path-effect1714)"
       inkscape:path-effect="#path-effect1714">
      <ellipse
         style="fill:#ffff00;fill-rule:evenodd;stroke:#000000;stroke-width:10"
         id="path1696"
         cx="120.02859"
         cy="484.28946"
         rx="83.072929"
         ry="89.409035"
         d="m 203.10152,484.28946 a 83.072929,89.409035 0 0 1 -83.07293,89.40903 83.072929,89.409035 0 0 1 -83.072932,-89.40903 83.072929,89.409035 0 0 1 83.072932,-89.40904 83.072929,89.409035 0 0 1 83.07293,89.40904 z" />
      <path
         sodipodi:type="star"
         style="fill:#ffff00;fill-rule:evenodd;stroke:#000000;stroke-width:10"
         id="path1698"
         sodipodi:sides="5"
         sodipodi:cx="-59.314068"
         sodipodi:cy="433.00345"
         sodipodi:r1="112.49419"
         sodipodi:r2="56.247089"
         sodipodi:arg1="0.85428738"
         sodipodi:arg2="1.4826059"
         inkscape:flatsided="false"
         inkscape:rounded="0"
         inkscape:randomized="0"
         d="m 14.567171,517.83566 -68.927212,-28.80371 -62.803699,40.45133 6.09428,-74.45451 -57.87891,-47.22972 72.693685,-17.21171 27.032563,-69.6409 38.832887,63.8171 74.585955,4.18927 -48.6936403,56.65284 z"
         inkscape:transform-center-x="3.0617565"
         inkscape:transform-center-y="-7.7885933" />
      <path
         id="path1700"
         style="fill:#ffff00;fill-rule:evenodd;stroke:#000000;stroke-width:10"
         d="m 171.73651,428.27258 a 75.414436,77.203995 0 0 1 -75.414435,77.204 75.414436,77.203995 0 0 1 -75.414436,-77.204 75.414436,77.203995 0 0 1 75.414436,-77.20399 75.414436,77.203995 0 0 1 75.414435,77.20399 z"
         inkscape:original-d="m 171.73651,428.27258 a 75.414436,77.203995 0 0 1 -75.414435,77.204 75.414436,77.203995 0 0 1 -75.414436,-77.204 75.414436,77.203995 0 0 1 75.414436,-77.20399 75.414436,77.203995 0 0 1 75.414435,77.20399 z" />
    </g>
  </g>
</svg>
)"""";

   testDoc(svg);
}