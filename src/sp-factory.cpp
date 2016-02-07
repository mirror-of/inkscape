/* ANSI-C code produced by gperf version 3.0.4 */
/* Command-line: gperf -CGD -t sp-factory.gperf  */
/* Computed positions: -k'5,7,$' */

#if !((' ' == 32) && ('!' == 33) && ('"' == 34) && ('#' == 35) \
      && ('%' == 37) && ('&' == 38) && ('\'' == 39) && ('(' == 40) \
      && (')' == 41) && ('*' == 42) && ('+' == 43) && (',' == 44) \
      && ('-' == 45) && ('.' == 46) && ('/' == 47) && ('0' == 48) \
      && ('1' == 49) && ('2' == 50) && ('3' == 51) && ('4' == 52) \
      && ('5' == 53) && ('6' == 54) && ('7' == 55) && ('8' == 56) \
      && ('9' == 57) && (':' == 58) && (';' == 59) && ('<' == 60) \
      && ('=' == 61) && ('>' == 62) && ('?' == 63) && ('A' == 65) \
      && ('B' == 66) && ('C' == 67) && ('D' == 68) && ('E' == 69) \
      && ('F' == 70) && ('G' == 71) && ('H' == 72) && ('I' == 73) \
      && ('J' == 74) && ('K' == 75) && ('L' == 76) && ('M' == 77) \
      && ('N' == 78) && ('O' == 79) && ('P' == 80) && ('Q' == 81) \
      && ('R' == 82) && ('S' == 83) && ('T' == 84) && ('U' == 85) \
      && ('V' == 86) && ('W' == 87) && ('X' == 88) && ('Y' == 89) \
      && ('Z' == 90) && ('[' == 91) && ('\\' == 92) && (']' == 93) \
      && ('^' == 94) && ('_' == 95) && ('a' == 97) && ('b' == 98) \
      && ('c' == 99) && ('d' == 100) && ('e' == 101) && ('f' == 102) \
      && ('g' == 103) && ('h' == 104) && ('i' == 105) && ('j' == 106) \
      && ('k' == 107) && ('l' == 108) && ('m' == 109) && ('n' == 110) \
      && ('o' == 111) && ('p' == 112) && ('q' == 113) && ('r' == 114) \
      && ('s' == 115) && ('t' == 116) && ('u' == 117) && ('v' == 118) \
      && ('w' == 119) && ('x' == 120) && ('y' == 121) && ('z' == 122) \
      && ('{' == 123) && ('|' == 124) && ('}' == 125) && ('~' == 126))
/* The character set is not based on ISO-646.  */
#error "gperf generated tables don't work with this execution character set. Please report a bug to <bug-gnu-gperf@gnu.org>."
#endif

#line 1 "sp-factory.gperf"

