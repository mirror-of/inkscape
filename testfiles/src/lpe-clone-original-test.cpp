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

class LPECloneOriginalTest : public LPESPathsTest {};

// INKSCAPE 0.92.5

TEST_F(LPECloneOriginalTest, mixed_0_92_5)
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
   sodipodi:docname="1.svg">
  <defs
     id="defs2">
    <inkscape:path-effect
       effect="clone_original"
       id="path-effect01"
       is_visible="true"
       linkedpath="#path01" />
  </defs>
  <g
     inkscape:label="Layer 1"
     inkscape:groupmode="layer"
     id="layer1">
    <path
       style="fill:none;stroke:#ff0000;stroke-width:0.26458332"
       d="m 157.23809,221.40477 a 54.42857,53.672619 0 0 1 -54.42857,53.67262 54.42857,53.672619 0 0 1 -54.428573,-53.67262 54.42857,53.672619 0 0 1 54.428573,-53.67262 54.42857,53.672619 0 0 1 54.42857,53.67262 z"
       id="rect02"
       inkscape:path-effect="#path-effect01"
       inkscape:original-d="M 15.119047,15.785713 H 138.33928 V 124.64285 H 15.119047 Z" />
    <path
       style="fill:none;stroke:#ff0000;stroke-width:0.26458332"
       d="m 157.23809,221.40477 a 54.42857,53.672619 0 0 1 -54.42857,53.67262 54.42857,53.672619 0 0 1 -54.428573,-53.67262 54.42857,53.672619 0 0 1 54.428573,-53.67262 54.42857,53.672619 0 0 1 54.42857,53.67262 z"
       id="path01" />
  </g>
