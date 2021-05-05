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

class LPESimplifyTest : public LPESPathsTest {};

// TEST fails if multiple simplify on stack

// INKSCAPE 0.92.5

TEST_F(LPESimplifyTest, path_0_92_5)
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
       effect="simplify"
       id="path-effect37"
       is_visible="true"
       steps="7"
       threshold="0.01"
       smooth_angles="70"
       helper_size="5"
       simplify_individual_paths="false"
       simplify_just_coalesce="false" />
    <inkscape:path-effect
       effect="simplify"
       id="path-effect17"
       is_visible="true"
       steps="3"
       threshold="0.05"
       smooth_angles="0"
       helper_size="5"
       simplify_individual_paths="false"
       simplify_just_coalesce="false" />
    <inkscape:path-effect
       effect="simplify"
       id="path-effect12"
       is_visible="true"
       steps="1"
       threshold="0.2"
       smooth_angles="0"
       helper_size="5"
       simplify_individual_paths="false"
       simplify_just_coalesce="false" />
  </defs>
  <sodipodi:namedview
     id="base"
     pagecolor="#ffffff"
     bordercolor="#666666"
     borderopacity="1.0"
     inkscape:pageopacity="0.0"
     inkscape:pageshadow="2"
     inkscape:zoom="0.35"
     inkscape:cx="-97.142857"
     inkscape:cy="560"
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
       style="fill:none;fill-rule:evenodd;stroke:#000000;stroke-width:0.86199427"
       id="path10"
       sodipodi:sides="5"
       sodipodi:cx="-21.922619"
       sodipodi:cy="38.464287"
       sodipodi:r1="95.552498"
       sodipodi:r2="47.776249"
       sodipodi:arg1="0.64508339"
       sodipodi:arg2="1.2734019"
       inkscape:flatsided="false"
       inkscape:rounded="0"
       inkscape:randomized="0"
       d="M 54.428569,95.916666 C -59.441148,98.70858 -32.089142,127.28252 -108.84592,41.295713 -50.491299,-9.1658611 -60.94788,-59.488976 33.451831,-12.868901 81.878401,-27.015301 -9.2712258,60.957124 54.428569,95.916666 Z"
       inkscape:transform-center-x="8.6523977"
       inkscape:transform-center-y="-0.49499898"
       inkscape:path-effect="#path-effect12" />
    <path
       style="fill:none;fill-rule:evenodd;stroke:#000000;stroke-width:0.86199427"
       inkscape:transform-center-x="6.0612798"
       inkscape:transform-center-y="-4.4298032"
       d="M -9.8273869,249.37498 C -44.066014,205.05236 -149.0589,269.37406 -132.88626,259.10689 c 18.18119,-46.11161 -57.0837,-101.88392 -49.72481,-100.70204 49.47304,3.04207 79.25755,-85.773681 80.40752,-78.409754 16.183173,48.319014 102.42885612,48.739704 99.5934429,53.903444 -34.3995849,30.78447 -30.0230149,79.18789 -7.2172798,115.47644 z"
       id="path14"
       inkscape:path-effect="#path-effect17"
       inkscape:original-d="m -9.8273869,249.37498 -68.8116791,-21.46059 -56.689124,44.51907 -0.85374,-72.07548 -59.85806,-40.15741 68.28404,-23.0845 19.69481,-69.337724 43.055596,57.808474 72.0301198,-2.69566 -41.6742168,58.81211 z" />
    <g
       id="g35"
       inkscape:path-effect="#path-effect37">
      <path
         id="path19"
         d="m 191.25594,159.41665 c -18.48766,-6.95696 -41.36237,-26.8838 -60.48607,-8.06902 -13.10783,12.89609 -22.46967,7.75066 -18.80176,-13.41435 3.84698,-22.19831 -17.22862,-36.45558 -29.666503,-51.458763 15.437041,-10.54604 46.313973,-2.449765 47.993713,-28.644643 0.76887,-11.990193 14.90952,-27.847479 21.01629,-7.844831 7.26545,23.797878 43.64957,10.414682 53.22788,27.788853 8.20802,14.888595 -18.39745,20.287566 -20.74398,39.508194 -1.77041,14.50159 3.499,28.47868 7.46043,42.13456 z"
         inkscape:transform-center-y="-5.746647"
         inkscape:transform-center-x="1.1785038"
         style="fill:none;fill-rule:evenodd;stroke:#000000;stroke-width:0.86199427"
         inkscape:original-d="m 191.25594,159.41665 -43.45028,-19.98194 -41.09328,24.46523 5.57709,-47.49843 -35.966346,-31.521863 46.897106,-9.373701 18.86486,-43.946814 23.40692,41.705169 47.62547,4.361234 -32.43084,35.148915 z" />
      <path
         id="path21"
         d="m 177.64881,248.61904 c -19.76792,-3.90285 -45.5149,-32.75483 -58.99174,-13.64229 -7.22546,10.24698 -42.567633,28.01699 -33.868633,3.29428 7.985039,-22.69362 -13.228695,-35.8789 -28.921696,-53.59637 -12.432592,-14.03645 6.567999,-15.69761 18.971313,-18.18354 20.012341,-4.01096 20.898617,-28.59919 32.559616,-44.18956 13.59578,-18.17711 21.61055,10.54849 26.67655,20.28302 11.96273,22.98689 72.29586,5.30619 56.441,21.63767 -12.27172,12.64063 -26.3798,24.85632 -24.19487,46.06884 1.38414,13.43799 8.4146,25.26874 11.32846,38.32795 z"
         inkscape:transform-center-y="-5.0438939"
         inkscape:transform-center-x="2.7598212"
         style="fill:none;fill-rule:evenodd;stroke:#000000;stroke-width:0.86199427"
         inkscape:original-d="m 177.64881,248.61904 -49.96309,-19.56673 -44.443449,30.06571 3.169627,-53.56417 -42.327973,-32.97742 51.922024,-13.53775 18.283321,-50.44687 28.91995,45.19738 53.62769,1.79954 -34.04851,41.47126 z" />
      <path
         id="path23"
         d="m 208.64287,308.33928 c -18.45909,4.56079 -52.3219,-3.5329 -55.93707,21.41976 -2.6221,18.09828 -16.24869,25.47673 -24.84657,3.08418 -7.5262,-19.60144 -30.670383,-27.74917 -50.230727,-26.30664 -8.372361,-20.73048 30.282957,-21.45045 26.178707,-45.0754 -1.73214,-9.97057 -10.296233,-45.03995 8.4141,-29.88924 17.51084,14.17942 35.77187,3.46915 57.92601,-0.7378 23.93264,-4.54468 5.65722,18.32112 6.00811,30.15259 0.56626,19.09316 15.61358,32.81295 29.44714,43.7738 0,0 1.55678,1.75804 1.55678,1.75804 0,0 1.48352,1.82071 1.48352,1.82071 z"
         inkscape:transform-center-y="6.3957149"
         inkscape:transform-center-x="0.69036649"
         style="fill:none;fill-rule:evenodd;stroke:#000000;stroke-width:0.86199427"
         inkscape:original-d="m 208.64287,308.33928 -48.74659,5.58415 -23.0351,43.32202 -20.37437,-44.63517 -48.319919,-8.52045 36.154539,-33.17019 -6.828251,-48.58794 42.719101,24.13486 44.09983,-21.50856 -9.75269,48.08636 z" />
      <path
         id="path25"
         d="m 234.34523,208.93155 c 1.72408,20.71298 -13.82545,50.62725 -35.82949,44.74695 -25.97148,-6.94055 -31.03197,-38.30462 -27.30855,-62.2095 3.28968,-21.12023 32.89522,-36.2639 49.48701,-19.27655 9.43605,9.66101 13.65663,23.44761 13.65103,36.7391 z"
         style="fill:none;fill-rule:evenodd;stroke:#000000;stroke-width:0.86199427"
         inkscape:original-d="m 234.34523,208.93155 a 32.505951,44.979168 0 0 1 -32.50595,44.97917 32.505951,44.979168 0 0 1 -32.50595,-44.97917 32.505951,44.979168 0 0 1 32.50595,-44.97917 32.505951,44.979168 0 0 1 32.50595,44.97917 z" />
    </g>
  </g>