/*
 * Factory for SPObject tree
 *
 * Authors:
 *   Markus Engel
 *   Liam P. White
 *
 * Copyright (C) 2013-2016 Authors
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "sp-factory.h"

// objects
#include "box3d.h"
#include "box3d-side.h"
#include "color-profile.h"
#include "persp3d.h"
#include "sp-anchor.h"
#include "sp-clippath.h"
#include "sp-defs.h"
#include "sp-desc.h"
#include "sp-ellipse.h"
#include "sp-filter.h"
#include "sp-flowdiv.h"
#include "sp-flowregion.h"
#include "sp-flowtext.h"
#include "sp-font.h"
#include "sp-font-face.h"
#include "sp-glyph.h"
#include "sp-glyph-kerning.h"
#include "sp-guide.h"
#include "sp-hatch.h"
#include "sp-hatch-path.h"
#include "sp-image.h"
#include "sp-item-group.h"
#include "sp-line.h"
#include "sp-linear-gradient.h"
#include "sp-marker.h"
#include "sp-mask.h"
#include "sp-mesh.h"
#include "sp-mesh-patch.h"
#include "sp-mesh-row.h"
#include "sp-metadata.h"
#include "sp-missing-glyph.h"
#include "sp-namedview.h"
#include "sp-object.h"
#include "sp-offset.h"
#include "sp-path.h"
#include "sp-pattern.h"
#include "sp-polygon.h"
#include "sp-polyline.h"
#include "sp-radial-gradient.h"
#include "sp-rect.h"
#include "sp-root.h"
#include "sp-script.h"
#include "sp-solid-color.h"
#include "sp-spiral.h"
#include "sp-star.h"
#include "sp-stop.h"
#include "sp-string.h"
#include "sp-style-elem.h"
#include "sp-switch.h"
#include "sp-symbol.h"
#include "sp-tag.h"
#include "sp-tag-use.h"
#include "sp-text.h"
#include "sp-textpath.h"
#include "sp-title.h"
#include "sp-tref.h"
#include "sp-tspan.h"
#include "sp-use.h"
#include "live_effects/lpeobject.h"

// filters
#include "filters/blend.h"
#include "filters/colormatrix.h"
#include "filters/componenttransfer.h"
#include "filters/componenttransfer-funcnode.h"
#include "filters/composite.h"
#include "filters/convolvematrix.h"
#include "filters/diffuselighting.h"
#include "filters/displacementmap.h"
#include "filters/distantlight.h"
#include "filters/flood.h"
#include "filters/gaussian-blur.h"
#include "filters/image.h"
#include "filters/merge.h"
#include "filters/mergenode.h"
#include "filters/morphology.h"
#include "filters/offset.h"
#include "filters/pointlight.h"
#include "filters/specularlighting.h"
#include "filters/spotlight.h"
#include "filters/tile.h"
#include "filters/turbulence.h"

#define NEW_OBJECT_FUNC(derived) \
    static SPObject *create_##derived() { return new derived; }

struct factory_object {
    char const *name;
    SPObject *(*create_func)();
};

factory_object const *get_object_create_func(char const *str, unsigned int len);

SPObject *SPFactory::createObject(std::string const& id)
{
    SPObject *ret = NULL;
    if (id.empty()) return ret;

    factory_object const *obj = get_object_create_func(id.c_str(), id.length());
    if (obj) {
        ret = obj->create_func();
    } else {
        fprintf(stderr, "WARNING: unknown type: %s\n", id.c_str());
    }
    
    return ret;
}

std::string NodeTraits::get_type_string(Inkscape::XML::Node const &node)
{
    std::string name;

    switch (node.type()) {
    case Inkscape::XML::TEXT_NODE:
        name = "string";
        break;

    case Inkscape::XML::ELEMENT_NODE: {
        char const *const sptype = node.attribute("sodipodi:type");

        if (sptype) {
            name = sptype;
        } else {
            name = node.name();
        }
        break;
    }
    default:
        name = "";
        break;
    }

    return name;
}

// objects

NEW_OBJECT_FUNC(SPBox3D);
NEW_OBJECT_FUNC(Box3DSide);
NEW_OBJECT_FUNC(Persp3D);
NEW_OBJECT_FUNC(SPAnchor);
NEW_OBJECT_FUNC(SPClipPath);
NEW_OBJECT_FUNC(SPDefs);
NEW_OBJECT_FUNC(SPDesc);
NEW_OBJECT_FUNC(SPFilter);
NEW_OBJECT_FUNC(SPFlowdiv);
NEW_OBJECT_FUNC(SPFlowtspan);
NEW_OBJECT_FUNC(SPFlowpara);
NEW_OBJECT_FUNC(SPFlowline);
NEW_OBJECT_FUNC(SPFlowregionbreak);
NEW_OBJECT_FUNC(SPFlowregion);
NEW_OBJECT_FUNC(SPFlowregionExclude);
NEW_OBJECT_FUNC(SPFlowtext);
NEW_OBJECT_FUNC(SPFont);
NEW_OBJECT_FUNC(SPFontFace);
NEW_OBJECT_FUNC(SPGlyph);
NEW_OBJECT_FUNC(SPHkern);
NEW_OBJECT_FUNC(SPVkern);
NEW_OBJECT_FUNC(SPGuide);
NEW_OBJECT_FUNC(SPHatch);
NEW_OBJECT_FUNC(SPHatchPath);
NEW_OBJECT_FUNC(SPImage);
NEW_OBJECT_FUNC(SPGroup);
NEW_OBJECT_FUNC(SPLine);
NEW_OBJECT_FUNC(SPLinearGradient);
NEW_OBJECT_FUNC(SPMarker);
NEW_OBJECT_FUNC(SPMask);
NEW_OBJECT_FUNC(SPMesh);
NEW_OBJECT_FUNC(SPMeshpatch);
NEW_OBJECT_FUNC(SPMeshrow);
NEW_OBJECT_FUNC(SPMetadata);
NEW_OBJECT_FUNC(SPMissingGlyph);
NEW_OBJECT_FUNC(SPNamedView);
NEW_OBJECT_FUNC(SPOffset);
NEW_OBJECT_FUNC(SPPath);
NEW_OBJECT_FUNC(SPPattern);
NEW_OBJECT_FUNC(SPPolygon);
NEW_OBJECT_FUNC(SPPolyLine);
NEW_OBJECT_FUNC(SPRadialGradient);
NEW_OBJECT_FUNC(SPRect);
NEW_OBJECT_FUNC(SPRoot);
NEW_OBJECT_FUNC(SPScript);
NEW_OBJECT_FUNC(SPSolidColor);
NEW_OBJECT_FUNC(SPSpiral);
NEW_OBJECT_FUNC(SPStar);
NEW_OBJECT_FUNC(SPStop);
NEW_OBJECT_FUNC(SPString);
NEW_OBJECT_FUNC(SPStyleElem);
NEW_OBJECT_FUNC(SPSwitch);
NEW_OBJECT_FUNC(SPSymbol);
NEW_OBJECT_FUNC(SPTag);
NEW_OBJECT_FUNC(SPTagUse);
NEW_OBJECT_FUNC(SPText);
NEW_OBJECT_FUNC(SPTitle);
NEW_OBJECT_FUNC(SPTRef);
NEW_OBJECT_FUNC(SPTSpan);
NEW_OBJECT_FUNC(SPTextPath);
NEW_OBJECT_FUNC(SPUse);
NEW_OBJECT_FUNC(LivePathEffectObject);
NEW_OBJECT_FUNC(SPObject);

static SPObject *create_nothing()      { return NULL; }
static SPObject *create_ColorProfile() { return new Inkscape::ColorProfile; }
static SPObject *create_ellipse() {
    SPGenericEllipse *e = new SPGenericEllipse;
    e->type = SP_GENERIC_ELLIPSE_ELLIPSE;
    return e;
}
static SPObject *create_circle() {
    SPGenericEllipse *e = new SPGenericEllipse;
    e->type = SP_GENERIC_ELLIPSE_CIRCLE;
    return e;
}
static SPObject *create_arc() {
    SPGenericEllipse *e = new SPGenericEllipse;
    e->type = SP_GENERIC_ELLIPSE_ARC;
    return e;
}

// filters

NEW_OBJECT_FUNC(SPFeBlend);
NEW_OBJECT_FUNC(SPFeColorMatrix);
NEW_OBJECT_FUNC(SPFeComponentTransfer);
NEW_OBJECT_FUNC(SPFeComposite);
NEW_OBJECT_FUNC(SPFeConvolveMatrix);
NEW_OBJECT_FUNC(SPFeDiffuseLighting);
NEW_OBJECT_FUNC(SPFeDisplacementMap);
NEW_OBJECT_FUNC(SPFeDistantLight);
NEW_OBJECT_FUNC(SPFeFlood);
NEW_OBJECT_FUNC(SPGaussianBlur);
NEW_OBJECT_FUNC(SPFeImage);
NEW_OBJECT_FUNC(SPFeMerge);
NEW_OBJECT_FUNC(SPFeMergeNode);
NEW_OBJECT_FUNC(SPFeMorphology);
NEW_OBJECT_FUNC(SPFeOffset);
NEW_OBJECT_FUNC(SPFePointLight);
NEW_OBJECT_FUNC(SPFeSpecularLighting);
NEW_OBJECT_FUNC(SPFeSpotLight);
NEW_OBJECT_FUNC(SPFeTile);
NEW_OBJECT_FUNC(SPFeTurbulence);

static SPObject *create_SPFeFuncNode_R() { return new SPFeFuncNode(SPFeFuncNode::R); }
static SPObject *create_SPFeFuncNode_G() { return new SPFeFuncNode(SPFeFuncNode::G); }
static SPObject *create_SPFeFuncNode_B() { return new SPFeFuncNode(SPFeFuncNode::B); }
static SPObject *create_SPFeFuncNode_A() { return new SPFeFuncNode(SPFeFuncNode::A); }
#line 265 "sp-factory.gperf"
struct factory_object;

#define TOTAL_KEYWORDS 94
#define MIN_WORD_LENGTH 3
#define MAX_WORD_LENGTH 23
#define MIN_HASH_VALUE 3
#define MAX_HASH_VALUE 151
/* maximum key range = 149, duplicates = 0 */

