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

class LPEPerspectiveEnvelopeTest : public LPESPathsTest {};

// INKSCAPE 0.92.5

TEST_F(LPEPerspectiveEnvelopeTest, mixed_0_92_5)
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
   inkscape:version="0.92.3 (unknown)">
  <defs
     id="defs2">
    <inkscape:path-effect
       effect="perspective-envelope"
       up_left_point="19.294789,32.326008"
       up_right_point="76.515527,30.38769"
       down_left_point="-8.8108261,69.390131"
       down_right_point="88.145437,104.27986"
       id="path-effect303"
       is_visible="true"
       deform_type="perspective"
       horizontal_mirror="false"
       vertical_mirror="false"
       overflow_perspective="false" />
    <inkscape:path-effect
       effect="perspective-envelope"
       up_left_point="85.686691,-11.43829"
       up_right_point="136.33092,53.312894"
       down_left_point="85.686691,79.489613"
       down_right_point="238.38908,79.489613"
       id="path-effect3805"
       is_visible="true"
       deform_type="perspective"
       horizontal_mirror="false"
       vertical_mirror="false"
       overflow_perspective="true" />
    <inkscape:path-effect
       effect="perspective-envelope"
       up_left_point="-53.672617,234.35352"
       up_right_point="47.523319,252.49638"
       down_left_point="-50.648808,329.50595"
       down_right_point="77.761413,329.50595"
       id="path-effect837"
       is_visible="true"
       deform_type="perspective"
       horizontal_mirror="false"
       vertical_mirror="false"
       overflow_perspective="false"
       />
    <inkscape:path-effect
       effect="perspective-envelope"
       up_left_point="-42.333332,246.44876"
       up_right_point="25.600699,238.88924"
       down_left_point="-50.648808,329.50595"
       down_right_point="77.761413,329.50595"
       id="path-effect4625"
       is_visible="true"
       deform_type="envelope_deformation"
       horizontal_mirror="false"
       vertical_mirror="false"
       overflow_perspective="false"
        />
    <inkscape:path-effect
       effect="perspective-envelope"
       up_left_point="4.9416287,26.026668"
       up_right_point="77.80573,25.270718"
       down_left_point="4.9416287,85.381068"
       down_right_point="77.80573,86.137018"
       id="path-effect4611"
       is_visible="true"
       deform_type="perspective"
       horizontal_mirror="false"
       vertical_mirror="false"
       overflow_perspective="false"
        />
    <inkscape:path-effect
       effect="perspective-envelope"
       up_left_point="123.58118,41.749945"
       up_right_point="197.95064,140.73721"
       down_left_point="85.68669,97.903634"
       down_right_point="264.09146,29.111974"
       id="path-effect4606"
       is_visible="true"
       deform_type="perspective"
       horizontal_mirror="false"
       vertical_mirror="false"
       overflow_perspective="true"
        />
  </defs>
  <g
     inkscape:label="Layer 1"
     inkscape:groupmode="layer"
     id="layer1">
    <path
       style="fill:#00ff00;stroke:#000000;stroke-width:3;stroke-miterlimit:4;stroke-dasharray:none"
       id="path10"
       transform="translate(12.599068,9.6915908)"
       inkscape:path-effect="#path-effect3805"
       sodipodi:type="arc"
       sodipodi:cx="162.03789"
       sodipodi:cy="34.025661"
       sodipodi:rx="76.351196"
       sodipodi:ry="45.463951"
       d="m 161.74888,59.832303 c 29.1687,7.481429 152.14838,19.65731 -1113.65486,19.65731 986.305191,0 1037.592671,25.384057 1037.592671,282.752597 0,-429.688957 25.345799,-341.274624 39.323589,-323.403327 7.43791,9.509749 20.23085,16.759375 36.7386,20.99342 z" />
    <path
       style="fill:#00ff00;stroke:#000000;stroke-width:3;stroke-miterlimit:4;stroke-dasharray:none"
       d="m 84.581373,81.635108 c 0,0 -43.455475,-16.354067 -43.455475,-16.354067 0,0 -28.766246,11.727291 -28.766246,11.727291 0,0 8.659219,-22.024213 8.659219,-22.024213 0,0 -12.173634,-8.877822 -12.173634,-8.877822 0,0 19.788039,-3.86814 19.788039,-3.86814 0,0 8.940451,-10.531337 8.940451,-10.531337 0,0 11.504129,7.025541 11.504129,7.025541 0,0 28.847537,3.405973 28.847537,3.405973 0,0 -14.986551,10.642295 -14.986551,10.642295 0,0 21.642531,28.854479 21.642531,28.854479 z"
       id="path09"
       inkscape:test-threshold="5.0"
       inkscape:connector-curvature="0"
       sodipodi:nodetypes="ccccccccccc"
       inkscape:path-effect="#path-effect303"
       inkscape:original-d="M 88.145437,88.114051 54.95569,80.982141 30.26483,104.27986 26.79148,70.510681 -2.9958713,54.22766 28.04722,40.48905 34.32849,7.1278705 56.018371,29.498664 87.74944,35.824084 73.61788,57.43225 Z" />
    <g
       transform="translate(139.57099,-82.861554)"
       id="g08"
       inkscape:path-effect="#path-effect837">
      <path
         inkscape:connector-curvature="0"
         id="path07"
         d="m 12.089306,279.60018 c 3.8778,16.02133 -8.1662906,29.6312 -28.794541,28.74725 -22.508978,-0.96453 -34.556281,1.87795 -35.197608,-18.30311 -0.568766,-17.89769 8.535213,-43.60597 27.230274,-40.62198 17.3984954,2.77703 33.2517303,15.67549 36.761875,30.17784 z"
         style="fill:#00ff00;stroke:#000000;stroke-width:3;stroke-miterlimit:4;stroke-dasharray:none"
         inkscape:original-d="m 18.142857,261.84821 c -1e-6,24.00631 -15.3995392,43.46726 -34.395832,43.46726 -18.996293,0 -28.580878,3.79886 -28.580879,-20.20744 -3e-6,-24.00631 9.584583,-66.72709 28.580879,-66.72709 18.9962959,0 34.395835,19.46096 34.395832,43.46727 z"
         sodipodi:nodetypes="sssss" />
      <path
         inkscape:connector-curvature="0"
         id="path06"
         d="m 47.352847,290.03937 c 7.132806,20.64378 -2.237514,39.46658 -23.570059,39.46658 -23.54911526,0 -45.244046,-6.29949 -48.277375,-32.48131 -2.610569,-22.53282 10.899934,-46.63935 29.9530979,-43.56694 17.6287331,2.84272 35.5849471,18.32103 41.8943361,36.58167 z"
         style="fill:#00ff00;stroke:#000000;stroke-width:3;stroke-miterlimit:4;stroke-dasharray:none"
         inkscape:original-d="m 57.452381,273.1875 c 0,31.10382 -17.768699,56.31845 -39.6875,56.31845 -21.9188007,0 -39.6875,-6.80061 -39.6875,-37.90443 0,-31.10382 17.7686993,-74.73247 39.6875,-74.73247 21.918801,0 39.6875,25.21463 39.6875,56.31845 z"
         sodipodi:nodetypes="sssss" />
      <path
         style="fill:#00ff00;stroke:#000000;stroke-width:3;stroke-miterlimit:4;stroke-dasharray:none"
         inkscape:transform-center-x="2.1026741"
         inkscape:transform-center-y="-2.7552688"
         d="m 57.871571,298.57945 c 0,0 -28.040186,-9.46617 -28.040186,-9.46617 0,0 -21.6484029,11.52265 -21.6484029,11.52265 0,0 1.0443189,-9.81853 1.0443189,-9.81853 0,0 -32.850363,-28.58162 -32.850363,-28.58162 0,0 28.6959052,-1.26985 28.6959052,-1.26985 0,0 4.6073787,-15.25423 4.6073787,-15.25423 0,0 18.7168551,16.59132 18.7168551,16.59132 0,0 24.469473,3.80136 24.469473,3.80136 0,0 -9.693266,12.29946 -9.693266,12.29946 0,0 14.698287,20.17561 14.698287,20.17561 z"
         id="path05"
         inkscape:test-threshold="5.0"
         inkscape:connector-curvature="0"
         inkscape:original-d="m 67.279759,285.66071 -31.371611,-11.38806 -27.1662632,19.38711 3.3474212,-14.05958 -29.044164,-39.14141 32.073905,-9.22663 10.582509,-31.65243 18.686428,27.65291 33.373429,0.28342 -20.525058,26.31707 z"
         sodipodi:nodetypes="ccccccccccc" />
    </g>
    <g
       id="g04"
       inkscape:path-effect="#path-effect4625"
       transform="translate(24.395035,-58.26932)">
      <path
         style="fill:#00ff00;stroke:#000000;stroke-width:3;stroke-miterlimit:4;stroke-dasharray:none"
         d="m 13.038299,289.60296 c 5.791599,18.3682 -2.500178,33.31874 -18.5202007,33.39297 -16.0200233,0.0742 -43.4736393,-10.40278 -41.7289763,-27.82894 1.744662,-17.42616 21.444904,-36.52531 31.519995,-37.34266 10.0750899,-0.81734 22.9375835,13.41043 28.729182,31.77863 z"
         id="path03"
         inkscape:original-d="m 18.142857,261.84821 c -1e-6,24.00631 -15.3995392,43.46726 -34.395832,43.46726 -18.996293,0 -52.809855,-14.61516 -52.809856,-38.62146 -3e-6,-24.00631 33.81356,-48.31307 52.809856,-48.31307 18.9962959,0 34.395835,19.46096 34.395832,43.46727 z"
         inkscape:connector-curvature="0"
         sodipodi:nodetypes="sssss" />
      <path
         style="fill:#00ff00;stroke:#000000;stroke-width:3;stroke-miterlimit:4;stroke-dasharray:none"
         d="m 44.378197,297.55721 c 11.905364,24.34897 0.08102,31.94874 -19.088826,31.94874 -19.1698455,0 -35.651984,-7.25302 -38.669771,-30.49105 -3.017788,-23.23803 3.8631124,-42.85279 15.3689201,-43.81079 11.5058079,-0.95801 30.4843129,18.00412 42.3896769,42.3531 z"
         id="path02"
         inkscape:original-d="m 57.452381,273.1875 c 0,31.10382 -17.768699,40.8119 -39.6875,40.8119 -21.9188007,0 -39.6875,-9.70808 -39.6875,-40.8119 0,-31.10382 17.7686993,-56.31845 39.6875,-56.31845 21.918801,0 39.6875,25.21463 39.6875,56.31845 z"
         inkscape:connector-curvature="0"
         sodipodi:nodetypes="sssss" />
      <path
         inkscape:connector-curvature="0"
         id="path01"
         inkscape:test-threshold="5.0"
         d="m 56.744799,307.19628 c 0,0 -27.712443,-8.40445 -27.712443,-8.40445 0,0 -15.853025,15.23748 -15.853025,15.23748 0,0 -6.1440788,-25.40783 -6.1440788,-25.40783 0,0 -17.3713082,-1.92674 -17.3713082,-1.92674 0,0 14.2444974,-20.40475 14.2444974,-20.40475 0,0 -2.3952898,-24.72035 -2.3952898,-24.72035 0,0 17.9299824,20.52283 17.9299824,20.52283 0,0 18.892951,-1.07856 18.892951,-1.07856 0,0 -1.508159,21.39797 -1.508159,21.39797 0,0 19.916873,24.7844 19.916873,24.7844 z"
         inkscape:transform-center-y="-2.7552688"
         inkscape:transform-center-x="2.1026741"
         style="fill:#00ff00;stroke:#000000;stroke-width:3;stroke-miterlimit:4;stroke-dasharray:none"
         inkscape:original-d="M 67.279759,285.66071 35.908148,274.27265 8.7418848,293.65976 9.8782199,260.30448 -15.01654,256.93447 l 30.135587,-25.70233 10.582509,-31.65243 18.686428,27.65291 33.373429,0.28342 -20.525058,26.31707 z"
         sodipodi:nodetypes="ccccccccccc" />
    </g>
  </g>