</svg>
)"""";

   testDoc(svg);
}


// INKSCAPE 1.0.2

TEST_F(LPESimplifyTest, multi_PX_1_0_2)
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
       effect="simplify"
       id="path-effect37"
       is_visible="true"
       steps="1"
       threshold="0.05"
       smooth_angles="70"
       helper_size="5"
       simplify_individual_paths="false"
       simplify_just_coalesce="false"
       lpeversion="1" />
    <inkscape:path-effect
       effect="simplify"
       id="path-effect17"
       is_visible="true"
       steps="1"
       threshold="0.05"
       smooth_angles="0"
       helper_size="5"
       simplify_individual_paths="false"
       simplify_just_coalesce="false"
       lpeversion="1" />
    <inkscape:path-effect
       effect="simplify"
       id="path-effect12"
       is_visible="true"
       steps="1"
       threshold="0.2"
       smooth_angles="0"
       helper_size="5"
       simplify_individual_paths="false"
       simplify_just_coalesce="false"
       lpeversion="1" />
  </defs>
  <sodipodi:namedview
     id="base"
     pagecolor="#ffffff"
     bordercolor="#666666"
     borderopacity="1.0"
     inkscape:pageopacity="0.0"
     inkscape:pageshadow="2"
     inkscape:zoom="0.35"
     inkscape:cx="-97.142857"
     inkscape:cy="560"
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
       style="fill:none;fill-rule:evenodd;stroke:#000000;stroke-width:2.89863"
       id="path10"
       sodipodi:sides="5"
       sodipodi:cx="228.92471"
       sodipodi:cy="-145.62271"
       sodipodi:r1="321.31464"
       sodipodi:r2="160.65732"
       sodipodi:arg1="0.64508339"
       sodipodi:arg2="1.2734019"
       inkscape:flatsided="false"
       inkscape:rounded="0"
       inkscape:randomized="0"
       d="m 485.67105,47.572557 c -976.32528,270.378213 3.41735,820.585083 -43.07481,-85.378225 0,0 43.07481,85.378225 43.07481,85.378225 z"
       inkscape:transform-center-x="27.341886"
       inkscape:transform-center-y="-120.25863"
       inkscape:path-effect="#path-effect12"
       transform="matrix(1,0,0,2.1048645,-207.37653,-418.96435)" />
    <path
       style="fill:none;fill-rule:evenodd;stroke:#000000;stroke-width:2.89863"
       inkscape:transform-center-x="-34.120645"
       inkscape:transform-center-y="48.195994"
       d="M 548.08444,1087.9707 C -138.67797,1077.3896 -173.3864,756.08332 -39.270495,814.14887 167.28003,658.26156 827.61707,897.69424 509.16065,666.22054 713.92129,631.75992 322.60336,922.65597 548.08444,1087.9707 Z"
       id="path14"
       inkscape:path-effect="#path-effect17"
       inkscape:original-d="M 548.08444,1087.9707 316.6912,1015.8051 126.06251,1165.5095 123.19164,923.14104 -78.093216,788.10358 151.52571,710.47723 l 66.2278,-233.16215 144.78318,194.39274 242.21589,-9.06473 -140.138,197.76765 z" />
    <g
       id="g35"
       inkscape:path-effect="#path-effect37"
       transform="matrix(3.3627029,0,0,3.3627029,437.23469,-514.27732)">
      <path
         id="path19"
         d="m 191.25594,159.41665 c -27.32188,-31.40634 -87.36081,14.48396 -83.88541,-1.12198 8.68269,-38.98871 -7.499976,-70.823482 -19.978787,-75.627397 43.355437,20.074847 66.667527,-74.1267991 62.980067,-36.343308 -4.52284,46.34325 92.77479,20.092518 47.43707,47.893207 -28.24344,17.318628 -9.75889,39.501508 -6.55294,65.199478 z"
         inkscape:transform-center-y="-5.746647"
         inkscape:transform-center-x="1.1785038"
         style="fill:none;fill-rule:evenodd;stroke:#000000;stroke-width:0.861994"
         inkscape:original-d="m 191.25594,159.41665 -43.45028,-19.98194 -41.09328,24.46523 5.57709,-47.49843 -35.966346,-31.521863 46.897106,-9.373701 18.86486,-43.946814 23.40692,41.705169 47.62547,4.361234 -32.43084,35.148915 z" />
      <path
         id="path21"
         d="m 177.64881,248.61904 c -33.36196,-21.1679 -64.34137,-20.91198 -92.034471,8.89429 0.692987,-25.48129 -31.219977,-37.98781 -34.819162,-79.70821 -1.304284,-15.11877 39.392387,-14.76233 58.672153,-55.90874 13.61054,-29.047283 20.44465,34.80186 67.58551,33.02847 48.60415,-1.82844 -19.10671,25.33459 -6.44929,69.25025 2.34842,8.14798 4.69684,16.29596 7.04526,24.44394 z"
         inkscape:transform-center-y="-5.0438939"
         inkscape:transform-center-x="2.7598212"
         style="fill:none;fill-rule:evenodd;stroke:#000000;stroke-width:0.861994"
         inkscape:original-d="m 177.64881,248.61904 -49.96309,-19.56673 -44.443449,30.06571 3.169627,-53.56417 -42.327973,-32.97742 51.922024,-13.53775 18.283321,-50.44687 28.91995,45.19738 53.62769,1.79954 -34.04851,41.47126 z" />
      <path
         id="path23"
         d="m 208.64287,308.33928 c -40.46188,-12.29798 -68.63455,59.92162 -74.12526,43.77198 -13.29992,-39.11858 -45.203656,-57.66484 -58.03332,-55.65228 48.04939,-5.97498 19.670068,-99.01403 35.75026,-65.7989 20.72354,42.80639 91.346,-30.49949 67.59032,16.42258 -15.41233,30.44234 11.84075,40.33194 28.818,61.25662 z"
         inkscape:transform-center-y="6.3957149"
         inkscape:transform-center-x="0.69036649"
         style="fill:none;fill-rule:evenodd;stroke:#000000;stroke-width:0.861994"
         inkscape:original-d="m 208.64287,308.33928 -48.74659,5.58415 -23.0351,43.32202 -20.37437,-44.63517 -48.319919,-8.52045 36.154539,-33.17019 -6.828251,-48.58794 42.719101,24.13486 44.09983,-21.50856 -9.75269,48.08636 z" />
      <path
         id="path25"
         d="m 234.34523,208.93155 c -18.46451,121.60561 -106.90001,-94.96182 -19.85317,-41.44448 13.97189,8.59007 19.81414,25.83934 19.85317,41.44448 z"
         style="fill:none;fill-rule:evenodd;stroke:#000000;stroke-width:0.861994"
         inkscape:original-d="m 234.34523,208.93155 a 32.505951,44.979168 0 0 1 -32.50595,44.97917 32.505951,44.979168 0 0 1 -32.50595,-44.97917 32.505951,44.979168 0 0 1 32.50595,-44.97917 32.505951,44.979168 0 0 1 32.50595,44.97917 z" />
    </g>
  </g>
</svg>
)"""";

   testDoc(svg);
}