</svg>
)"""";
    testDoc(svg);
}


// INKSCAPE 1.0.2

TEST_F(LPECloneOriginalTest, boken_1_02)
{
   std::cout << "linked item is broken in 1.0.2 because group cliboard items, use same version of 1.1 but resaved in 1.2 to get comapat in 1.0.1 or before the group clipboard is added" << std::endl;
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
       effect="clone_original"
       linkeditem="#path02"
       method="bsplinespiro"
       allow_transforms="false"
       id="path-effect191"
       lpeversion="1" />
    <inkscape:path-effect
       effect="spiro"
       id="path-effect189"
       is_visible="true"
       lpeversion="1" />
    <inkscape:path-effect
       effect="clone_original"
       id="path-effect118"
       is_visible="true"
       lpeversion="1"
       linkeditem="#g16"
       method="d"
       attributes=""
       css_properties=""
       allow_transforms="true" />
    <inkscape:path-effect
       effect="clone_original"
       id="path-effect66"
       is_visible="true"
       lpeversion="1"
       linkeditem="#path17"
       method="d"
       attributes=""
       css_properties=""
       allow_transforms="true" />
    <inkscape:path-effect
       effect="clone_original"
       id="path-effect138"
       is_visible="true"
       lpeversion="1"
       linkeditem="#path18"
       method="d"
       attributes=""
       css_properties=""
       allow_transforms="true" />
    <inkscape:path-effect
       effect="bounding_box"
       id="path-effect39"
       is_visible="true"
       linkedpath="m 89.540131,132.33983 c 39.825569,-5.82992 71.760559,-64.263684 122.100169,0 v 86.58012 c -41.85049,31.06173 -82.21035,21.87631 -122.100169,0 z"
       visualbounds="false"
       lpeversion="1" />
  </defs>
  <path
     style="fill:#ffff00;fill-rule:evenodd;stroke-width:0.254511"
     id="ellipse20"
     inkscape:path-effect="#path-effect138"
     sodipodi:type="arc"
     sodipodi:cx="109.05201"
     sodipodi:cy="48.879856"
     sodipodi:rx="20.797256"
     sodipodi:ry="19.170473"
     d="M 109.01812,49.994839 92.182785,41.542495 80.726178,60.22578 83.562431,41.60251 62.253285,36.480081 80.841518,33.422611 79.128348,11.573492 87.780255,28.307142 108.0306,19.926071 94.789544,33.325506 Z"
     transform="translate(97.44188,-0.57297094)" />
  <path
     style="fill:#ffff00;fill-rule:evenodd;stroke-width:0.263701"
     id="ellipse19"
     inkscape:path-effect="#path-effect66"
     sodipodi:type="arc"
     sodipodi:cx="109.24622"
     sodipodi:cy="116.51102"
     sodipodi:rx="21.548172"
     sodipodi:ry="19.86265"
     d="m 119.66307,109.80981 -17.8727,-9.00128 -11.440767,19.60162 3.037758,-19.77949 L 71.209719,95.807081 90.959854,92.58396 88.694082,70.001202 97.862579,87.788697 118.6399,78.655364 104.5562,92.871761 Z"
     transform="translate(74.514283,-1.956031)" />
  <path
     sodipodi:type="star"
     style="fill:#ffff00;fill-rule:evenodd;stroke-width:0.254511"
     id="path18"
     sodipodi:sides="5"
     sodipodi:cx="87.831308"
     sodipodi:cy="35.640054"
     sodipodi:r1="25.591812"
     sodipodi:r2="7.3330889"
     sodipodi:arg1="0.59548852"
     sodipodi:arg2="0.93551599"
     inkscape:flatsided="false"
     inkscape:rounded="0"
     inkscape:randomized="0"
     inkscape:transform-center-x="0.58090876"
     inkscape:transform-center-y="0.068688338"
     d="M 109.01812,49.994839 92.182785,41.542495 80.726178,60.22578 83.562431,41.60251 62.253285,36.480081 80.841518,33.422611 79.128348,11.573492 87.780255,28.307142 108.0306,19.926071 94.789544,33.325506 Z" />
  <path
     sodipodi:type="star"
     style="fill:#ffff00;fill-rule:evenodd;stroke-width:0.263701"
     id="path17"
     sodipodi:sides="5"
     sodipodi:cx="97.711277"
     sodipodi:cy="94.936718"
     sodipodi:r1="26.515842"
     sodipodi:r2="7.1496258"
     sodipodi:arg1="0.59548852"
     sodipodi:arg2="0.96364212"
     inkscape:flatsided="false"
     inkscape:rounded="0"
     inkscape:randomized="0"
     inkscape:transform-center-x="0.6019777"
     inkscape:transform-center-y="0.071179086"
     d="m 119.66307,109.80981 -17.8727,-9.00128 -11.440767,19.60162 3.037758,-19.77949 L 71.209719,95.807081 90.959854,92.58396 88.694082,70.001202 97.862579,87.788697 118.6399,78.655364 104.5562,92.871761 Z" />
  <g
     id="g16"
     transform="matrix(0.77010063,0,0,0.77010063,-7.7475443,6.0030225)">
    <g
       id="g15"
       transform="translate(-2.1023541,-5.8034479)">
      <ellipse
         style="fill:#ff0000;fill-rule:evenodd;stroke-width:0.342424"
         id="ellipse14"
         cx="59.413036"
         cy="217.29919"
         rx="27.980984"
         ry="7.9669428" />
      <path
         sodipodi:type="star"
         style="fill:#ffff00;fill-rule:evenodd;stroke-width:0.342424"
         id="path13"
         sodipodi:sides="5"
         sodipodi:cx="139.37627"
         sodipodi:cy="215.95181"
         sodipodi:r1="34.43166"
         sodipodi:r2="17.21583"
         sodipodi:arg1="0.59548852"
         sodipodi:arg2="1.2238071"
         inkscape:flatsided="false"
         inkscape:rounded="0"
         inkscape:randomized="0"
         inkscape:transform-center-x="2.9540026"
         inkscape:transform-center-y="0.34924413"
         d="m 167.88136,235.26499 -22.65054,-3.1234 -15.41391,16.88828 -4.02888,-22.50713 -20.82487,-9.44074 20.16056,-10.78678 2.54344,-22.72297 16.48879,15.84053 22.3968,-4.60283 -9.96993,20.57677 z" />
    </g>
    <g
       id="g12"
       transform="translate(-3.9814864,37.147042)">
      <ellipse
         style="fill:#ff0000;fill-rule:evenodd;stroke-width:0.342424"
         id="path11"
         cx="59.413036"
         cy="217.29919"
         rx="27.980984"
         ry="25.792278" />
      <path
         sodipodi:type="star"
         style="fill:#ffff00;fill-rule:evenodd;stroke-width:0.342424"
         id="path10"
         sodipodi:sides="5"
         sodipodi:cx="139.37627"
         sodipodi:cy="215.95181"
         sodipodi:r1="34.43166"
         sodipodi:r2="17.21583"
         sodipodi:arg1="0.59548852"
         sodipodi:arg2="1.2238071"
         inkscape:flatsided="false"
         inkscape:rounded="0"
         inkscape:randomized="0"
         inkscape:transform-center-x="2.9540026"
         inkscape:transform-center-y="0.34924413"
         d="m 167.88136,235.26499 -22.65054,-3.1234 -15.41391,16.88828 -4.02888,-22.50713 -20.82487,-9.44074 20.16056,-10.78678 2.54344,-22.72297 16.48879,15.84053 22.3968,-4.60283 -9.96993,20.57677 z" />
    </g>
  </g>
  <g
     id="g09"
     transform="matrix(0.77010063,0,0,0.77010063,128.95967,4.1869072)"
     inkscape:path-effect="#path-effect118">
    <g
       id="g08"
       transform="translate(-2.1023541,-5.8034479)">
      <ellipse
         style="fill:#ff0000;fill-rule:evenodd;stroke-width:0.342424"
         id="ellipse07"
         cx="59.413036"
         cy="217.29919"
         rx="27.980984"
         ry="25.792278"
         d="m 87.39402,217.29919 a 27.980984,7.9669428 0 0 1 -27.980984,7.96695 27.980984,7.9669428 0 0 1 -27.980983,-7.96695 27.980984,7.9669428 0 0 1 27.980983,-7.96694 27.980984,7.9669428 0 0 1 27.980984,7.96694 z" />
      <path
         sodipodi:type="star"
         style="fill:#ffff00;fill-rule:evenodd;stroke-width:0.342424"
         id="path06"
         sodipodi:sides="5"
         sodipodi:cx="139.37627"
         sodipodi:cy="215.95181"
         sodipodi:r1="34.43166"
         sodipodi:r2="17.21583"
         sodipodi:arg1="0.59548852"
         sodipodi:arg2="1.2238071"
         inkscape:flatsided="false"
         inkscape:rounded="0"
         inkscape:randomized="0"
         inkscape:transform-center-x="2.9540026"
         inkscape:transform-center-y="0.34924413"
         d="m 167.88136,235.26499 -22.65054,-3.1234 -15.41391,16.88828 -4.02888,-22.50713 -20.82487,-9.44074 20.16056,-10.78678 2.54344,-22.72297 16.48879,15.84053 22.3968,-4.60283 -9.96993,20.57677 z" />
    </g>
    <g
       id="g05"
       transform="translate(-3.9814864,37.147042)">
      <ellipse
         style="fill:#ff0000;fill-rule:evenodd;stroke-width:0.342424"
         id="ellipse04"
         cx="59.413036"
         cy="217.29919"
         rx="27.980984"
         ry="25.792278"
         d="M 87.39402,217.29919 A 27.980984,25.792278 0 0 1 59.413036,243.09147 27.980984,25.792278 0 0 1 31.432053,217.29919 27.980984,25.792278 0 0 1 59.413036,191.50692 27.980984,25.792278 0 0 1 87.39402,217.29919 Z" />
      <path
         sodipodi:type="star"
         style="fill:#ffff00;fill-rule:evenodd;stroke-width:0.342424"
         id="path03"
         sodipodi:sides="5"
         sodipodi:cx="139.37627"
         sodipodi:cy="215.95181"
         sodipodi:r1="34.43166"
         sodipodi:r2="17.21583"
         sodipodi:arg1="0.59548852"
         sodipodi:arg2="1.2238071"
         inkscape:flatsided="false"
         inkscape:rounded="0"
         inkscape:randomized="0"
         inkscape:transform-center-x="2.9540026"
         inkscape:transform-center-y="0.34924413"
         d="m 167.88136,235.26499 -22.65054,-3.1234 -15.41391,16.88828 -4.02888,-22.50713 -20.82487,-9.44074 20.16056,-10.78678 2.54344,-22.72297 16.48879,15.84053 22.3968,-4.60283 -9.96993,20.57677 z" />
    </g>
  </g>
  <path
     style="fill:none;stroke:#000000;stroke-width:1px;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1"
     d="M 18.460949,62.250537 C 18.405129,61.4448 18.303581,60.642437 18.164509,59.846832 17.056101,53.505845 13.524379,47.697511 8.7121966,43.422047 3.9000141,39.146584 -2.1364469,36.360589 -8.4245037,34.983262 c -12.5761133,-2.754654 -25.7185673,0.01323 -37.9220783,4.114587 -10.090816,3.391318 -19.995295,7.769961 -28.315635,14.410558 -8.320341,6.640598 -14.992021,15.740606 -17.219594,26.150386 -1.113787,5.20489 -1.094133,10.674018 0.267925,15.81952 1.362058,5.145507 4.086302,9.952917 7.951056,13.612837 3.864754,3.65993 8.877144,6.137 14.161534,6.77467 5.284391,0.63767 10.811168,-0.60724 15.197394,-3.6226 4.109183,-2.82491 7.084699,-7.0419 9.302359,-11.50816 2.21766,-4.466259 3.753059,-9.232917 5.557539,-13.881502 1.80448,-4.648584 3.922243,-9.253465 7.122195,-13.077833 3.199952,-3.824368 7.611526,-6.832441 12.555635,-7.481496 3.611781,-0.474149 7.338935,0.340015 10.5613493,2.038787 3.2224143,1.698773 5.9531062,4.252205 8.0980519,7.19652 4.2898913,5.888629 6.2079592,13.147643 7.6297884,20.293098 1.4266854,7.169856 2.4458581,14.480346 2.0083538,21.777666 -0.4375043,7.29732 -2.4025816,14.62346 -6.5314675,20.65626 -4.1288859,6.03279 -10.5683292,10.65781 -17.8006479,11.72359 -3.61616,0.53289 -7.37626,0.17098 -10.781011,-1.15878 -3.404751,-1.32975 -6.439357,-3.6381 -8.529723,-6.63659 -2.525603,-3.62281 -3.606078,-8.15842 -3.338928,-12.5666 0.267151,-4.40818 1.831556,-8.68518 4.169543,-12.43181 4.675973,-7.49327 12.192398,-12.72367 19.794982,-17.219718 7.6025844,-4.496046 15.5950248,-8.541055 22.099192,-14.5168 3.252084,-2.987872 6.105959,-6.455343 8.087325,-10.402192 1.981367,-3.946849 3.065535,-8.391414 2.760315,-12.797123 z"
     id="path02"
     inkscape:path-effect="#path-effect189"
     inkscape:original-d="m 18.460949,62.250537 c 17.856477,-26.645749 -0.196626,-1.60147 -0.29644,-2.403705 -0.09981,-0.802235 -43.006394,-13.831655 -64.511091,-20.748983 -21.504697,-6.917328 -7.51166,50.480965 -7.95732,73.145371 -0.44566,22.6644 25.233933,-32.348046 34.537728,-45.948991 C -10.462378,52.693284 -2.2390476,85.980834 6.5230156,95.822634 15.285079,105.66443 -21.232315,126.73173 -35.11148,142.18478 -48.990646,157.63783 0.60447241,88.896284 18.460949,62.250537 Z"
     sodipodi:nodetypes="ssssssss" />
  <path
     d="M 18.460949,62.250537 C 18.405129,61.4448 18.303581,60.642437 18.164509,59.846832 17.056101,53.505845 13.524379,47.697511 8.7121966,43.422047 3.9000141,39.146584 -2.1364469,36.360589 -8.4245037,34.983262 c -12.5761133,-2.754654 -25.7185673,0.01323 -37.9220783,4.114587 -10.090816,3.391318 -19.995295,7.769961 -28.315635,14.410558 -8.320341,6.640598 -14.992021,15.740606 -17.219594,26.150386 -1.113787,5.20489 -1.094133,10.674018 0.267925,15.81952 1.362058,5.145507 4.086302,9.952917 7.951056,13.612837 3.864754,3.65993 8.877144,6.137 14.161534,6.77467 5.284391,0.63767 10.811168,-0.60724 15.197394,-3.6226 4.109183,-2.82491 7.084699,-7.0419 9.302359,-11.50816 2.21766,-4.466259 3.753059,-9.232917 5.557539,-13.881502 1.80448,-4.648584 3.922243,-9.253465 7.122195,-13.077833 3.199952,-3.824368 7.611526,-6.832441 12.555635,-7.481496 3.611781,-0.474149 7.338935,0.340015 10.5613493,2.038787 3.2224143,1.698773 5.9531062,4.252205 8.0980519,7.19652 4.2898913,5.888629 6.2079592,13.147643 7.6297884,20.293098 1.4266854,7.169856 2.4458581,14.480346 2.0083538,21.777666 -0.4375043,7.29732 -2.4025816,14.62346 -6.5314675,20.65626 -4.1288859,6.03279 -10.5683292,10.65781 -17.8006479,11.72359 -3.61616,0.53289 -7.37626,0.17098 -10.781011,-1.15878 -3.404751,-1.32975 -6.439357,-3.6381 -8.529723,-6.63659 -2.525603,-3.62281 -3.606078,-8.15842 -3.338928,-12.5666 0.267151,-4.40818 1.831556,-8.68518 4.169543,-12.43181 4.675973,-7.49327 12.192398,-12.72367 19.794982,-17.219718 7.6025844,-4.496046 15.5950248,-8.541055 22.099192,-14.5168 3.252084,-2.987872 6.105959,-6.455343 8.087325,-10.402192 1.981367,-3.946849 3.065535,-8.391414 2.760315,-12.797123 z"
     id="path01"
     inkscape:path-effect="#path-effect191"
     inkscape:original-d="M 0,0"
     style="fill:#00ffff" />
</svg>
)"""";

   testDoc(svg);
}