#ifdef __GNUC__
__inline
#else
#ifdef __cplusplus
inline
#endif
#endif
static unsigned int
hash (register const char *str, register unsigned int len)
{
  static const unsigned char asso_values[] =
    {
      152, 152, 152, 152, 152, 152, 152, 152, 152, 152,
      152, 152, 152, 152, 152, 152, 152, 152, 152, 152,
      152, 152, 152, 152, 152, 152, 152, 152, 152, 152,
      152, 152, 152, 152, 152, 152, 152, 152, 152, 152,
      152, 152, 152, 152, 152, 152, 152, 152, 152, 152,
      152, 152, 152, 152, 152, 152, 152, 152, 152, 152,
      152, 152, 152, 152, 152,  90,  60,  40,  35, 152,
        0,  70, 152,  75, 152, 152, 152,  35, 152,  25,
       70, 152,   5,  25,  55, 152, 152, 152, 152, 152,
      152, 152, 152, 152, 152, 152, 152,  55, 152,   0,
       45,   5,  35,  40,  60,  10, 152,  50,  15,   0,
       10,   0,  10, 152,  20,  40,   0,   0,  20,  30,
        5,  10, 152, 152, 152, 152, 152, 152, 152, 152,
      152, 152, 152, 152, 152, 152, 152, 152, 152, 152,
      152, 152, 152, 152, 152, 152, 152, 152, 152, 152,
      152, 152, 152, 152, 152, 152, 152, 152, 152, 152,
      152, 152, 152, 152, 152, 152, 152, 152, 152, 152,
      152, 152, 152, 152, 152, 152, 152, 152, 152, 152,
      152, 152, 152, 152, 152, 152, 152, 152, 152, 152,
      152, 152, 152, 152, 152, 152, 152, 152, 152, 152,
      152, 152, 152, 152, 152, 152, 152, 152, 152, 152,
      152, 152, 152, 152, 152, 152, 152, 152, 152, 152,
      152, 152, 152, 152, 152, 152, 152, 152, 152, 152,
      152, 152, 152, 152, 152, 152, 152, 152, 152, 152,
      152, 152, 152, 152, 152, 152, 152, 152, 152, 152,
      152, 152, 152, 152, 152, 152
    };
  register int hval = len;

  switch (hval)
    {
      default:
        hval += asso_values[(unsigned char)str[6]];
      /*FALLTHROUGH*/
      case 6:
      case 5:
        hval += asso_values[(unsigned char)str[4]];
      /*FALLTHROUGH*/
      case 4:
      case 3:
        break;
    }
  return hval + asso_values[(unsigned char)str[len - 1]];
}