</svg>
)"""";
   testDoc(svg);
}

// INKSCAPE 1.0.2

TEST_F(LPEPerspectiveEnvelopeTest, multi_mm_1_02)
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
   inkscape:version="1.0.2 (e86c870879, 2021-01-15)">
  <defs
     id="defs2">
    <inkscape:path-effect
       effect="perspective-envelope"
       up_left_point="-53.672617,234.35352"
       up_right_point="47.523319,252.49638"
       down_left_point="-50.648808,329.50595"
       down_right_point="77.761413,329.50595"
       id="path-effect837"
       is_visible="true"
       deform_type="perspective"
       horizontal_mirror="false"
       vertical_mirror="false"
       overflow_perspective="false"
       lpeversion="1" />
    <inkscape:path-effect
       effect="perspective-envelope"
       up_left_point="-42.333332,246.44876"
       up_right_point="25.600699,238.88924"
       down_left_point="-50.648808,329.50595"
       down_right_point="77.761413,329.50595"
       id="path-effect4625"
       is_visible="true"
       deform_type="envelope_deformation"
       horizontal_mirror="false"
       vertical_mirror="false"
       overflow_perspective="false"
       lpeversion="1" />
    <inkscape:path-effect
       effect="perspective-envelope"
       up_left_point="5.6012502,54.745291"
       up_right_point="78.46535,53.989334"
       down_left_point="5.6012502,114.09968"
       down_right_point="78.46535,114.85564"
       id="path-effect4611"
       is_visible="true"
       deform_type="perspective"
       horizontal_mirror="true"
       vertical_mirror="false"
       overflow_perspective="false"
       lpeversion="1" />
    <inkscape:path-effect
       effect="perspective-envelope"
       up_left_point="124.2408,64.65361"
       up_right_point="202.4869,162.67172"
       down_left_point="90.222945,119.83814"
       down_right_point="268.62772,51.046473"
       id="path-effect4606"
       is_visible="true"
       deform_type="perspective"
       horizontal_mirror="false"
       vertical_mirror="false"
       overflow_perspective="true"
       lpeversion="1" />
  </defs>
  <g
     inkscape:label="Layer 1"
     inkscape:groupmode="layer"
     id="layer1">
    <path
       style="fill:#00ff00;stroke:#000000;stroke-width:3;stroke-miterlimit:4;stroke-dasharray:none"
       id="path10"
       inkscape:path-effect="#path-effect4606"
       sodipodi:type="arc"
       sodipodi:cx="166.57414"
       sodipodi:cy="55.960159"
       sodipodi:rx="76.351196"
       sodipodi:ry="63.877975"
       d="M 235.87869,106.3167 C 254.04376,75.659694 508.57261,-41.474643 -90.243989,189.42496 59.957311,131.50836 97.89519,107.39205 107.3138,92.112973 116.68243,76.914968 111.40937,48.579803 49.523588,-28.943898 315.30351,303.99596 217.51759,137.30457 235.87869,106.3167 Z" />
    <path
       style="fill:#00ff00;stroke:#000000;stroke-width:3;stroke-miterlimit:4;stroke-dasharray:none"
       inkscape:transform-center-x="4.2494356"
       inkscape:transform-center-y="-0.66018494"
       d="M 76.462953,104.7138 50.255334,100.10717 31.060668,114.36382 28.380818,93.540646 5.6012502,83.520602 29.349105,75.049916 34.2023,54.448558 51.846263,69.958773 78.46535,67.110209 64.933598,85.500328 Z"
       id="path09"
       inkscape:connector-curvature="0"
       inkscape:path-effect="#path-effect4611"
       inkscape:original-d="M 88.80506,116.83267 55.61531,109.70076 30.92445,132.99849 27.4511,99.229291 -2.3362497,82.946271 28.70684,69.207661 l 6.28127,-33.36118 22.65904,25.27827 33.66939,-4.33533 -17.03904,29.36144 z" />
    <g
       transform="translate(147.98389,-86.12519)"
       id="g08"
       inkscape:path-effect="#path-effect837">
      <path
         inkscape:connector-curvature="0"
         id="path07"
         d="m 14.506377,279.85961 c 3.982482,15.94721 -7.074711,29.52212 -26.433623,28.69257 -21.023542,-0.90089 -39.956814,-17.91685 -40.527645,-35.87957 -0.509603,-16.03595 14.25945,-25.40871 31.789408,-22.61069 16.3764028,2.6139 31.565088,15.35493 35.17186,29.79769 z"
         style="fill:#00ff00;stroke:#000000;stroke-width:3;stroke-miterlimit:4;stroke-dasharray:none"
         inkscape:original-d="m 18.142857,261.84821 a 34.395832,43.467262 0 0 1 -34.395832,43.46726 34.395832,43.467262 0 0 1 -34.395833,-43.46726 34.395832,43.467262 0 0 1 34.395833,-43.46727 34.395832,43.467262 0 0 1 34.395832,43.46727 z" />
      <path
         inkscape:connector-curvature="0"
         id="path06"
         d="m 48.078317,290.106 c 7.167234,20.611 -1.369664,39.39995 -21.537595,39.39995 -22.1511579,0 -44.669577,-22.05727 -47.782862,-45.76668 -2.700317,-20.56444 10.979476,-32.81668 28.9597592,-29.91727 16.7006958,2.69306 34.0195388,18.04857 40.3606978,36.284 z"
         style="fill:#00ff00;stroke:#000000;stroke-width:3;stroke-miterlimit:4;stroke-dasharray:none"
         inkscape:original-d="m 57.452381,273.1875 a 39.6875,56.318451 0 0 1 -39.6875,56.31845 39.6875,56.318451 0 0 1 -39.6875,-56.31845 39.6875,56.318451 0 0 1 39.6875,-56.31845 39.6875,56.318451 0 0 1 39.6875,56.31845 z" />
      <path
         style="fill:#00ff00;stroke:#000000;stroke-width:3;stroke-miterlimit:4;stroke-dasharray:none"
         inkscape:transform-center-x="2.1026741"
         inkscape:transform-center-y="-2.7552688"
         d="m 58.239512,298.6066 -26.78745,-9.34694 -20.192356,11.56571 -4.1125405,-22.71576 -26.5491295,-15.3067 26.9124419,-1.483 4.0270081,-15.27523 18.025674,16.433 23.30339,3.6267 -8.979826,12.38348 z"
         id="path05"
         inkscape:connector-curvature="0"
         inkscape:original-d="M 67.279759,285.66071 35.908148,274.27265 8.7418848,293.65976 9.8782199,260.30448 -16.954858,240.45877 l 32.073905,-9.22663 10.582509,-31.65243 18.686428,27.65291 33.373429,0.28342 -20.525058,26.31707 z" />
    </g>
    <g
       id="g04"
       inkscape:path-effect="#path-effect4625"
       transform="translate(28.931292,-45.05725)">
      <path
         style="fill:#00ff00;stroke:#000000;stroke-width:3;stroke-miterlimit:4;stroke-dasharray:none"
         d="m 5.6020376,284.1459 c 4.4497304,16.09465 -5.99225793,29.31072 -23.3228336,29.51894 -17.330576,0.20821 -30.134253,-12.06369 -28.597814,-27.41006 1.536438,-15.34637 11.978427,-28.56245 23.322833,-29.51894 11.344407,-0.95648 24.1480853,11.31542 28.5978146,27.41006 z"
         id="path03"
         inkscape:original-d="m 18.142857,261.84821 a 34.395832,43.467262 0 0 1 -34.395832,43.46726 34.395832,43.467262 0 0 1 -34.395833,-43.46726 34.395832,43.467262 0 0 1 34.395833,-43.46727 34.395832,43.467262 0 0 1 34.395832,43.46727 z"
         inkscape:connector-curvature="0" />
      <path
         style="fill:#00ff00;stroke:#000000;stroke-width:3;stroke-miterlimit:4;stroke-dasharray:none"
         d="m 38.988535,290.74505 c 10.1973,21.40705 0.695147,38.7609 -21.223654,38.7609 -21.9188007,0 -40.699265,-16.447 -41.947341,-36.73541 -1.248075,-20.2884 8.254079,-37.64225 21.2236541,-38.7609 12.9695759,-1.11865 31.7500399,15.32835 41.9473409,36.73541 z"
         id="path02"
         inkscape:original-d="m 57.452381,273.1875 a 39.6875,56.318451 0 0 1 -39.6875,56.31845 39.6875,56.318451 0 0 1 -39.6875,-56.31845 39.6875,56.318451 0 0 1 39.6875,-56.31845 39.6875,56.318451 0 0 1 39.6875,56.31845 z"
         inkscape:connector-curvature="0" />
      <path
         inkscape:connector-curvature="0"
         id="path01"
         d="m 51.343338,299.13444 -29.22987,-7.10337 -18.7943879,13.59507 -4.19468784,-22.25606 -21.25586726,-12.14827 20.1119672,-7.46731 0.078506,-21.80049 17.6422422,17.76822 21.114891,-1.34887 -4.329649,19.05853 z"
         inkscape:transform-center-y="-2.7552688"
         inkscape:transform-center-x="2.1026741"
         style="fill:#00ff00;stroke:#000000;stroke-width:3;stroke-miterlimit:4;stroke-dasharray:none"
         inkscape:original-d="M 67.279759,285.66071 35.908148,274.27265 8.7418848,293.65976 9.8782199,260.30448 -16.954858,240.45877 l 32.073905,-9.22663 10.582509,-31.65243 18.686428,27.65291 33.373429,0.28342 -20.525058,26.31707 z" />
    </g>
  </g>
</svg>
)"""";

   testDoc(svg);
}

