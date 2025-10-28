#pragma once

#include "ofxAEComposition.h"
#include "ofxAELayer.h"
#include "ofxAELayerSource.h"
#include "ofxAEShapeSource.h"
#include "ofxAEVideoSource.h"
#include "ofxAEStillSource.h"
#include "ofxAESequenceSource.h"
#include "ofxAECompositionSource.h"
#include "ofxAESolidSource.h"
#include "ofxAEShapeProp.h"
#include "ofxAEMaskProp.h"
#include "ofxAEPath.h"
#include "ofxAEProperty.h"

namespace ofx { namespace ae {

class Visitor
{
public:
    void setVisitChildren(bool visit) { visit_children_ = visit; }
    void setVisitSources(bool visit) { visit_sources_ = visit; }
    void setVisitProperties(bool visit) { visit_properties_ = visit; }

    bool getVisitChildren() const { return visit_children_; }
    bool getVisitSources() const { return visit_sources_; }
    bool getVisitProperties() const { return visit_properties_; }

    virtual void visit(const Composition& composition);
    virtual void visit(const Layer& layer);
    
    virtual void visit(const LayerSource& source) {}
    virtual void visit(const ShapeSource& source);
    virtual void visit(const VideoSource& source) {}
    virtual void visit(const StillSource& source) {}
    virtual void visit(const SequenceSource& source) {}
    virtual void visit(const CompositionSource& source) {}
    virtual void visit(const SolidSource& source) {}

	virtual void visit(const ShapeData& shape);
    virtual void visit(const GroupData& group);
	virtual void visit(const ShapeDataBase& shape) {}
    virtual void visit(const EllipseData& ellipse) {}
    virtual void visit(const RectangleData& rectangle) {}
    virtual void visit(const PathData& path) {}
    virtual void visit(const PolygonData& polygon) {}
    virtual void visit(const FillData& fill) {}
    virtual void visit(const StrokeData& stroke) {}
    virtual void visit(const MaskAtomData& data) {}

    virtual void visit(const PropertyBase& property) {}
    virtual void visit(const PropertyGroup& group);
    virtual void visit(const PropertyArray& array);

protected:
    virtual void visitChildren(const Composition& composition);
    virtual void visitChildren(const ShapeSource& source);
    virtual void visitChildren(const CompositionSource& source);
    virtual void visitChildren(const ShapeData& shape);
    virtual void visitChildren(const GroupData& group);
    virtual void visitChildren(const PropertyGroup& group);
    virtual void visitChildren(const PropertyArray& array);

private:
    bool visit_children_ = true;
    bool visit_sources_ = true;
    bool visit_properties_ = true;
};

}} // namespace ofx::ae
