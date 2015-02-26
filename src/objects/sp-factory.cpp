/*
 * Factory for SPObject tree
 *
 * Authors:
 *   Markus Engel
 *
 * Copyright (C) 2013 Authors
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "objects/sp-factory.h"

// primary
#include "box3d.h"
#include "box3d-side.h"
#include "color-profile.h"
#include "persp3d.h"
#include "objects/sp-anchor.h"
#include "objects/sp-clippath.h"
#include "objects/sp-defs.h"
#include "objects/sp-desc.h"
#include "objects/sp-ellipse.h"
#include "objects/sp-filter.h"
#include "objects/sp-flowdiv.h"
#include "objects/sp-flowregion.h"
#include "objects/sp-flowtext.h"
#include "objects/sp-font.h"
#include "objects/sp-font-face.h"
#include "objects/sp-glyph.h"
#include "objects/sp-guide.h"
#include "objects/sp-hatch.h"
#include "objects/sp-hatch-path.h"
#include "objects/sp-image.h"
#include "objects/sp-item-group.h"
#include "objects/sp-line.h"
#include "objects/sp-linear-gradient.h"
#include "objects/sp-marker.h"
#include "objects/sp-mask.h"
#include "objects/sp-mesh-gradient.h"
#include "objects/sp-mesh-patch.h"
#include "objects/sp-mesh-row.h"
#include "objects/sp-metadata.h"
#include "objects/sp-missing-glyph.h"
#include "objects/sp-namedview.h"
#include "objects/sp-object.h"
#include "objects/sp-offset.h"
#include "objects/sp-path.h"
#include "objects/sp-pattern.h"
#include "objects/sp-polygon.h"
#include "objects/sp-polyline.h"
#include "objects/sp-radial-gradient.h"
#include "objects/sp-rect.h"
#include "objects/sp-root.h"
#include "objects/sp-script.h"
#include "objects/sp-solid-color.h"
#include "objects/sp-spiral.h"
#include "objects/sp-star.h"
#include "objects/sp-stop.h"
#include "objects/sp-string.h"
#include "objects/sp-style-elem.h"
#include "objects/sp-switch.h"
#include "objects/sp-symbol.h"
#include "objects/sp-tag.h"
#include "objects/sp-tag-use.h"
#include "objects/sp-text.h"
#include "objects/sp-textpath.h"
#include "objects/sp-title.h"
#include "objects/sp-tref.h"
#include "objects/sp-tspan.h"
#include "objects/sp-use.h"
#include "live_effects/lpeobject.h"

// filters
#include "objects/filters/blend.h"
#include "objects/filters/colormatrix.h"
#include "objects/filters/componenttransfer.h"
#include "objects/filters/componenttransfer-funcnode.h"
#include "objects/filters/composite.h"
#include "objects/filters/convolvematrix.h"
#include "objects/filters/diffuselighting.h"
#include "objects/filters/displacementmap.h"
#include "objects/filters/distantlight.h"
#include "objects/filters/flood.h"
#include "objects/filters/gaussian-blur.h"
#include "objects/filters/image.h"
#include "objects/filters/merge.h"
#include "objects/filters/mergenode.h"
#include "objects/filters/morphology.h"
#include "objects/filters/offset.h"
#include "objects/filters/pointlight.h"
#include "objects/filters/specularlighting.h"
#include "objects/filters/spotlight.h"
#include "objects/filters/tile.h"
#include "objects/filters/turbulence.h"

SPObject *SPFactory::createObject(std::string const& id)
{
    SPObject *ret = NULL;

    if (id == "inkscape:box3d")
        ret = new SPBox3D;
    else if (id == "inkscape:box3dside")
        ret = new Box3DSide;
    else if (id == "svg:color-profile")
        ret = new Inkscape::ColorProfile;
    else if (id == "inkscape:persp3d")
        ret = new Persp3D;
    else if (id == "svg:a")
        ret = new SPAnchor;
    else if (id == "svg:clipPath")
        ret = new SPClipPath;
    else if (id == "svg:defs")
        ret = new SPDefs;
    else if (id == "svg:desc")
        ret = new SPDesc;
    else if (id == "svg:ellipse") {
        SPGenericEllipse *e = new SPGenericEllipse;
        e->type = SP_GENERIC_ELLIPSE_ELLIPSE;
        ret = e;
    } else if (id == "svg:circle") {
        SPGenericEllipse *c = new SPGenericEllipse;
        c->type = SP_GENERIC_ELLIPSE_CIRCLE;
        ret = c;
    } else if (id == "arc") {
        SPGenericEllipse *a = new SPGenericEllipse;
        a->type = SP_GENERIC_ELLIPSE_ARC;
        ret = a;
    }
    else if (id == "svg:filter")
        ret = new SPFilter;
    else if (id == "svg:flowDiv")
        ret = new SPFlowdiv;
    else if (id == "svg:flowSpan")
        ret = new SPFlowtspan;
    else if (id == "svg:flowPara")
        ret = new SPFlowpara;
    else if (id == "svg:flowLine")
        ret = new SPFlowline;
    else if (id == "svg:flowRegionBreak")
        ret = new SPFlowregionbreak;
    else if (id == "svg:flowRegion")
        ret = new SPFlowregion;
    else if (id == "svg:flowRegionExclude")
        ret = new SPFlowregionExclude;
    else if (id == "svg:flowRoot")
        ret = new SPFlowtext;
    else if (id == "svg:font")
        ret = new SPFont;
    else if (id == "svg:font-face")
        ret = new SPFontFace;
    else if (id == "svg:glyph")
        ret = new SPGlyph;
    else if (id == "sodipodi:guide")
        ret = new SPGuide;
    else if (id == "svg:hatch")
        ret = new SPHatch;
    else if (id == "svg:hatchPath")
        ret = new SPHatchPath;
    else if (id == "svg:image")
        ret = new SPImage;
    else if (id == "svg:g")
        ret = new SPGroup;
    else if (id == "svg:line")
        ret = new SPLine;
    else if (id == "svg:linearGradient")
        ret = new SPLinearGradient;
    else if (id == "svg:marker")
        ret = new SPMarker;
    else if (id == "svg:mask")
        ret = new SPMask;
    else if (id == "svg:meshGradient")
        ret = new SPMeshGradient;
    else if (id == "svg:meshPatch")
        ret = new SPMeshPatch;
    else if (id == "svg:meshRow")
        ret = new SPMeshRow;
    else if (id == "svg:metadata")
        ret = new SPMetadata;
    else if (id == "svg:missing-glyph")
        ret = new SPMissingGlyph;
    else if (id == "sodipodi:namedview")
        ret = new SPNamedView;
    else if (id == "inkscape:offset")
        ret = new SPOffset;
    else if (id == "svg:path")
        ret = new SPPath;
    else if (id == "svg:pattern")
        ret = new SPPattern;
    else if (id == "svg:polygon")
        ret = new SPPolygon;
    else if (id == "svg:polyline")
        ret = new SPPolyLine;
    else if (id == "svg:radialGradient")
        ret = new SPRadialGradient;
    else if (id == "svg:rect")
        ret = new SPRect;
    else if (id == "svg:svg")
        ret = new SPRoot;
    else if (id == "svg:script")
        ret = new SPScript;
    else if (id == "svg:solidColor")
        ret = new SPSolidColor;
    else if (id == "spiral")
        ret = new SPSpiral;
    else if (id == "star")
        ret = new SPStar;
    else if (id == "svg:stop")
        ret = new SPStop;
    else if (id == "string")
        ret = new SPString;
    else if (id == "svg:style")
        ret = new SPStyleElem;
    else if (id == "svg:switch")
        ret = new SPSwitch;
    else if (id == "svg:symbol")
        ret = new SPSymbol;
    else if (id == "inkscape:tag")
        ret = new SPTag;
    else if (id == "inkscape:tagref")
        ret = new SPTagUse;
    else if (id == "svg:text")
        ret = new SPText;
    else if (id == "svg:title")
        ret = new SPTitle;
    else if (id == "svg:tref")
        ret = new SPTRef;
    else if (id == "svg:tspan")
        ret = new SPTSpan;
    else if (id == "svg:textPath")
        ret = new SPTextPath;
    else if (id == "svg:use")
        ret = new SPUse;
    else if (id == "inkscape:path-effect")
        ret = new LivePathEffectObject;


    // filters
    else if (id == "svg:feBlend")
        ret = new SPFeBlend;
    else if (id == "svg:feColorMatrix")
        ret = new SPFeColorMatrix;
    else if (id == "svg:feComponentTransfer")
        ret = new SPFeComponentTransfer;
    else if (id == "svg:feFuncR")
        ret = new SPFeFuncNode(SPFeFuncNode::R);
    else if (id == "svg:feFuncG")
        ret = new SPFeFuncNode(SPFeFuncNode::G);
    else if (id == "svg:feFuncB")
        ret = new SPFeFuncNode(SPFeFuncNode::B);
    else if (id == "svg:feFuncA")
        ret = new SPFeFuncNode(SPFeFuncNode::A);
    else if (id == "svg:feComposite")
        ret = new SPFeComposite;
    else if (id == "svg:feConvolveMatrix")
        ret = new SPFeConvolveMatrix;
    else if (id == "svg:feDiffuseLighting")
        ret = new SPFeDiffuseLighting;
    else if (id == "svg:feDisplacementMap")
        ret = new SPFeDisplacementMap;
    else if (id == "svg:feDistantLight")
        ret = new SPFeDistantLight;
    else if (id == "svg:feFlood")
        ret = new SPFeFlood;
    else if (id == "svg:feGaussianBlur")
        ret = new SPGaussianBlur;
    else if (id == "svg:feImage")
        ret = new SPFeImage;
    else if (id == "svg:feMerge")
        ret = new SPFeMerge;
    else if (id == "svg:feMergeNode")
        ret = new SPFeMergeNode;
    else if (id == "svg:feMorphology")
        ret = new SPFeMorphology;
    else if (id == "svg:feOffset")
        ret = new SPFeOffset;
    else if (id == "svg:fePointLight")
        ret = new SPFePointLight;
    else if (id == "svg:feSpecularLighting")
        ret = new SPFeSpecularLighting;
    else if (id == "svg:feSpotLight")
        ret = new SPFeSpotLight;
    else if (id == "svg:feTile")
        ret = new SPFeTile;
    else if (id == "svg:feTurbulence")
        ret = new SPFeTurbulence;
    else if (id == "inkscape:grid")
        ret = new SPObject; // TODO wtf
    else if (id == "rdf:RDF") // no SP node yet
        {}
    else if (id == "inkscape:clipboard") // SP node not necessary
        {}
    else if (id.empty()) // comments
        {}
    else {
        fprintf(stderr, "WARNING: unknown type: %s", id.c_str());
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

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
