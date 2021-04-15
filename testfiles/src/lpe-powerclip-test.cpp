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

class LPEPowerClipTest : public LPESPathsTest {};
// INKSCAPE 1.0.2

TEST_F(LPEPowerClipTest, multi_PX_1_0_2)
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
       effect="powerclip"
       id="path-effect896"
       is_visible="true"
       lpeversion="1"
       inverse="true"
       flatten="false"
       hide_clip="false"
       message="Use fill-rule evenodd on &lt;b&gt;fill and stroke&lt;/b&gt; dialog if no flatten result after convert clip to paths." />
    <inkscape:path-effect
       effect="powerclip"
       id="path-effect887"
       is_visible="true"
       lpeversion="1"
       inverse="true"
       flatten="false"
       hide_clip="false"
       message="Use fill-rule evenodd on &lt;b&gt;fill and stroke&lt;/b&gt; dialog if no flatten result after convert clip to paths." />
    <inkscape:path-effect
       effect="powerclip"
       id="path-effect878"
       is_visible="true"
       lpeversion="1"
       inverse="true"
       flatten="false"
       hide_clip="false"
       message="Use fill-rule evenodd on &lt;b&gt;fill and stroke&lt;/b&gt; dialog if no flatten result after convert clip to paths." />
    <inkscape:path-effect
       effect="powerclip"
       id="path-effect872"
       is_visible="true"
       lpeversion="1"
       inverse="true"
       flatten="true"
       hide_clip="false"
       message="Use fill-rule evenodd on &lt;b&gt;fill and stroke&lt;/b&gt; dialog if no flatten result after convert clip to paths." />
    <inkscape:path-effect
       effect="powerclip"
       id="path-effect854"
       is_visible="true"
       lpeversion="1"
       inverse="true"
       flatten="true"
       hide_clip="false"
       message="Use fill-rule evenodd on &lt;b&gt;fill and stroke&lt;/b&gt; dialog if no flatten result after convert clip to paths." />
    <inkscape:path-effect
       effect="powerclip"
       id="path-effect841"
       is_visible="true"
       lpeversion="1"
       inverse="true"
       flatten="true"
       hide_clip="false"
       message="Use fill-rule evenodd on &lt;b&gt;fill and stroke&lt;/b&gt; dialog if no flatten result after convert clip to paths." />
    <clipPath
       clipPathUnits="userSpaceOnUse"
       id="clipPath837">
      <ellipse
         style="display:none;fill:#008080;fill-rule:evenodd;stroke:#000000;stroke-width:10;stroke-miterlimit:4;stroke-dasharray:none"
         id="ellipse839"
         cx="-67.134285"
         cy="125.55812"
         rx="68.439247"
         ry="63.992504"
         d="m 1.3049622,125.55812 a 68.439247,63.992504 0 0 1 -68.4392472,63.9925 68.439247,63.992504 0 0 1 -68.439245,-63.9925 68.439247,63.992504 0 0 1 68.439245,-63.992503 68.439247,63.992504 0 0 1 68.4392472,63.992503 z" />
      <path
         id="lpe_path-effect841"
         style="fill:#008080;fill-rule:evenodd;stroke:#000000;stroke-width:10;stroke-miterlimit:4;stroke-dasharray:none"
         class="powerclip"
         d="M -222.91331,-13.494339 H 67.721703 V 245.73199 H -222.91331 Z M 1.3049622,125.55812 a 68.439247,63.992504 0 0 0 -68.4392472,-63.992503 68.439247,63.992504 0 0 0 -68.439245,63.992503 68.439247,63.992504 0 0 0 68.439245,63.9925 68.439247,63.992504 0 0 0 68.4392472,-63.9925 z" />
    </clipPath>
    <clipPath
       clipPathUnits="userSpaceOnUse"
       id="clipPath850">
      <path
         sodipodi:type="star"
         style="display:none;fill:#008080;fill-rule:evenodd;stroke:#000000;stroke-width:10;stroke-miterlimit:4;stroke-dasharray:none"
         id="path852"
         sodipodi:sides="5"
         sodipodi:cx="292.73157"
         sodipodi:cy="97.526833"
         sodipodi:r1="81.321312"
         sodipodi:r2="40.66066"
         sodipodi:arg1="0.50033719"
         sodipodi:arg2="1.1286557"
         inkscape:flatsided="false"
         inkscape:rounded="0"
         inkscape:randomized="0"
         d="m 364.08458,136.53841 -53.95532,-2.26094 -32.45061,43.16535 -14.52284,-52.01323 -51.08047,-17.52354 44.97972,-29.885006 0.88113,-53.995489 42.32184,33.543279 51.62504,-15.847511 -18.82338,50.615897 z"
         inkscape:transform-center-x="4.6516115"
         inkscape:transform-center-y="3.2073551" />
      <path
         id="lpe_path-effect854"
         style="fill:#008080;fill-rule:evenodd;stroke:#000000;stroke-width:10;stroke-miterlimit:4;stroke-dasharray:none"
         class="powerclip"
         d="M 208.36109,11.498306 H 458.61535 V 260.07945 H 208.36109 Z M 364.08458,136.53841 333.05969,92.33722 351.88307,41.721323 300.25803,57.568834 257.93619,24.025555 l -0.88113,53.995489 -44.97972,29.885006 51.08047,17.52354 14.52284,52.01323 32.45061,-43.16535 z" />
    </clipPath>
    <clipPath
       clipPathUnits="userSpaceOnUse"
       id="clipPath868">
      <path
         id="path870"
         style="display:none;fill:#008080;fill-rule:evenodd;stroke:#000000;stroke-width:10;stroke-miterlimit:4;stroke-dasharray:none"
         d="m 112.6047,217.88522 h 97.9882 v 76.54216 h -97.9882 z" />
      <path
         id="lpe_path-effect872"
         style="fill:#008080;fill-rule:evenodd;stroke:#000000;stroke-width:10;stroke-miterlimit:4;stroke-dasharray:none"
         class="powerclip"
         d="M 13.868896,153.65791 H 280.46347 v 200.7421 H 13.868896 Z m 98.735804,64.22731 v 76.54216 h 97.9882 v -76.54216 z" />
    </clipPath>
    <clipPath
       clipPathUnits="userSpaceOnUse"
       id="clipath_lpe_path-effect878">
      <path
         id="path880"
         style="display:none;fill:#008080;fill-rule:evenodd;stroke:#000000;stroke-width:10;stroke-miterlimit:4;stroke-dasharray:none"
         d="m 112.6047,217.88522 h 97.9882 v 76.54216 h -97.9882 z" />
      <path
         id="lpe_path-effect878"
         style="fill:#008080;fill-rule:evenodd;stroke:#000000;stroke-width:10;stroke-miterlimit:4;stroke-dasharray:none"
         class="powerclip"
         d="M 13.868896,153.65791 H 280.46347 v 200.7421 H 13.868896 Z m 98.735804,64.22731 v 76.54216 h 97.9882 v -76.54216 z" />
    </clipPath>
    <clipPath
       clipPathUnits="userSpaceOnUse"
       id="clipath_lpe_path-effect887">
      <path
         sodipodi:type="star"
         style="display:none;fill:#008080;fill-rule:evenodd;stroke:#000000;stroke-width:10;stroke-miterlimit:4;stroke-dasharray:none"
         id="path889"
         sodipodi:sides="5"
         sodipodi:cx="292.73157"
         sodipodi:cy="97.526833"
         sodipodi:r1="81.321312"
         sodipodi:r2="40.66066"
         sodipodi:arg1="0.50033719"
         sodipodi:arg2="1.1286557"
         inkscape:flatsided="false"
         inkscape:rounded="0"
         inkscape:randomized="0"
         d="m 364.08458,136.53841 -53.95532,-2.26094 -32.45061,43.16535 -14.52284,-52.01323 -51.08047,-17.52354 44.97972,-29.885006 0.88113,-53.995489 42.32184,33.543279 51.62504,-15.847511 -18.82338,50.615897 z"
         inkscape:transform-center-x="4.6516115"
         inkscape:transform-center-y="3.2073551" />
      <path
         id="lpe_path-effect887"
         style="fill:#008080;fill-rule:evenodd;stroke:#000000;stroke-width:10;stroke-miterlimit:4;stroke-dasharray:none"
         class="powerclip"
         d="M 208.36109,11.498306 H 458.61535 V 260.07945 H 208.36109 Z M 364.08458,136.53841 333.05969,92.33722 351.88307,41.721323 300.25803,57.568834 257.93619,24.025555 l -0.88113,53.995489 -44.97972,29.885006 51.08047,17.52354 14.52284,52.01323 32.45061,-43.16535 z" />
    </clipPath>
    <clipPath
       clipPathUnits="userSpaceOnUse"
       id="clipath_lpe_path-effect896">
      <ellipse
         style="display:none;fill:#008080;fill-rule:evenodd;stroke:#000000;stroke-width:10;stroke-miterlimit:4;stroke-dasharray:none"
         id="ellipse898"
         cx="-67.134285"
         cy="125.55812"
         rx="68.439247"
         ry="63.992504"
         d="m 1.3049622,125.55812 a 68.439247,63.992504 0 0 1 -68.4392472,63.9925 68.439247,63.992504 0 0 1 -68.439245,-63.9925 68.439247,63.992504 0 0 1 68.439245,-63.992503 68.439247,63.992504 0 0 1 68.4392472,63.992503 z" />
      <path
         id="lpe_path-effect896"
         style="fill:#008080;fill-rule:evenodd;stroke:#000000;stroke-width:10;stroke-miterlimit:4;stroke-dasharray:none"
         class="powerclip"
         d="M -222.91331,-13.494339 H 67.721703 V 245.73199 H -222.91331 Z M 1.3049622,125.55812 a 68.439247,63.992504 0 0 0 -68.4392472,-63.992503 68.439247,63.992504 0 0 0 -68.439245,63.992503 68.439247,63.992504 0 0 0 68.439245,63.9925 68.439247,63.992504 0 0 0 68.4392472,-63.9925 z" />
    </clipPath>
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
       style="fill:#008080;fill-rule:evenodd;stroke:#000000;stroke-width:10;stroke-miterlimit:4;stroke-dasharray:none"
       id="path894"
       clip-path="url(#clipath_lpe_path-effect896)"
       inkscape:path-effect="#path-effect896"
       sodipodi:type="arc"
       sodipodi:cx="-77.595802"
       sodipodi:cy="116.11883"
       sodipodi:rx="135.3175"
       sodipodi:ry="119.61317"
       transform="matrix(2.2732151,0,0,2.2732151,94.109962,-54.675694)"
       d="M 57.721703,116.11883 A 135.3175,119.61317 0 0 1 -77.595802,235.73199 135.3175,119.61317 0 0 1 -212.91331,116.11883 135.3175,119.61317 0 0 1 -77.595802,-3.494339 135.3175,119.61317 0 0 1 57.721703,116.11883 Z" />
    <path
       style="fill:#008080;fill-rule:evenodd;stroke:#000000;stroke-width:10;stroke-miterlimit:4;stroke-dasharray:none"
       id="path833"
       clip-path="url(#clipPath837)"
       inkscape:path-effect="#path-effect841"
       sodipodi:type="arc"
       sodipodi:cx="-77.595802"
       sodipodi:cy="116.11883"
       sodipodi:rx="135.3175"
       sodipodi:ry="119.61317"
       d="m 1.3049622,125.55812 a 68.439247,63.992504 0 0 0 -68.4392472,-63.992503 68.439247,63.992504 0 0 0 -68.439245,63.992503 68.439247,63.992504 0 0 0 68.439245,63.9925 68.439247,63.992504 0 0 0 68.4392472,-63.9925 z M 57.721703,116.11883 A 135.3175,119.61317 0 0 1 -77.595802,235.73199 135.3175,119.61317 0 0 1 -212.91331,116.11883 135.3175,119.61317 0 0 1 -77.595802,-3.494339 135.3175,119.61317 0 0 1 57.721703,116.11883 Z"
       transform="matrix(2.2732151,0,0,2.2732151,685.82948,-146.06667)" />
    <path
       id="path885"
       style="fill:#008080;fill-rule:evenodd;stroke:#000000;stroke-width:10;stroke-miterlimit:4;stroke-dasharray:none"
       d="M 448.61535,135.78888 A 115.12713,114.29057 0 0 1 333.48822,250.07945 115.12713,114.29057 0 0 1 218.36109,135.78888 115.12713,114.29057 0 0 1 333.48822,21.498306 115.12713,114.29057 0 0 1 448.61535,135.78888 Z"
       clip-path="url(#clipath_lpe_path-effect887)"
       inkscape:path-effect="#path-effect887"
       inkscape:original-d="M 448.61535,135.78888 A 115.12713,114.29057 0 0 1 333.48822,250.07945 115.12713,114.29057 0 0 1 218.36109,135.78888 115.12713,114.29057 0 0 1 333.48822,21.498306 115.12713,114.29057 0 0 1 448.61535,135.78888 Z"
       transform="matrix(2.2732151,0,0,2.2732151,278.90635,-126.04217)" />
    <path
       id="path845"
       style="fill:#008080;fill-rule:evenodd;stroke:#000000;stroke-width:10;stroke-miterlimit:4;stroke-dasharray:none"
       d="m 274.85215,37.432736 25.40588,20.136098 51.62504,-15.847511 -18.82338,50.615897 31.02489,44.20119 -53.95532,-2.26094 -32.45061,43.16535 -14.52284,-52.01323 -42.05469,-14.42717 A 115.12713,114.29057 0 0 0 218.36109,135.78888 115.12713,114.29057 0 0 0 333.48822,250.07945 115.12713,114.29057 0 0 0 448.61535,135.78888 115.12713,114.29057 0 0 0 333.48822,21.498306 115.12713,114.29057 0 0 0 274.85215,37.432736 Z m -17.33849,12.485417 -0.4586,28.102891 -32.85378,21.828405 a 115.12713,114.29057 0 0 1 33.31238,-49.931296 z"
       clip-path="url(#clipPath850)"
       inkscape:path-effect="#path-effect854"
       inkscape:original-d="M 448.61535,135.78888 A 115.12713,114.29057 0 0 1 333.48822,250.07945 115.12713,114.29057 0 0 1 218.36109,135.78888 115.12713,114.29057 0 0 1 333.48822,21.498306 115.12713,114.29057 0 0 1 448.61535,135.78888 Z"
       transform="matrix(2.2732151,0,0,2.2732151,847.78811,-126.04217)" />
    <path
       id="path876"
       style="fill:#008080;fill-rule:evenodd;stroke:#000000;stroke-width:10;stroke-miterlimit:4;stroke-dasharray:none"
       d="M 270.46347,254.02896 A 123.29729,90.371048 0 0 1 147.16618,344.40001 123.29729,90.371048 0 0 1 23.868896,254.02896 123.29729,90.371048 0 0 1 147.16618,163.65791 123.29729,90.371048 0 0 1 270.46347,254.02896 Z"
       clip-path="url(#clipath_lpe_path-effect878)"
       inkscape:path-effect="#path-effect878"
       inkscape:original-d="M 270.46347,254.02896 A 123.29729,90.371048 0 0 1 147.16618,344.40001 123.29729,90.371048 0 0 1 23.868896,254.02896 123.29729,90.371048 0 0 1 147.16618,163.65791 123.29729,90.371048 0 0 1 270.46347,254.02896 Z"
       transform="matrix(2.2732151,0,0,2.2732151,175.05263,203.33194)" />
    <path
       id="path858"
       style="fill:#008080;fill-rule:evenodd;stroke:#000000;stroke-width:10;stroke-miterlimit:4;stroke-dasharray:none"
       d="m 112.6047,217.88522 0,76.54216 h 97.9882 v -76.54216 z m 157.85877,36.14374 A 123.29729,90.371048 0 0 1 147.16618,344.40001 123.29729,90.371048 0 0 1 23.868896,254.02896 123.29729,90.371048 0 0 1 147.16618,163.65791 123.29729,90.371048 0 0 1 270.46347,254.02896 Z"
       clip-path="url(#clipPath868)"
       inkscape:path-effect="#path-effect872"
       inkscape:original-d="M 270.46347,254.02896 A 123.29729,90.371048 0 0 1 147.16618,344.40001 123.29729,90.371048 0 0 1 23.868896,254.02896 123.29729,90.371048 0 0 1 147.16618,163.65791 123.29729,90.371048 0 0 1 270.46347,254.02896 Z"
       transform="matrix(2.2732151,0,0,2.2732151,862.05451,240.77309)" />
  </g>