// INKSCAPE 1.1

TEST_F(LPECloneOriginalTest, mixed_PX_1_1)
{
   std::string svg = R""""(<?xml version="1.0" encoding="UTF-8" standalone="no"?>
<svg
   width="250"
   height="250"
   viewBox="0 0 250 250"
   version="1.1"
   id="svg8"
   inkscape:version="1.1-alpha (82a87c3b8f, 2021-03-10)"
   xmlns:inkscape="http://www.inkscape.org/namespaces/inkscape"
   xmlns:sodipodi="http://sodipodi.sourceforge.net/DTD/sodipodi-0.dtd"
   xmlns="http://www.w3.org/2000/svg"
   xmlns:svg="http://www.w3.org/2000/svg">
  <defs
     id="defs2">
    <inkscape:path-effect
       effect="clone_original"
       linkeditem="#path02"
       method="bsplinespiro"
       allow_transforms="false"
       id="path-effect191"
       lpeversion="1" />
    <inkscape:path-effect
       effect="spiro"
       id="path-effect189"
       is_visible="true"
       lpeversion="1" />
    <inkscape:path-effect
       effect="clone_original"
       id="path-effect118"
       is_visible="true"
       lpeversion="1"
       linkeditem="#g16"
       method="d"
       attributes=""
       css_properties=""
       allow_transforms="true" />
    <inkscape:path-effect
       effect="clone_original"
       id="path-effect66"
       is_visible="true"
       lpeversion="1"
       linkeditem="#path17"
       method="d"
       attributes=""
       css_properties=""
       allow_transforms="true" />
    <inkscape:path-effect
       effect="clone_original"
       id="path-effect138"
       is_visible="true"
       lpeversion="1"
       linkeditem="#path18"
       method="d"
       attributes=""
       css_properties=""
       allow_transforms="true" />
    <inkscape:path-effect
       effect="bounding_box"
       id="path-effect39"
       is_visible="true"
       linkedpath="m 89.540131,132.33983 c 39.825569,-5.82992 71.760559,-64.263684 122.100169,0 v 86.58012 c -41.85049,31.06173 -82.21035,21.87631 -122.100169,0 z"
       visualbounds="false"
       lpeversion="1" />
  </defs>
  <path
     style="fill:#ffff00;fill-rule:evenodd;stroke-width:0.254511"
     id="ellipse20"
     inkscape:path-effect="#path-effect138"
     sodipodi:type="arc"
     sodipodi:cx="109.05201"
     sodipodi:cy="48.879856"
     sodipodi:rx="20.797256"
     sodipodi:ry="19.170473"
     d="M 92.230787,62.806149 75.395454,60.484643 63.938847,73.03709 60.944326,56.308353 45.465954,49.291391 60.450571,41.273969 62.341018,24.384801 74.596541,36.158499 91.243274,32.737381 83.832987,48.031349 Z"
     transform="translate(97.44188,-0.57297094)" />
  <path
     style="fill:#ffff00;fill-rule:evenodd;stroke-width:0.263701"
     id="ellipse19"
     inkscape:path-effect="#path-effect66"
     sodipodi:type="arc"
     sodipodi:cx="109.24622"
     sodipodi:cy="116.51102"
     sodipodi:rx="21.548172"
     sodipodi:ry="19.86265"
     d="m 118.26345,132.30255 -17.44319,-2.40533 -11.870269,13.00567 -3.102642,-17.33275 -16.037242,-7.27032 15.525658,-8.30691 1.958705,-17.498971 12.698026,12.198801 17.247784,-3.54464 -7.67784,15.84618 z"
     transform="translate(74.514283,-1.956031)" />
  <path
     sodipodi:type="star"
     style="fill:#ffff00;fill-rule:evenodd;stroke-width:0.254511"
     id="path18"
     sodipodi:sides="5"
     sodipodi:cx="71.043976"
     sodipodi:cy="48.451362"
     sodipodi:r1="25.591812"
     sodipodi:r2="12.795906"
     sodipodi:arg1="0.59548852"
     sodipodi:arg2="1.2238071"
     inkscape:flatsided="false"
     inkscape:rounded="0"
     inkscape:randomized="0"
     inkscape:transform-center-x="0.58090876"
     inkscape:transform-center-y="0.068688338"
     d="M 92.230787,62.806149 75.395454,60.484643 63.938847,73.03709 60.944326,56.308353 45.465954,49.291391 60.450571,41.273969 62.341018,24.384801 74.596541,36.158499 91.243274,32.737381 83.832987,48.031349 Z" />
  <path
     sodipodi:type="star"
     style="fill:#ffff00;fill-rule:evenodd;stroke-width:0.263701"
     id="path17"
     sodipodi:sides="5"
     sodipodi:cx="96.31166"
     sodipodi:cy="117.42946"
     sodipodi:r1="26.515842"
     sodipodi:r2="13.257921"
     sodipodi:arg1="0.59548852"
     sodipodi:arg2="1.2238071"
     inkscape:flatsided="false"
     inkscape:rounded="0"
     inkscape:randomized="0"
     inkscape:transform-center-x="0.6019777"
     inkscape:transform-center-y="0.071179086"
     d="m 118.26345,132.30255 -17.44319,-2.40533 -11.870269,13.00567 -3.102642,-17.33275 -16.037242,-7.27032 15.525658,-8.30691 1.958705,-17.498971 12.698026,12.198801 17.247784,-3.54464 -7.67784,15.84618 z" />
  <g
     id="g16"
     transform="matrix(0.77010063,0,0,0.77010063,-1.3916237,13.9502)">
    <g
       id="g15"
       transform="translate(-2.1023541,-5.8034479)">
      <ellipse
         style="fill:#ff0000;fill-rule:evenodd;stroke-width:0.342424"
         id="ellipse14"
         cx="59.413036"
         cy="217.29919"
         rx="27.980984"
         ry="25.792278" />
      <path
         sodipodi:type="star"
         style="fill:#ffff00;fill-rule:evenodd;stroke-width:0.342424"
         id="path13"
         sodipodi:sides="5"
         sodipodi:cx="139.37627"
         sodipodi:cy="215.95181"
         sodipodi:r1="34.43166"
         sodipodi:r2="17.21583"
         sodipodi:arg1="0.59548852"
         sodipodi:arg2="1.2238071"
         inkscape:flatsided="false"
         inkscape:rounded="0"
         inkscape:randomized="0"
         inkscape:transform-center-x="2.9540026"
         inkscape:transform-center-y="0.34924413"
         d="m 167.88136,235.26499 -22.65054,-3.1234 -15.41391,16.88828 -4.02888,-22.50713 -20.82487,-9.44074 20.16056,-10.78678 2.54344,-22.72297 16.48879,15.84053 22.3968,-4.60283 -9.96993,20.57677 z" />
    </g>
    <g
       id="g12"
       transform="translate(-3.9814864,37.147042)">
      <ellipse
         style="fill:#ff0000;fill-rule:evenodd;stroke-width:0.342424"
         id="path11"
         cx="59.413036"
         cy="217.29919"
         rx="27.980984"
         ry="25.792278" />
      <path
         sodipodi:type="star"
         style="fill:#ffff00;fill-rule:evenodd;stroke-width:0.342424"
         id="path10"
         sodipodi:sides="5"
         sodipodi:cx="139.37627"
         sodipodi:cy="215.95181"
         sodipodi:r1="34.43166"
         sodipodi:r2="17.21583"
         sodipodi:arg1="0.59548852"
         sodipodi:arg2="1.2238071"
         inkscape:flatsided="false"
         inkscape:rounded="0"
         inkscape:randomized="0"
         inkscape:transform-center-x="2.9540026"
         inkscape:transform-center-y="0.34924413"
         d="m 167.88136,235.26499 -22.65054,-3.1234 -15.41391,16.88828 -4.02888,-22.50713 -20.82487,-9.44074 20.16056,-10.78678 2.54344,-22.72297 16.48879,15.84053 22.3968,-4.60283 -9.96993,20.57677 z" />
    </g>
  </g>
  <g
     id="g09"
     transform="matrix(0.77010063,0,0,0.77010063,123.4285,13.600369)"
     inkscape:path-effect="#path-effect118">
    <g
       id="g08"
       transform="translate(-2.1023541,-5.8034479)">
      <ellipse
         style="fill:#ff0000;fill-rule:evenodd;stroke-width:0.342424"
         id="ellipse07"
         cx="59.413036"
         cy="217.29919"
         rx="27.980984"
         ry="25.792278"
         d="M 87.39402,217.29919 A 27.980984,25.792278 0 0 1 59.413036,243.09147 27.980984,25.792278 0 0 1 31.432053,217.29919 27.980984,25.792278 0 0 1 59.413036,191.50692 27.980984,25.792278 0 0 1 87.39402,217.29919 Z" />
      <path
         sodipodi:type="star"
         style="fill:#ffff00;fill-rule:evenodd;stroke-width:0.342424"
         id="path06"
         sodipodi:sides="5"
         sodipodi:cx="139.37627"
         sodipodi:cy="215.95181"
         sodipodi:r1="34.43166"
         sodipodi:r2="17.21583"
         sodipodi:arg1="0.59548852"
         sodipodi:arg2="1.2238071"
         inkscape:flatsided="false"
         inkscape:rounded="0"
         inkscape:randomized="0"
         inkscape:transform-center-x="2.9540026"
         inkscape:transform-center-y="0.34924413"
         d="m 167.88136,235.26499 -22.65054,-3.1234 -15.41391,16.88828 -4.02888,-22.50713 -20.82487,-9.44074 20.16056,-10.78678 2.54344,-22.72297 16.48879,15.84053 22.3968,-4.60283 -9.96993,20.57677 z" />
    </g>
    <g
       id="g05"
       transform="translate(-3.9814864,37.147042)">
      <ellipse
         style="fill:#ff0000;fill-rule:evenodd;stroke-width:0.342424"
         id="ellipse04"
         cx="59.413036"
         cy="217.29919"
         rx="27.980984"
         ry="25.792278"
         d="M 87.39402,217.29919 A 27.980984,25.792278 0 0 1 59.413036,243.09147 27.980984,25.792278 0 0 1 31.432053,217.29919 27.980984,25.792278 0 0 1 59.413036,191.50692 27.980984,25.792278 0 0 1 87.39402,217.29919 Z" />
      <path
         sodipodi:type="star"
         style="fill:#ffff00;fill-rule:evenodd;stroke-width:0.342424"
         id="path03"
         sodipodi:sides="5"
         sodipodi:cx="139.37627"
         sodipodi:cy="215.95181"
         sodipodi:r1="34.43166"
         sodipodi:r2="17.21583"
         sodipodi:arg1="0.59548852"
         sodipodi:arg2="1.2238071"
         inkscape:flatsided="false"
         inkscape:rounded="0"
         inkscape:randomized="0"
         inkscape:transform-center-x="2.9540026"
         inkscape:transform-center-y="0.34924413"
         d="m 167.88136,235.26499 -22.65054,-3.1234 -15.41391,16.88828 -4.02888,-22.50713 -20.82487,-9.44074 20.16056,-10.78678 2.54344,-22.72297 16.48879,15.84053 22.3968,-4.60283 -9.96993,20.57677 z" />
    </g>
  </g>
  <path
     style="fill:none;stroke:#000000;stroke-width:1px;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1"
     d="m 20.589764,94.953503 c -0.05582,-0.805737 -0.157368,-1.6081 -0.29644,-2.403705 C 19.184916,86.208811 15.653194,80.400477 10.841012,76.125014 6.0288291,71.84955 -0.00763193,69.063555 -6.2956887,67.686228 c -12.5761133,-2.754654 -25.7185673,0.01324 -37.9220783,4.114587 -10.090817,3.391318 -19.995297,7.769961 -28.315638,14.410559 -8.320341,6.640599 -14.992022,15.740606 -17.219595,26.150386 -1.113787,5.20489 -1.094133,10.67402 0.267926,15.81952 1.362058,5.14551 4.086302,9.95292 7.951057,13.61285 3.864754,3.65992 8.877144,6.13699 14.161535,6.77466 5.284391,0.63767 10.811169,-0.60723 15.197395,-3.6226 4.109183,-2.82491 7.084699,-7.0419 9.302359,-11.50816 2.21766,-4.46626 3.753059,-9.23292 5.557538,-13.8815 1.80448,-4.64859 3.922243,-9.25347 7.122195,-13.07784 3.199952,-3.82437 7.611527,-6.83244 12.555636,-7.481495 3.611781,-0.474149 7.338935,0.340015 10.5613493,2.038785 3.2224143,1.69877 5.9531062,4.25221 8.0980519,7.19652 4.2898913,5.88863 6.2079593,13.14765 7.6297887,20.2931 1.4266861,7.16986 2.4458591,14.48034 2.0083551,21.77767 -0.437505,7.29732 -2.4025816,14.62346 -6.5314674,20.65625 -4.12888590446,6.0328 -10.5683293,10.65782 -17.8006486,11.7236 -3.61616,0.53289 -7.376261,0.17098 -10.781012,-1.15878 -3.404751,-1.32975 -6.439357,-3.6381 -8.529723,-6.63659 -2.525603,-3.62281 -3.606079,-8.15842 -3.338928,-12.5666 0.267151,-4.40818 1.831555,-8.68518 4.169542,-12.43181 4.675973,-7.49327 12.192398,-12.72367 19.794983,-17.21972 7.602584,-4.49605 15.5950245,-8.54106 22.0991919,-14.5168 3.2520841,-2.98787 6.1059591,-6.45534 8.0873251,-10.40219 1.981367,-3.94685 3.065535,-8.391418 2.760315,-12.797127 z"
     id="path02"
     inkscape:path-effect="#path-effect189"
     inkscape:original-d="m 20.589764,94.953503 c 17.856477,-26.645749 -0.196626,-1.60147 -0.29644,-2.403705 -0.09981,-0.802235 -43.006394,-13.831655 -64.511091,-20.748983 -21.504697,-6.917328 -7.51166,50.480965 -7.95732,73.145375 -0.44566,22.6644 25.233933,-32.34805 34.537728,-45.948995 C -8.333563,85.39625 -0.11023229,118.6838 8.6518309,128.5256 17.413894,138.3674 -19.1035,159.4347 -32.982665,174.88775 -46.861831,190.3408 2.7332877,121.59925 20.589764,94.953503 Z"
     sodipodi:nodetypes="ssssssss" />
  <path
     d="m 20.589764,94.953503 c -0.05582,-0.805737 -0.157368,-1.6081 -0.29644,-2.403705 C 19.184916,86.208811 15.653194,80.400477 10.841012,76.125014 6.0288291,71.84955 -0.00763193,69.063555 -6.2956887,67.686228 c -12.5761133,-2.754654 -25.7185673,0.01324 -37.9220783,4.114587 -10.090817,3.391318 -19.995297,7.769961 -28.315638,14.410559 -8.320341,6.640599 -14.992022,15.740606 -17.219595,26.150386 -1.113787,5.20489 -1.094133,10.67402 0.267926,15.81952 1.362058,5.14551 4.086302,9.95292 7.951057,13.61285 3.864754,3.65992 8.877144,6.13699 14.161535,6.77466 5.284391,0.63767 10.811169,-0.60723 15.197395,-3.6226 4.109183,-2.82491 7.084699,-7.0419 9.302359,-11.50816 2.21766,-4.46626 3.753059,-9.23292 5.557538,-13.8815 1.80448,-4.64859 3.922243,-9.25347 7.122195,-13.07784 3.199952,-3.82437 7.611527,-6.83244 12.555636,-7.481495 3.611781,-0.474149 7.338935,0.340015 10.5613493,2.038785 3.2224143,1.69877 5.9531062,4.25221 8.0980519,7.19652 4.2898913,5.88863 6.2079593,13.14765 7.6297887,20.2931 1.4266861,7.16986 2.4458591,14.48034 2.0083551,21.77767 -0.437505,7.29732 -2.4025816,14.62346 -6.5314674,20.65625 -4.12888590446,6.0328 -10.5683293,10.65782 -17.8006486,11.7236 -3.61616,0.53289 -7.376261,0.17098 -10.781012,-1.15878 -3.404751,-1.32975 -6.439357,-3.6381 -8.529723,-6.63659 -2.525603,-3.62281 -3.606079,-8.15842 -3.338928,-12.5666 0.267151,-4.40818 1.831555,-8.68518 4.169542,-12.43181 4.675973,-7.49327 12.192398,-12.72367 19.794983,-17.21972 7.602584,-4.49605 15.5950245,-8.54106 22.0991919,-14.5168 3.2520841,-2.98787 6.1059591,-6.45534 8.0873251,-10.40219 1.981367,-3.94685 3.065535,-8.391418 2.760315,-12.797127 z"
     id="path01"
     inkscape:path-effect="#path-effect191"
     inkscape:original-d="M 0 0"
     style="fill:#00ffff" />
</svg>
)"""";

   testDoc(svg);
}