static const struct factory_object wordlist[] =
  {
#line 277 "sp-factory.gperf"
    {"arc",                create_arc},
#line 358 "sp-factory.gperf"
    {"rdf:RDF",            create_nothing},
#line 326 "sp-factory.gperf"
    {"svg:text",           create_SPText},
#line 327 "sp-factory.gperf"
    {"svg:title",          create_SPTitle},
#line 331 "sp-factory.gperf"
    {"svg:use",            create_SPUse},
#line 318 "sp-factory.gperf"
    {"star",               create_SPStar},
#line 307 "sp-factory.gperf"
    {"inkscape:offset",    create_SPOffset},
#line 313 "sp-factory.gperf"
    {"svg:rect",           create_SPRect},
#line 329 "sp-factory.gperf"
    {"svg:tspan",          create_SPTSpan},
#line 332 "sp-factory.gperf"
    {"inkscape:path-effect", create_LivePathEffectObject},
#line 309 "sp-factory.gperf"
    {"svg:pattern",        create_SPPattern},
#line 360 "sp-factory.gperf"
    {"inkscape:_templateinfo", create_nothing},
#line 268 "sp-factory.gperf"
    {"inkscape:box3dside", create_Box3DSide},
#line 276 "sp-factory.gperf"
    {"svg:circle",         create_circle},
#line 275 "sp-factory.gperf"
    {"svg:ellipse",        create_ellipse},
#line 270 "sp-factory.gperf"
    {"svg:color-profile",  create_ColorProfile},
#line 297 "sp-factory.gperf"
    {"svg:line",           create_SPLine},
#line 311 "sp-factory.gperf"
    {"svg:polyline",       create_SPPolyLine},
#line 298 "sp-factory.gperf"
    {"svg:linearGradient", create_SPLinearGradient},
#line 291 "sp-factory.gperf"
    {"svg:vkern",          create_SPVkern},
#line 310 "sp-factory.gperf"
    {"svg:polygon",        create_SPPolygon},
#line 286 "sp-factory.gperf"
    {"svg:flowRoot",       create_SPFlowtext},
#line 328 "sp-factory.gperf"
    {"svg:tref",           create_SPTRef},
#line 299 "sp-factory.gperf"
    {"svg:marker",         create_SPMarker},
#line 336 "sp-factory.gperf"
    {"svg:feFuncR",        create_SPFeFuncNode_R},
#line 282 "sp-factory.gperf"
    {"svg:flowLine",       create_SPFlowline},
#line 287 "sp-factory.gperf"
    {"svg:font",           create_SPFont},
#line 320 "sp-factory.gperf"
    {"string",             create_SPString},
#line 280 "sp-factory.gperf"
    {"svg:flowSpan",       create_SPFlowtspan},
#line 319 "sp-factory.gperf"
    {"svg:stop",           create_SPStop},
#line 283 "sp-factory.gperf"
    {"svg:flowRegion",     create_SPFlowregion},
#line 325 "sp-factory.gperf"
    {"inkscape:tagref",    create_SPTagUse},
#line 285 "sp-factory.gperf"
    {"svg:flowRegionExclude", create_SPFlowregionExclude},
#line 324 "sp-factory.gperf"
    {"inkscape:tag",       create_SPTag},
#line 288 "sp-factory.gperf"
    {"svg:font-face",      create_SPFontFace},
#line 321 "sp-factory.gperf"
    {"svg:style",          create_SPStyleElem},
#line 323 "sp-factory.gperf"
    {"svg:symbol",         create_SPSymbol},
#line 279 "sp-factory.gperf"
    {"svg:flowDiv",        create_SPFlowdiv},
#line 304 "sp-factory.gperf"
    {"svg:metadata",       create_SPMetadata},
#line 357 "sp-factory.gperf"
    {"inkscape:grid",      create_SPObject},
#line 267 "sp-factory.gperf"
    {"inkscape:box3d",     create_SPBox3D},
#line 315 "sp-factory.gperf"
    {"svg:script",         create_SPScript},
#line 269 "sp-factory.gperf"
    {"inkscape:persp3d",   create_Persp3D},
#line 351 "sp-factory.gperf"
    {"svg:feOffset",       create_SPFeOffset},
#line 359 "sp-factory.gperf"
    {"inkscape:clipboard", create_nothing},
#line 292 "sp-factory.gperf"
    {"sodipodi:guide",     create_SPGuide},
#line 354 "sp-factory.gperf"
    {"svg:feSpotLight",    create_SPFeSpotLight},
#line 317 "sp-factory.gperf"
    {"spiral",             create_SPSpiral},
#line 330 "sp-factory.gperf"
    {"svg:textPath",       create_SPTextPath},
#line 308 "sp-factory.gperf"
    {"svg:path",           create_SPPath},
#line 295 "sp-factory.gperf"
    {"svg:image",          create_SPImage},
#line 278 "sp-factory.gperf"
    {"svg:filter",         create_SPFilter},
#line 303 "sp-factory.gperf"
    {"svg:meshrow",        create_SPMeshrow},
#line 272 "sp-factory.gperf"
    {"svg:clipPath",       create_SPClipPath},
#line 312 "sp-factory.gperf"
    {"svg:radialGradient", create_SPRadialGradient},
#line 290 "sp-factory.gperf"
    {"svg:hkern",          create_SPHkern},
#line 296 "sp-factory.gperf"
    {"svg:g",              create_SPGroup},
#line 348 "sp-factory.gperf"
    {"svg:feMerge",        create_SPFeMerge},
#line 344 "sp-factory.gperf"
    {"svg:feDistantLight", create_SPFeDistantLight},
#line 316 "sp-factory.gperf"
    {"svg:solidColor",     create_SPSolidColor},
#line 349 "sp-factory.gperf"
    {"svg:feMergeNode",    create_SPFeMergeNode},
#line 345 "sp-factory.gperf"
    {"svg:feFlood",        create_SPFeFlood},
#line 274 "sp-factory.gperf"
    {"svg:desc",           create_SPDesc},
#line 340 "sp-factory.gperf"
    {"svg:feComposite",    create_SPFeComposite},
#line 350 "sp-factory.gperf"
    {"svg:feMorphology",   create_SPFeMorphology},
#line 334 "sp-factory.gperf"
    {"svg:feColorMatrix",  create_SPFeColorMatrix},
#line 300 "sp-factory.gperf"
    {"svg:mask",           create_SPMask},
#line 341 "sp-factory.gperf"
    {"svg:feConvolveMatrix", create_SPFeConvolveMatrix},
#line 343 "sp-factory.gperf"
    {"svg:feDisplacementMap", create_SPFeDisplacementMap},
#line 281 "sp-factory.gperf"
    {"svg:flowPara",       create_SPFlowpara},
#line 306 "sp-factory.gperf"
    {"sodipodi:namedview", create_SPNamedView},
#line 284 "sp-factory.gperf"
    {"svg:flowRegionBreak",  create_SPFlowregionbreak},
#line 355 "sp-factory.gperf"
    {"svg:feTile",         create_SPFeTile},
#line 338 "sp-factory.gperf"
    {"svg:feFuncB",        create_SPFeFuncNode_B},
#line 301 "sp-factory.gperf"
    {"svg:mesh",           create_SPMesh},
#line 356 "sp-factory.gperf"
    {"svg:feTurbulence",   create_SPFeTurbulence},
#line 302 "sp-factory.gperf"
    {"svg:meshpatch",      create_SPMeshpatch},
#line 271 "sp-factory.gperf"
    {"svg:a",              create_SPAnchor},
#line 337 "sp-factory.gperf"
    {"svg:feFuncG",        create_SPFeFuncNode_G},
#line 305 "sp-factory.gperf"
    {"svg:missing-glyph",  create_SPMissingGlyph},
#line 335 "sp-factory.gperf"
    {"svg:feComponentTransfer", create_SPFeComponentTransfer},
#line 289 "sp-factory.gperf"
    {"svg:glyph",          create_SPGlyph},
#line 322 "sp-factory.gperf"
    {"svg:switch",         create_SPSwitch},
#line 352 "sp-factory.gperf"
    {"svg:fePointLight",   create_SPFePointLight},
#line 353 "sp-factory.gperf"
    {"svg:feSpecularLighting", create_SPFeSpecularLighting},
#line 347 "sp-factory.gperf"
    {"svg:feImage",        create_SPFeImage},
#line 314 "sp-factory.gperf"
    {"svg:svg",            create_SPRoot},
#line 273 "sp-factory.gperf"
    {"svg:defs",           create_SPDefs},
#line 293 "sp-factory.gperf"
    {"svg:hatch",          create_SPHatch},
#line 342 "sp-factory.gperf"
    {"svg:feDiffuseLighting", create_SPFeDiffuseLighting},
#line 294 "sp-factory.gperf"
    {"svg:hatchPath",      create_SPHatchPath},
#line 339 "sp-factory.gperf"
    {"svg:feFuncA",        create_SPFeFuncNode_A},
#line 346 "sp-factory.gperf"
    {"svg:feGaussianBlur", create_SPGaussianBlur},
#line 333 "sp-factory.gperf"
    {"svg:feBlend",        create_SPFeBlend}
  };