TEST_F(LPESimplifyTest, multi_MM_1_0_2)
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
   id="svg8"
   inkscape:version="1.0.2 (e86c870879, 2021-01-15)"
   sodipodi:docname="1.svg">
  <defs
     id="defs2">
    <inkscape:path-effect
       effect="simplify"
       id="path-effect37"
       is_visible="true"
       steps="1"
       threshold="0.05"
       smooth_angles="70"
       helper_size="5"
       simplify_individual_paths="false"
       simplify_just_coalesce="false"
       lpeversion="1" />
    <inkscape:path-effect
       effect="simplify"
       id="path-effect17"
       is_visible="true"
       steps="1"
       threshold="0.05"
       smooth_angles="0"
       helper_size="5"
       simplify_individual_paths="false"
       simplify_just_coalesce="false"
       lpeversion="1" />
    <inkscape:path-effect
       effect="simplify"
       id="path-effect12"
       is_visible="true"
       steps="1"
       threshold="0.2"
       smooth_angles="0"
       helper_size="5"
       simplify_individual_paths="false"
       simplify_just_coalesce="false"
       lpeversion="1" />
  </defs>
  <sodipodi:namedview
     id="base"
     pagecolor="#ffffff"
     bordercolor="#666666"
     borderopacity="1.0"
     inkscape:pageopacity="0.0"
     inkscape:pageshadow="2"
     inkscape:zoom="0.24748737"
     inkscape:cx="-332.71058"
     inkscape:cy="723.97691"
     inkscape:document-units="mm"
     inkscape:current-layer="layer1"
     showgrid="false"
     inkscape:window-width="1920"
     inkscape:window-height="1016"
     inkscape:window-x="0"
     inkscape:window-y="27"
     inkscape:window-maximized="1"
     inkscape:document-rotation="0"
     units="mm" />
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
       style="fill:none;fill-rule:evenodd;stroke:#000000;stroke-width:2.89863"
       id="path10"
       sodipodi:sides="5"
       sodipodi:cx="228.92471"
       sodipodi:cy="-145.62271"
       sodipodi:r1="321.31464"
       sodipodi:r2="160.65732"
       sodipodi:arg1="0.64508339"
       sodipodi:arg2="1.2734019"
       inkscape:flatsided="false"
       inkscape:rounded="0"
       inkscape:randomized="0"
       d="m 485.67105,47.572557 c -976.33842,270.368333 3.43346,820.677343 -43.057,-85.34293 0,0 43.057,85.34293 43.057,85.34293 z"
       inkscape:transform-center-x="-1.5951224"
       inkscape:transform-center-y="178.40481"
       inkscape:path-effect="#path-effect12"
       transform="matrix(0.26449453,0,0,0.55672514,-62.931993,-82.13639)" />
    <path
       style="fill:none;fill-rule:evenodd;stroke:#000000;stroke-width:0.766672"
       inkscape:transform-center-x="-9.024773"
       inkscape:transform-center-y="12.747568"
       d="m 151.07926,339.21572 c -21.0985,-4.57186 -43.75843,-17.64222 -63.894085,-16.97355 -13.939867,17.543 -63.810985,38.79138 -47.768954,34.05961 -2.158559,-20.33763 3.284632,-44.78738 -3.561099,-62.5895 -20.992136,-7.83662 -56.611279,-48.70053 -47.154022,-34.90601 18.675206,-8.33757 43.610348,-10.71616 58.425731,-22.72799 0.96604,-22.38615 28.823226,-68.89017 18.626158,-55.63268 13.700471,15.18472 23.668047,38.16441 39.670191,48.54282 21.58893,-5.99897 74.42546,6.12425 58.66562,0.52304 -10.20785,17.72228 -28.98266,34.30314 -33.90818,52.72919 6.96621,18.99169 13.93243,37.98338 20.89864,56.97507 z"
       id="path14"
       inkscape:path-effect="#path-effect17"
       inkscape:original-d="m 151.07926,339.21572 -61.20225,-19.0875 -50.42025,39.596 -0.75933,-64.1051 -53.23874,-35.7167 60.73295,-20.5317 17.51689,-61.67011 38.29436,51.41581 64.06477,-2.3976 -37.0657,52.3085 z" />
    <g
       id="g35"
       inkscape:path-effect="#path-effect37"
       transform="matrix(0.88941651,0,0,0.88941651,86.924193,-123.73781)">
      <path
         id="path19"
         d="m 191.25594,159.41665 c -27.3194,-31.41565 -87.35937,14.47393 -83.88519,-1.12386 8.68331,-38.98493 -7.487318,-70.822268 -19.97529,-75.62626 43.3593,20.08525 66.6708,-74.1473817 62.97914,-36.337608 -4.52511,46.345883 92.79122,20.084036 47.42914,47.893821 -28.24384,17.315237 -9.74878,39.500047 -6.5478,65.193907 z"
         inkscape:transform-center-y="-5.746647"
         inkscape:transform-center-x="1.1785038"
         style="fill:none;fill-rule:evenodd;stroke:#000000;stroke-width:0.861994"
         inkscape:original-d="m 191.25594,159.41665 -43.45028,-19.98194 -41.09328,24.46523 5.57709,-47.49843 -35.966346,-31.521863 46.897106,-9.373701 18.86486,-43.946814 23.40692,41.705169 47.62547,4.361234 -32.43084,35.148915 z" />
      <path
         id="path21"
         d="m 177.64881,248.61904 c -33.3683,-21.17938 -64.35061,-20.90404 -92.048597,8.90385 0.73997,-25.54806 -31.369468,-38.00227 -34.816994,-79.72709 -1.247757,-15.10139 39.389493,-14.77776 58.688631,-55.91189 13.61097,-29.010383 20.44658,34.8093 67.59235,33.04132 48.58482,-1.82194 -19.11816,25.33402 -6.45803,69.25898 2.34755,8.14494 4.69509,16.28989 7.04264,24.43483 z"
         inkscape:transform-center-y="-5.0438939"
         inkscape:transform-center-x="2.7598212"
         style="fill:none;fill-rule:evenodd;stroke:#000000;stroke-width:0.861994"
         inkscape:original-d="m 177.64881,248.61904 -49.96309,-19.56673 -44.443449,30.06571 3.169627,-53.56417 -42.327973,-32.97742 51.922024,-13.53775 18.283321,-50.44687 28.91995,45.19738 53.62769,1.79954 -34.04851,41.47126 z" />
      <path
         id="path23"
         d="m 208.64287,308.33928 c -40.46469,-12.30734 -68.63831,59.91171 -74.12605,43.77026 -13.29784,-39.11383 -45.19052,-57.67164 -58.029737,-55.65312 48.057867,-5.96834 19.661972,-99.033 35.752417,-65.79354 20.7231,42.80955 91.35521,-30.51546 67.58387,16.4272 -15.41415,30.43922 11.84866,40.32582 28.8195,61.2492 z"
         inkscape:transform-center-y="6.3957149"
         inkscape:transform-center-x="0.69036649"
         style="fill:none;fill-rule:evenodd;stroke:#000000;stroke-width:0.861994"
         inkscape:original-d="m 208.64287,308.33928 -48.74659,5.58415 -23.0351,43.32202 -20.37437,-44.63517 -48.319919,-8.52045 36.154539,-33.17019 -6.828251,-48.58794 42.719101,24.13486 44.09983,-21.50856 -9.75269,48.08636 z" />
      <path
         id="path25"
         d="m 234.34523,208.93155 c -18.46451,121.60561 -106.90001,-94.96182 -19.85317,-41.44448 13.97189,8.59007 19.81414,25.83934 19.85317,41.44448 z"
         style="fill:none;fill-rule:evenodd;stroke:#000000;stroke-width:0.861994"
         inkscape:original-d="m 234.34523,208.93155 a 32.505951,44.979168 0 0 1 -32.50595,44.97917 32.505951,44.979168 0 0 1 -32.50595,-44.97917 32.505951,44.979168 0 0 1 32.50595,-44.97917 32.505951,44.979168 0 0 1 32.50595,44.97917 z" />
    </g>
  </g>
</svg>
)"""";

   testDoc(svg);
}