TEST_F(LPECloneOriginalTest, mixed_MM_1_1)
{
   std::string svg = R""""(<?xml version="1.0" encoding="UTF-8" standalone="no"?>
<svg
   width="250mm"
   height="250mm"
   viewBox="0 0 250 250"
   version="1.1"
   id="svg8"
   inkscape:version="1.1-alpha (82a87c3b8f, 2021-03-10)"
   xmlns:inkscape="http://www.inkscape.org/namespaces/inkscape"
   xmlns:sodipodi="http://sodipodi.sourceforge.net/DTD/sodipodi-0.dtd"
   xmlns="http://www.w3.org/2000/svg"
   xmlns:svg="http://www.w3.org/2000/svg">
  <defs
     id="defs2">
    <inkscape:path-effect
       effect="clone_original"
       linkeditem="#path02"
       method="bsplinespiro"
       allow_transforms="false"
       id="path-effect191"
       lpeversion="1"
       is_visible="true"
       attributes=""
       css_properties="" />
    <inkscape:path-effect
       effect="spiro"
       id="path-effect189"
       is_visible="true"
       lpeversion="1" />
    <inkscape:path-effect
       effect="clone_original"
       id="path-effect118"
       is_visible="true"
       lpeversion="1"
       linkeditem="#g16"
       method="d"
       attributes=""
       css_properties=""
       allow_transforms="true" />
    <inkscape:path-effect
       effect="clone_original"
       id="path-effect66"
       is_visible="true"
       lpeversion="1"
       linkeditem="#path17"
       method="d"
       attributes=""
       css_properties=""
       allow_transforms="true" />
    <inkscape:path-effect
       effect="clone_original"
       id="path-effect138"
       is_visible="true"
       lpeversion="1"
       linkeditem="#path18"
       method="d"
       attributes=""
       css_properties=""
       allow_transforms="true" />
    <inkscape:path-effect
       effect="bounding_box"
       id="path-effect39"
       is_visible="true"
       linkedpath="m 89.540131,132.33983 c 39.825569,-5.82992 71.760559,-64.263684 122.100169,0 v 86.58012 c -41.85049,31.06173 -82.21035,21.87631 -122.100169,0 z"
       visualbounds="false"
       lpeversion="1" />
  </defs>
  <path
     style="fill:#ffff00;fill-rule:evenodd;stroke-width:0.254511"
     id="ellipse20"
     inkscape:path-effect="#path-effect138"
     sodipodi:type="arc"
     sodipodi:cx="109.05201"
     sodipodi:cy="48.879856"
     sodipodi:rx="20.797256"
     sodipodi:ry="19.170473"
     d="M 92.230787,62.806149 75.395454,60.484643 63.938847,73.03709 60.944326,56.308353 45.465954,49.291391 60.450571,41.273969 62.341018,24.384801 74.596541,36.158499 91.243274,32.737381 83.832987,48.031349 Z"
     transform="translate(97.44188,-0.57297094)" />
  <path
     style="fill:#ffff00;fill-rule:evenodd;stroke-width:0.263701"
     id="ellipse19"
     inkscape:path-effect="#path-effect66"
     sodipodi:type="arc"
     sodipodi:cx="109.24622"
     sodipodi:cy="116.51102"
     sodipodi:rx="21.548172"
     sodipodi:ry="19.86265"
     d="m 118.26345,132.30255 -17.44319,-2.40533 -11.870269,13.00567 -3.102642,-17.33275 -16.037242,-7.27032 15.525658,-8.30691 1.958705,-17.498971 12.698026,12.198801 17.247784,-3.54464 -7.67784,15.84618 z"
     transform="translate(74.514283,-1.956031)" />
  <path
     sodipodi:type="star"
     style="fill:#ffff00;fill-rule:evenodd;stroke-width:0.254511"
     id="path18"
     sodipodi:sides="5"
     sodipodi:cx="71.043976"
     sodipodi:cy="48.451363"
     sodipodi:r1="25.591812"
     sodipodi:r2="12.795906"
     sodipodi:arg1="0.59548852"
     sodipodi:arg2="1.2238071"
     inkscape:flatsided="false"
     inkscape:rounded="0"
     inkscape:randomized="0"
     inkscape:transform-center-x="0.58090876"
     inkscape:transform-center-y="0.068688338"
     d="M 92.230787,62.806149 75.395454,60.484643 63.938847,73.03709 60.944326,56.308353 45.465954,49.291391 60.450571,41.273969 62.341018,24.384801 74.596541,36.158499 91.243274,32.737381 83.832987,48.031349 Z" />
  <path
     sodipodi:type="star"
     style="fill:#ffff00;fill-rule:evenodd;stroke-width:0.263701"
     id="path17"
     sodipodi:sides="5"
     sodipodi:cx="96.311661"
     sodipodi:cy="117.42946"
     sodipodi:r1="26.515842"
     sodipodi:r2="13.257921"
     sodipodi:arg1="0.59548852"
     sodipodi:arg2="1.2238071"
     inkscape:flatsided="false"
     inkscape:rounded="0"
     inkscape:randomized="0"
     inkscape:transform-center-x="0.6019777"
     inkscape:transform-center-y="0.071179086"
     d="m 118.26345,132.30255 -17.44319,-2.40533 -11.870269,13.00567 -3.102642,-17.33275 -16.037242,-7.27032 15.525658,-8.30691 1.958705,-17.498971 12.698026,12.198801 17.247784,-3.54464 -7.67784,15.84618 z" />
  <g
     id="g16"
     transform="matrix(0.77010063,0,0,0.77010063,-1.3916237,13.9502)">
    <g
       id="g15"
       transform="translate(-2.1023541,-5.8034479)">
      <ellipse
         style="fill:#ff0000;fill-rule:evenodd;stroke-width:0.342424"
         id="ellipse14"
         cx="59.413036"
         cy="217.29919"
         rx="27.980984"
         ry="25.792278" />
      <path
         sodipodi:type="star"
         style="fill:#ffff00;fill-rule:evenodd;stroke-width:0.342424"
         id="path13"
         sodipodi:sides="5"
         sodipodi:cx="139.37627"
         sodipodi:cy="215.95181"
         sodipodi:r1="34.43166"
         sodipodi:r2="17.21583"
         sodipodi:arg1="0.59548852"
         sodipodi:arg2="1.2238071"
         inkscape:flatsided="false"
         inkscape:rounded="0"
         inkscape:randomized="0"
         inkscape:transform-center-x="2.9540026"
         inkscape:transform-center-y="0.34924413"
         d="m 167.88136,235.26499 -22.65054,-3.1234 -15.41391,16.88828 -4.02888,-22.50713 -20.82487,-9.44074 20.16056,-10.78678 2.54344,-22.72297 16.48879,15.84053 22.3968,-4.60283 -9.96993,20.57677 z" />
    </g>
    <g
       id="g12"
       transform="translate(-3.9814864,37.147042)">
      <ellipse
         style="fill:#ff0000;fill-rule:evenodd;stroke-width:0.342424"
         id="path11"
         cx="59.413036"
         cy="217.29919"
         rx="27.980984"
         ry="25.792278" />
      <path
         sodipodi:type="star"
         style="fill:#ffff00;fill-rule:evenodd;stroke-width:0.342424"
         id="path10"
         sodipodi:sides="5"
         sodipodi:cx="139.37627"
         sodipodi:cy="215.95181"
         sodipodi:r1="34.43166"
         sodipodi:r2="17.21583"
         sodipodi:arg1="0.59548852"
         sodipodi:arg2="1.2238071"
         inkscape:flatsided="false"
         inkscape:rounded="0"
         inkscape:randomized="0"
         inkscape:transform-center-x="2.9540026"
         inkscape:transform-center-y="0.34924413"
         d="m 167.88136,235.26499 -22.65054,-3.1234 -15.41391,16.88828 -4.02888,-22.50713 -20.82487,-9.44074 20.16056,-10.78678 2.54344,-22.72297 16.48879,15.84053 22.3968,-4.60283 -9.96993,20.57677 z" />
    </g>
  </g>
  <g
     id="g09"
     transform="matrix(0.77010063,0,0,0.77010063,123.4285,13.600369)"
     inkscape:path-effect="#path-effect118">
    <g
       id="g08"
       transform="translate(-2.1023541,-5.8034479)">
      <ellipse
         style="fill:#ff0000;fill-rule:evenodd;stroke-width:0.342424"
         id="ellipse07"
         cx="59.413036"
         cy="217.29919"
         rx="27.980984"
         ry="25.792278"
         d="M 87.39402,217.29919 A 27.980984,25.792278 0 0 1 59.413036,243.09147 27.980984,25.792278 0 0 1 31.432053,217.29919 27.980984,25.792278 0 0 1 59.413036,191.50692 27.980984,25.792278 0 0 1 87.39402,217.29919 Z" />
      <path
         sodipodi:type="star"
         style="fill:#ffff00;fill-rule:evenodd;stroke-width:0.342424"
         id="path06"
         sodipodi:sides="5"
         sodipodi:cx="139.37627"
         sodipodi:cy="215.95181"
         sodipodi:r1="34.43166"
         sodipodi:r2="17.21583"
         sodipodi:arg1="0.59548852"
         sodipodi:arg2="1.2238071"
         inkscape:flatsided="false"
         inkscape:rounded="0"
         inkscape:randomized="0"
         inkscape:transform-center-x="2.9540026"
         inkscape:transform-center-y="0.34924413"
         d="m 167.88136,235.26499 -22.65054,-3.1234 -15.41391,16.88828 -4.02888,-22.50713 -20.82487,-9.44074 20.16056,-10.78678 2.54344,-22.72297 16.48879,15.84053 22.3968,-4.60283 -9.96993,20.57677 z" />
    </g>
    <g
       id="g05"
       transform="translate(-3.9814864,37.147042)">
      <ellipse
         style="fill:#ff0000;fill-rule:evenodd;stroke-width:0.342424"
         id="ellipse04"
         cx="59.413036"
         cy="217.29919"
         rx="27.980984"
         ry="25.792278"
         d="M 87.39402,217.29919 A 27.980984,25.792278 0 0 1 59.413036,243.09147 27.980984,25.792278 0 0 1 31.432053,217.29919 27.980984,25.792278 0 0 1 59.413036,191.50692 27.980984,25.792278 0 0 1 87.39402,217.29919 Z" />
      <path
         sodipodi:type="star"
         style="fill:#ffff00;fill-rule:evenodd;stroke-width:0.342424"
         id="path03"
         sodipodi:sides="5"
         sodipodi:cx="139.37627"
         sodipodi:cy="215.95181"
         sodipodi:r1="34.43166"
         sodipodi:r2="17.21583"
         sodipodi:arg1="0.59548852"
         sodipodi:arg2="1.2238071"
         inkscape:flatsided="false"
         inkscape:rounded="0"
         inkscape:randomized="0"
         inkscape:transform-center-x="2.9540026"
         inkscape:transform-center-y="0.34924413"
         d="m 167.88136,235.26499 -22.65054,-3.1234 -15.41391,16.88828 -4.02888,-22.50713 -20.82487,-9.44074 20.16056,-10.78678 2.54344,-22.72297 16.48879,15.84053 22.3968,-4.60283 -9.96993,20.57677 z" />
    </g>
  </g>
  <path
     style="fill:none;stroke:#000000;stroke-width:1px;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1"
     d="m 20.589764,94.953503 c -0.05582,-0.805737 -0.157368,-1.6081 -0.29644,-2.403705 C 19.184916,86.208811 15.653194,80.400477 10.841012,76.125014 6.0288291,71.84955 -0.00763193,69.063555 -6.2956887,67.686228 c -12.5761133,-2.754654 -25.7185673,0.01324 -37.9220783,4.114587 -10.090817,3.391318 -19.995297,7.769961 -28.315638,14.410559 -8.320341,6.640599 -14.992022,15.740606 -17.219595,26.150386 -1.113787,5.20489 -1.094133,10.67402 0.267926,15.81952 1.362058,5.14551 4.086302,9.95292 7.951057,13.61285 3.864754,3.65992 8.877144,6.13699 14.161535,6.77466 5.284391,0.63767 10.811169,-0.60723 15.197395,-3.6226 4.109183,-2.82491 7.084699,-7.0419 9.302359,-11.50816 2.21766,-4.46626 3.753059,-9.23292 5.557538,-13.8815 1.80448,-4.64859 3.922243,-9.25347 7.122195,-13.07784 3.199952,-3.82437 7.611527,-6.83244 12.555636,-7.481495 3.611781,-0.474149 7.338935,0.340015 10.5613493,2.038785 3.2224143,1.69877 5.9531062,4.25221 8.0980519,7.19652 4.2898913,5.88863 6.2079593,13.14765 7.6297887,20.2931 1.4266861,7.16986 2.4458591,14.48034 2.0083551,21.77767 -0.437505,7.29732 -2.4025816,14.62346 -6.5314674,20.65625 -4.12888590446,6.0328 -10.5683293,10.65782 -17.8006486,11.7236 -3.61616,0.53289 -7.376261,0.17098 -10.781012,-1.15878 -3.404751,-1.32975 -6.439357,-3.6381 -8.529723,-6.63659 -2.525603,-3.62281 -3.606079,-8.15842 -3.338928,-12.5666 0.267151,-4.40818 1.831555,-8.68518 4.169542,-12.43181 4.675973,-7.49327 12.192398,-12.72367 19.794983,-17.21972 7.602584,-4.49605 15.5950245,-8.54106 22.0991919,-14.5168 3.2520841,-2.98787 6.1059591,-6.45534 8.0873251,-10.40219 1.981367,-3.94685 3.065535,-8.391418 2.760315,-12.797127 z"
     id="path02"
     inkscape:path-effect="#path-effect189"
     inkscape:original-d="m 20.589764,94.953503 c 17.856477,-26.645749 -0.196626,-1.60147 -0.29644,-2.403705 -0.09981,-0.802235 -43.006394,-13.831655 -64.511091,-20.748983 -21.504697,-6.917328 -7.51166,50.480965 -7.95732,73.145375 -0.44566,22.6644 25.233933,-32.34805 34.537728,-45.948995 C -8.333563,85.39625 -0.11023229,118.6838 8.6518309,128.5256 17.413894,138.3674 -19.1035,159.4347 -32.982665,174.88775 -46.861831,190.3408 2.7332877,121.59925 20.589764,94.953503 Z"
     sodipodi:nodetypes="ssssssss" />
  <path
     d="m 20.589764,94.953503 c -0.05582,-0.805737 -0.157368,-1.6081 -0.29644,-2.403705 C 19.184916,86.208811 15.653194,80.400477 10.841012,76.125014 6.0288291,71.84955 -0.00763193,69.063555 -6.2956887,67.686228 c -12.5761133,-2.754654 -25.7185673,0.01324 -37.9220783,4.114587 -10.090817,3.391318 -19.995297,7.769961 -28.315638,14.410559 -8.320341,6.640599 -14.992022,15.740606 -17.219595,26.150386 -1.113787,5.20489 -1.094133,10.67402 0.267926,15.81952 1.362058,5.14551 4.086302,9.95292 7.951057,13.61285 3.864754,3.65992 8.877144,6.13699 14.161535,6.77466 5.284391,0.63767 10.811169,-0.60723 15.197395,-3.6226 4.109183,-2.82491 7.084699,-7.0419 9.302359,-11.50816 2.21766,-4.46626 3.753059,-9.23292 5.557538,-13.8815 1.80448,-4.64859 3.922243,-9.25347 7.122195,-13.07784 3.199952,-3.82437 7.611527,-6.83244 12.555636,-7.481495 3.611781,-0.474149 7.338935,0.340015 10.5613493,2.038785 3.2224143,1.69877 5.9531062,4.25221 8.0980519,7.19652 4.2898913,5.88863 6.2079593,13.14765 7.6297887,20.2931 1.4266861,7.16986 2.4458591,14.48034 2.0083551,21.77767 -0.437505,7.29732 -2.4025816,14.62346 -6.5314674,20.65625 -4.12888590446,6.0328 -10.5683293,10.65782 -17.8006486,11.7236 -3.61616,0.53289 -7.376261,0.17098 -10.781012,-1.15878 -3.404751,-1.32975 -6.439357,-3.6381 -8.529723,-6.63659 -2.525603,-3.62281 -3.606079,-8.15842 -3.338928,-12.5666 0.267151,-4.40818 1.831555,-8.68518 4.169542,-12.43181 4.675973,-7.49327 12.192398,-12.72367 19.794983,-17.21972 7.602584,-4.49605 15.5950245,-8.54106 22.0991919,-14.5168 3.2520841,-2.98787 6.1059591,-6.45534 8.0873251,-10.40219 1.981367,-3.94685 3.065535,-8.391418 2.760315,-12.797127 z"
     id="path01"
     inkscape:path-effect="#path-effect191"
     inkscape:original-d="M 0,0"
     style="fill:#00ffff" />
</svg>
)"""";

   testDoc(svg);
}