static const signed char lookup[] =
  {
    -1, -1, -1,  0, -1, -1, -1, -1, -1, -1, -1, -1,  1,  2,
     3, -1, -1,  4, -1, -1, -1, -1, -1, -1,  5,  6, -1, -1,
     7,  8,  9, 10, 11, 12, -1, 13, 14, 15, 16, -1, -1, -1,
    17, 18, 19, -1, 20, 21, 22, -1, 23, 24, 25, 26, -1, -1,
    27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
    41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54,
    55, 56, 57, -1, 58, 59, 60, 61, -1, 62, -1, 63, 64, 65,
    66, -1, 67, 68, 69, 70, 71, 72, 73, -1, 74, -1, -1, 75,
    -1, 76, -1, 77, 78, 79, 80, 81, 82, 83, 84, -1, -1, -1,
    85, 86, 87, 88, -1, 89, -1, 90, -1, -1, 91, -1, -1, -1,
    -1, -1, -1, 92, -1, -1, -1, -1, -1, -1, -1, 93
  };

#ifdef __GNUC__
__inline
#if defined __GNUC_STDC_INLINE__ || defined __GNUC_GNU_INLINE__
__attribute__ ((__gnu_inline__))
#endif
#endif
const struct factory_object *
get_object_create_func (register const char *str, register unsigned int len)
{
  if (len <= MAX_WORD_LENGTH && len >= MIN_WORD_LENGTH)
    {
      register int key = hash (str, len);

      if (key <= MAX_HASH_VALUE && key >= 0)
        {
          register int index = lookup[key];

          if (index >= 0)
            {
              register const char *s = wordlist[index].name;

              if (*str == *s && !strcmp (str + 1, s + 1))
                return &wordlist[index];
            }
        }
    }
  return 0;
}