// INKSCAPE 1.0.2

TEST_F(LPEPerspectiveEnvelopeTest, multi_px_1_02)
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
   inkscape:version="1.0.2 (e86c870879, 2021-01-15)">
  <defs
     id="defs2">
    <inkscape:path-effect
       effect="perspective-envelope"
       up_left_point="-53.672617,234.35352"
       up_right_point="47.523319,252.49638"
       down_left_point="-50.648808,329.50595"
       down_right_point="77.761413,329.50595"
       id="path-effect837"
       is_visible="true"
       deform_type="perspective"
       horizontal_mirror="false"
       vertical_mirror="false"
       overflow_perspective="false"
       lpeversion="1" />
    <inkscape:path-effect
       effect="perspective-envelope"
       up_left_point="-42.333332,246.44876"
       up_right_point="25.600699,238.88924"
       down_left_point="-50.648808,329.50595"
       down_right_point="77.761413,329.50595"
       id="path-effect4625"
       is_visible="true"
       deform_type="envelope_deformation"
       horizontal_mirror="false"
       vertical_mirror="false"
       overflow_perspective="false"
       lpeversion="1" />
    <inkscape:path-effect
       effect="perspective-envelope"
       up_left_point="1.0649927,32.794487"
       up_right_point="73.929093,32.038537"
       down_left_point="1.0649927,92.148887"
       down_right_point="73.929093,92.904837"
       id="path-effect4611"
       is_visible="true"
       deform_type="perspective"
       horizontal_mirror="true"
       vertical_mirror="false"
       overflow_perspective="false"
       lpeversion="1" />
    <inkscape:path-effect
       effect="perspective-envelope"
       up_left_point="119.70454,42.719104"
       up_right_point="197.95064,140.73721"
       down_left_point="85.68669,97.903634"
       down_right_point="264.09146,29.111974"
       id="path-effect4606"
       is_visible="true"
       deform_type="perspective"
       horizontal_mirror="false"
       vertical_mirror="false"
       overflow_perspective="true"
       lpeversion="1" />
  </defs>
  <g
     inkscape:label="Layer 1"
     inkscape:groupmode="layer"
     id="layer1">
    <path
       style="fill:#00ff00;stroke:#000000;stroke-width:3;stroke-miterlimit:4;stroke-dasharray:none"
       id="path10"
       inkscape:path-effect="#path-effect4606"
       sodipodi:type="arc"
       sodipodi:cx="162.03789"
       sodipodi:cy="34.025661"
       sodipodi:rx="76.351196"
       sodipodi:ry="63.877975"
       d="M 231.34243,84.382201 C 249.50749,53.725195 504.03635,-63.409134 -94.780238,167.49045 55.421057,109.57386 93.358934,85.457546 102.77754,70.178468 112.14617,54.980463 106.87311,26.645294 44.987303,-50.878431 310.76721,282.06139 212.98133,115.37006 231.34243,84.382201 Z" />
    <path
       style="fill:#00ff00;stroke:#000000;stroke-width:3;stroke-miterlimit:4;stroke-dasharray:none"
       inkscape:transform-center-x="4.2494356"
       inkscape:transform-center-y="-0.66018494"
       d="M 71.926697,82.779292 45.719083,78.172666 26.524416,92.429318 23.844566,71.606149 1.0649927,61.586102 24.812853,53.115415 29.666048,32.514052 47.310011,48.024271 73.929093,45.175708 60.397346,63.565828 Z"
       id="path09"
       inkscape:connector-curvature="0"
       inkscape:path-effect="#path-effect4611"
       inkscape:original-d="m 84.2688,94.898164 -33.189747,-7.13191 -24.69086,23.297726 -3.47335,-33.769186 -29.7873503,-16.28302 31.0430903,-13.73861 6.28127,-33.36118 22.65904,25.27827 33.669387,-4.33533 -17.039037,29.36144 z" />
    <g
       transform="translate(143.44763,-108.05969)"
       id="g08"
       inkscape:path-effect="#path-effect837">
      <path
         inkscape:connector-curvature="0"
         id="path07"
         d="m 14.506377,279.85961 c 3.982482,15.94721 -7.074711,29.52212 -26.433623,28.69257 -21.023542,-0.90089 -39.956814,-17.91685 -40.527645,-35.87957 -0.509603,-16.03595 14.25945,-25.40871 31.789408,-22.61069 16.3764028,2.6139 31.565088,15.35493 35.17186,29.79769 z"
         style="fill:#00ff00;stroke:#000000;stroke-width:3;stroke-miterlimit:4;stroke-dasharray:none"
         inkscape:original-d="m 18.142857,261.84821 a 34.395832,43.467262 0 0 1 -34.395832,43.46726 34.395832,43.467262 0 0 1 -34.395833,-43.46726 34.395832,43.467262 0 0 1 34.395833,-43.46727 34.395832,43.467262 0 0 1 34.395832,43.46727 z" />
      <path
         inkscape:connector-curvature="0"
         id="path06"
         d="m 48.078317,290.106 c 7.167234,20.611 -1.369664,39.39995 -21.537595,39.39995 -22.1511579,0 -44.669577,-22.05727 -47.782862,-45.76668 -2.700317,-20.56444 10.979476,-32.81668 28.9597592,-29.91727 16.7006958,2.69306 34.0195388,18.04857 40.3606978,36.284 z"
         style="fill:#00ff00;stroke:#000000;stroke-width:3;stroke-miterlimit:4;stroke-dasharray:none"
         inkscape:original-d="m 57.452381,273.1875 a 39.6875,56.318451 0 0 1 -39.6875,56.31845 39.6875,56.318451 0 0 1 -39.6875,-56.31845 39.6875,56.318451 0 0 1 39.6875,-56.31845 39.6875,56.318451 0 0 1 39.6875,56.31845 z" />
      <path
         style="fill:#00ff00;stroke:#000000;stroke-width:3;stroke-miterlimit:4;stroke-dasharray:none"
         inkscape:transform-center-x="2.1026741"
         inkscape:transform-center-y="-2.7552688"
         d="m 58.239512,298.6066 -26.78745,-9.34694 -20.192356,11.56571 -4.1125405,-22.71576 -26.5491295,-15.3067 26.9124419,-1.483 4.0270081,-15.27523 18.025674,16.433 23.30339,3.6267 -8.979826,12.38348 z"
         id="path05"
         inkscape:connector-curvature="0"
         inkscape:original-d="M 67.279759,285.66071 35.908148,274.27265 8.7418848,293.65976 9.8782199,260.30448 -16.954858,240.45877 l 32.073905,-9.22663 10.582509,-31.65243 18.686428,27.65291 33.373429,0.28342 -20.525058,26.31707 z" />
    </g>
    <g
       id="g04"
       inkscape:path-effect="#path-effect4625"
       transform="translate(24.395035,-66.991752)">
      <path
         style="fill:#00ff00;stroke:#000000;stroke-width:3;stroke-miterlimit:4;stroke-dasharray:none"
         d="m 5.6020376,284.1459 c 4.4497304,16.09465 -5.99225793,29.31072 -23.3228336,29.51894 -17.330576,0.20821 -30.134253,-12.06369 -28.597814,-27.41006 1.536438,-15.34637 11.978427,-28.56245 23.322833,-29.51894 11.344407,-0.95648 24.1480853,11.31542 28.5978146,27.41006 z"
         id="path03"
         inkscape:original-d="m 18.142857,261.84821 a 34.395832,43.467262 0 0 1 -34.395832,43.46726 34.395832,43.467262 0 0 1 -34.395833,-43.46726 34.395832,43.467262 0 0 1 34.395833,-43.46727 34.395832,43.467262 0 0 1 34.395832,43.46727 z"
         inkscape:connector-curvature="0" />
      <path
         style="fill:#00ff00;stroke:#000000;stroke-width:3;stroke-miterlimit:4;stroke-dasharray:none"
         d="m 38.988535,290.74505 c 10.1973,21.40705 0.695147,38.7609 -21.223654,38.7609 -21.9188007,0 -40.699265,-16.447 -41.947341,-36.73541 -1.248075,-20.2884 8.254079,-37.64225 21.2236541,-38.7609 12.9695759,-1.11865 31.7500399,15.32835 41.9473409,36.73541 z"
         id="path02"
         inkscape:original-d="m 57.452381,273.1875 a 39.6875,56.318451 0 0 1 -39.6875,56.31845 39.6875,56.318451 0 0 1 -39.6875,-56.31845 39.6875,56.318451 0 0 1 39.6875,-56.31845 39.6875,56.318451 0 0 1 39.6875,56.31845 z"
         inkscape:connector-curvature="0" />
      <path
         inkscape:connector-curvature="0"
         id="path01"
         d="m 51.343338,299.13444 -29.22987,-7.10337 -18.7943879,13.59507 -4.19468784,-22.25606 -21.25586726,-12.14827 20.1119672,-7.46731 0.078506,-21.80049 17.6422422,17.76822 21.114891,-1.34887 -4.329649,19.05853 z"
         inkscape:transform-center-y="-2.7552688"
         inkscape:transform-center-x="2.1026741"
         style="fill:#00ff00;stroke:#000000;stroke-width:3;stroke-miterlimit:4;stroke-dasharray:none"
         inkscape:original-d="M 67.279759,285.66071 35.908148,274.27265 8.7418848,293.65976 9.8782199,260.30448 -16.954858,240.45877 l 32.073905,-9.22663 10.582509,-31.65243 18.686428,27.65291 33.373429,0.28342 -20.525058,26.31707 z" />
    </g>
  </g>
</svg>
)"""";

   testDoc(svg);
}