</svg>
)"""";

   testDoc(svg);
}

TEST_F(LPEPowerClipTest, multi_MM_1_0_2)
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
       effect="powerclip"
       id="path-effect896"
       is_visible="true"
       lpeversion="1"
       inverse="true"
       flatten="false"
       hide_clip="false"
       message="Use fill-rule evenodd on &lt;b&gt;fill and stroke&lt;/b&gt; dialog if no flatten result after convert clip to paths." />
    <inkscape:path-effect
       effect="powerclip"
       id="path-effect887"
       is_visible="true"
       lpeversion="1"
       inverse="true"
       flatten="false"
       hide_clip="false"
       message="Use fill-rule evenodd on &lt;b&gt;fill and stroke&lt;/b&gt; dialog if no flatten result after convert clip to paths." />
    <inkscape:path-effect
       effect="powerclip"
       id="path-effect878"
       is_visible="true"
       lpeversion="1"
       inverse="true"
       flatten="false"
       hide_clip="false"
       message="Use fill-rule evenodd on &lt;b&gt;fill and stroke&lt;/b&gt; dialog if no flatten result after convert clip to paths." />
    <inkscape:path-effect
       effect="powerclip"
       id="path-effect872"
       is_visible="true"
       lpeversion="1"
       inverse="true"
       flatten="true"
       hide_clip="false"
       message="Use fill-rule evenodd on &lt;b&gt;fill and stroke&lt;/b&gt; dialog if no flatten result after convert clip to paths." />
    <inkscape:path-effect
       effect="powerclip"
       id="path-effect854"
       is_visible="true"
       lpeversion="1"
       inverse="true"
       flatten="true"
       hide_clip="false"
       message="Use fill-rule evenodd on &lt;b&gt;fill and stroke&lt;/b&gt; dialog if no flatten result after convert clip to paths." />
    <inkscape:path-effect
       effect="powerclip"
       id="path-effect841"
       is_visible="true"
       lpeversion="1"
       inverse="true"
       flatten="true"
       hide_clip="false"
       message="Use fill-rule evenodd on &lt;b&gt;fill and stroke&lt;/b&gt; dialog if no flatten result after convert clip to paths." />
    <clipPath
       clipPathUnits="userSpaceOnUse"
       id="clipPath837">
      <ellipse
         style="display:none;fill:#008080;fill-rule:evenodd;stroke:#000000;stroke-width:10;stroke-miterlimit:4;stroke-dasharray:none"
         id="ellipse839"
         cx="-67.134285"
         cy="125.55812"
         rx="68.439247"
         ry="63.992504"
         d="m 1.3049622,125.55812 a 68.439247,63.992504 0 0 1 -68.4392472,63.9925 68.439247,63.992504 0 0 1 -68.439245,-63.9925 68.439247,63.992504 0 0 1 68.439245,-63.992503 68.439247,63.992504 0 0 1 68.4392472,63.992503 z" />
      <path
         id="lpe_path-effect841"
         style="fill:#008080;fill-rule:evenodd;stroke:#000000;stroke-width:10;stroke-miterlimit:4;stroke-dasharray:none"
         class="powerclip"
         d="M -222.91331,-13.494339 H 67.721703 V 245.73199 H -222.91331 Z M 1.3049622,125.55812 a 68.439247,63.992504 0 0 0 -68.4392472,-63.992503 68.439247,63.992504 0 0 0 -68.439245,63.992503 68.439247,63.992504 0 0 0 68.439245,63.9925 68.439247,63.992504 0 0 0 68.4392472,-63.9925 z" />
    </clipPath>
    <clipPath
       clipPathUnits="userSpaceOnUse"
       id="clipPath850">
      <path
         sodipodi:type="star"
         style="display:none;fill:#008080;fill-rule:evenodd;stroke:#000000;stroke-width:10;stroke-miterlimit:4;stroke-dasharray:none"
         id="path852"
         sodipodi:sides="5"
         sodipodi:cx="292.73157"
         sodipodi:cy="97.526833"
         sodipodi:r1="81.321312"
         sodipodi:r2="40.66066"
         sodipodi:arg1="0.50033719"
         sodipodi:arg2="1.1286557"
         inkscape:flatsided="false"
         inkscape:rounded="0"
         inkscape:randomized="0"
         d="m 364.08458,136.53841 -53.95532,-2.26094 -32.45061,43.16535 -14.52284,-52.01323 -51.08047,-17.52354 44.97972,-29.885006 0.88113,-53.995489 42.32184,33.543279 51.62504,-15.847511 -18.82338,50.615897 z"
         inkscape:transform-center-x="4.6516115"
         inkscape:transform-center-y="3.2073551" />
      <path
         id="lpe_path-effect854"
         style="fill:#008080;fill-rule:evenodd;stroke:#000000;stroke-width:10;stroke-miterlimit:4;stroke-dasharray:none"
         class="powerclip"
         d="M 208.36109,11.498306 H 458.61535 V 260.07945 H 208.36109 Z M 364.08458,136.53841 333.05969,92.33722 351.88307,41.721323 300.25803,57.568834 257.93619,24.025555 l -0.88113,53.995489 -44.97972,29.885006 51.08047,17.52354 14.52284,52.01323 32.45061,-43.16535 z" />
    </clipPath>
    <clipPath
       clipPathUnits="userSpaceOnUse"
       id="clipPath868">
      <path
         id="path870"
         style="display:none;fill:#008080;fill-rule:evenodd;stroke:#000000;stroke-width:10;stroke-miterlimit:4;stroke-dasharray:none"
         d="m 112.6047,217.88522 h 97.9882 v 76.54216 h -97.9882 z" />
      <path
         id="lpe_path-effect872"
         style="fill:#008080;fill-rule:evenodd;stroke:#000000;stroke-width:10;stroke-miterlimit:4;stroke-dasharray:none"
         class="powerclip"
         d="M 13.868896,153.65791 H 280.46347 v 200.7421 H 13.868896 Z m 98.735804,64.22731 v 76.54216 h 97.9882 v -76.54216 z" />
    </clipPath>
    <clipPath
       clipPathUnits="userSpaceOnUse"
       id="clipath_lpe_path-effect878">
      <path
         id="path880"
         style="display:none;fill:#008080;fill-rule:evenodd;stroke:#000000;stroke-width:10;stroke-miterlimit:4;stroke-dasharray:none"
         d="m 112.6047,217.88522 h 97.9882 v 76.54216 h -97.9882 z" />
      <path
         id="lpe_path-effect878"
         style="fill:#008080;fill-rule:evenodd;stroke:#000000;stroke-width:10;stroke-miterlimit:4;stroke-dasharray:none"
         class="powerclip"
         d="M 13.868896,153.65791 H 280.46347 v 200.7421 H 13.868896 Z m 98.735804,64.22731 v 76.54216 h 97.9882 v -76.54216 z" />
    </clipPath>
    <clipPath
       clipPathUnits="userSpaceOnUse"
       id="clipath_lpe_path-effect887">
      <path
         sodipodi:type="star"
         style="display:none;fill:#008080;fill-rule:evenodd;stroke:#000000;stroke-width:10;stroke-miterlimit:4;stroke-dasharray:none"
         id="path889"
         sodipodi:sides="5"
         sodipodi:cx="292.73157"
         sodipodi:cy="97.526833"
         sodipodi:r1="81.321312"
         sodipodi:r2="40.66066"
         sodipodi:arg1="0.50033719"
         sodipodi:arg2="1.1286557"
         inkscape:flatsided="false"
         inkscape:rounded="0"
         inkscape:randomized="0"
         d="m 364.08458,136.53841 -53.95532,-2.26094 -32.45061,43.16535 -14.52284,-52.01323 -51.08047,-17.52354 44.97972,-29.885006 0.88113,-53.995489 42.32184,33.543279 51.62504,-15.847511 -18.82338,50.615897 z"
         inkscape:transform-center-x="4.6516115"
         inkscape:transform-center-y="3.2073551" />
      <path
         id="lpe_path-effect887"
         style="fill:#008080;fill-rule:evenodd;stroke:#000000;stroke-width:10;stroke-miterlimit:4;stroke-dasharray:none"
         class="powerclip"
         d="M 208.36109,11.498306 H 458.61535 V 260.07945 H 208.36109 Z M 364.08458,136.53841 333.05969,92.33722 351.88307,41.721323 300.25803,57.568834 257.93619,24.025555 l -0.88113,53.995489 -44.97972,29.885006 51.08047,17.52354 14.52284,52.01323 32.45061,-43.16535 z" />
    </clipPath>
    <clipPath
       clipPathUnits="userSpaceOnUse"
       id="clipath_lpe_path-effect896">
      <ellipse
         style="display:none;fill:#008080;fill-rule:evenodd;stroke:#000000;stroke-width:10;stroke-miterlimit:4;stroke-dasharray:none"
         id="ellipse898"
         cx="-67.134285"
         cy="125.55812"
         rx="68.439247"
         ry="63.992504"
         d="m 1.3049622,125.55812 a 68.439247,63.992504 0 0 1 -68.4392472,63.9925 68.439247,63.992504 0 0 1 -68.439245,-63.9925 68.439247,63.992504 0 0 1 68.439245,-63.992503 68.439247,63.992504 0 0 1 68.4392472,63.992503 z" />
      <path
         id="lpe_path-effect896"
         style="fill:#008080;fill-rule:evenodd;stroke:#000000;stroke-width:10;stroke-miterlimit:4;stroke-dasharray:none"
         class="powerclip"
         d="M -222.91331,-13.494339 H 67.721703 V 245.73199 H -222.91331 Z M 1.3049622,125.55812 a 68.439247,63.992504 0 0 0 -68.4392472,-63.992503 68.439247,63.992504 0 0 0 -68.439245,63.992503 68.439247,63.992504 0 0 0 68.439245,63.9925 68.439247,63.992504 0 0 0 68.4392472,-63.9925 z" />
    </clipPath>
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
       style="fill:#008080;fill-rule:evenodd;stroke:#000000;stroke-width:10;stroke-miterlimit:4;stroke-dasharray:none"
       id="path894"
       clip-path="url(#clipath_lpe_path-effect896)"
       inkscape:path-effect="#path-effect896"
       sodipodi:type="arc"
       sodipodi:cx="-77.595802"
       sodipodi:cy="116.11883"
       sodipodi:rx="135.3175"
       sodipodi:ry="119.61317"
       transform="translate(-183.34055,-116.67839)"
       d="M 57.721703,116.11883 A 135.3175,119.61317 0 0 1 -77.595802,235.73199 135.3175,119.61317 0 0 1 -212.91331,116.11883 135.3175,119.61317 0 0 1 -77.595802,-3.494339 135.3175,119.61317 0 0 1 57.721703,116.11883 Z" />
    <path
       style="fill:#008080;fill-rule:evenodd;stroke:#000000;stroke-width:10;stroke-miterlimit:4;stroke-dasharray:none"
       id="path833"
       clip-path="url(#clipPath837)"
       inkscape:path-effect="#path-effect841"
       sodipodi:type="arc"
       sodipodi:cx="-77.595802"
       sodipodi:cy="116.11883"
       sodipodi:rx="135.3175"
       sodipodi:ry="119.61317"
       d="m 1.3049622,125.55812 a 68.439247,63.992504 0 0 0 -68.4392472,-63.992503 68.439247,63.992504 0 0 0 -68.439245,63.992503 68.439247,63.992504 0 0 0 68.439245,63.9925 68.439247,63.992504 0 0 0 68.4392472,-63.9925 z M 57.721703,116.11883 A 135.3175,119.61317 0 0 1 -77.595802,235.73199 135.3175,119.61317 0 0 1 -212.91331,116.11883 135.3175,119.61317 0 0 1 -77.595802,-3.494339 135.3175,119.61317 0 0 1 57.721703,116.11883 Z"
       transform="translate(76.960165,-156.88179)" />
    <path
       id="path885"
       style="fill:#008080;fill-rule:evenodd;stroke:#000000;stroke-width:10;stroke-miterlimit:4;stroke-dasharray:none"
       d="M 448.61535,135.78888 A 115.12713,114.29057 0 0 1 333.48822,250.07945 115.12713,114.29057 0 0 1 218.36109,135.78888 115.12713,114.29057 0 0 1 333.48822,21.498306 115.12713,114.29057 0 0 1 448.61535,135.78888 Z"
       clip-path="url(#clipath_lpe_path-effect887)"
       inkscape:path-effect="#path-effect887"
       inkscape:original-d="M 448.61535,135.78888 A 115.12713,114.29057 0 0 1 333.48822,250.07945 115.12713,114.29057 0 0 1 218.36109,135.78888 115.12713,114.29057 0 0 1 333.48822,21.498306 115.12713,114.29057 0 0 1 448.61535,135.78888 Z"
       transform="translate(-102.04759,-148.0729)" />
    <path
       id="path845"
       style="fill:#008080;fill-rule:evenodd;stroke:#000000;stroke-width:10;stroke-miterlimit:4;stroke-dasharray:none"
       d="m 274.85215,37.432736 25.40588,20.136098 51.62504,-15.847511 -18.82338,50.615897 31.02489,44.20119 -53.95532,-2.26094 -32.45061,43.16535 -14.52284,-52.01323 -42.05469,-14.42717 A 115.12713,114.29057 0 0 0 218.36109,135.78888 115.12713,114.29057 0 0 0 333.48822,250.07945 115.12713,114.29057 0 0 0 448.61535,135.78888 115.12713,114.29057 0 0 0 333.48822,21.498306 115.12713,114.29057 0 0 0 274.85215,37.432736 Z m -17.33849,12.485417 -0.4586,28.102891 -32.85378,21.828405 a 115.12713,114.29057 0 0 1 33.31238,-49.931296 z"
       clip-path="url(#clipPath850)"
       inkscape:path-effect="#path-effect854"
       inkscape:original-d="M 448.61535,135.78888 A 115.12713,114.29057 0 0 1 333.48822,250.07945 115.12713,114.29057 0 0 1 218.36109,135.78888 115.12713,114.29057 0 0 1 333.48822,21.498306 115.12713,114.29057 0 0 1 448.61535,135.78888 Z"
       transform="translate(148.20667,-148.0729)" />
    <path
       id="path876"
       style="fill:#008080;fill-rule:evenodd;stroke:#000000;stroke-width:10;stroke-miterlimit:4;stroke-dasharray:none"
       d="M 270.46347,254.02896 A 123.29729,90.371048 0 0 1 147.16618,344.40001 123.29729,90.371048 0 0 1 23.868896,254.02896 123.29729,90.371048 0 0 1 147.16618,163.65791 123.29729,90.371048 0 0 1 270.46347,254.02896 Z"
       clip-path="url(#clipath_lpe_path-effect878)"
       inkscape:path-effect="#path-effect878"
       inkscape:original-d="M 270.46347,254.02896 A 123.29729,90.371048 0 0 1 147.16618,344.40001 123.29729,90.371048 0 0 1 23.868896,254.02896 123.29729,90.371048 0 0 1 147.16618,163.65791 123.29729,90.371048 0 0 1 270.46347,254.02896 Z"
       transform="translate(-147.73342,-3.1793925)" />
    <path
       id="path858"
       style="fill:#008080;fill-rule:evenodd;stroke:#000000;stroke-width:10;stroke-miterlimit:4;stroke-dasharray:none"
       d="m 112.6047,217.88522 0,76.54216 h 97.9882 v -76.54216 z m 157.85877,36.14374 A 123.29729,90.371048 0 0 1 147.16618,344.40001 123.29729,90.371048 0 0 1 23.868896,254.02896 123.29729,90.371048 0 0 1 147.16618,163.65791 123.29729,90.371048 0 0 1 270.46347,254.02896 Z"
       clip-path="url(#clipPath868)"
       inkscape:path-effect="#path-effect872"
       inkscape:original-d="M 270.46347,254.02896 A 123.29729,90.371048 0 0 1 147.16618,344.40001 123.29729,90.371048 0 0 1 23.868896,254.02896 123.29729,90.371048 0 0 1 147.16618,163.65791 123.29729,90.371048 0 0 1 270.46347,254.02896 Z"
       transform="translate(154.48254,13.291178)" />
  </g>
</svg>
)"""";

   testDoc(svg);
}