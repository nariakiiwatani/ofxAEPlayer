#include "ofxAEVisitor.h"

namespace ofx { namespace ae {

void Visitor::visit(const Composition& composition) {
    if (visit_children_) {
        visitChildren(composition);
    }
}

void Visitor::visit(const Layer& layer) {
	if (visit_sources_) {
		const LayerSource* source = layer.getSource();
		if (source) {
			// Dispatch to specific source type
			switch (source->getSourceType()) {
				case LayerSource::SHAPE: {
					if (const ShapeSource* shapeSource = dynamic_cast<const ShapeSource*>(source)) {
						visit(*shapeSource);
					}
					break;
				}
				case LayerSource::VIDEO: {
					if (const VideoSource* videoSource = dynamic_cast<const VideoSource*>(source)) {
						visit(*videoSource);
					}
					break;
				}
				case LayerSource::STILL: {
					if (const StillSource* stillSource = dynamic_cast<const StillSource*>(source)) {
						visit(*stillSource);
					}
					break;
				}
				case LayerSource::SEQUENCE: {
					if (const SequenceSource* seqSource = dynamic_cast<const SequenceSource*>(source)) {
						visit(*seqSource);
					}
					break;
				}
				case LayerSource::COMPOSITION: {
					if (const CompositionSource* compSource = dynamic_cast<const CompositionSource*>(source)) {
						visit(*compSource);
					}
					break;
				}
				case LayerSource::SOLID: {
					if (const SolidSource* solidSource = dynamic_cast<const SolidSource*>(source)) {
						visit(*solidSource);
					}
					break;
				}
				default:
					// Visit as generic LayerSource for unknown types
					visit(*source);
					break;
			}
		}
	}
}

void Visitor::visit(const ShapeData& shape) {
    if (visit_children_) {
        visitChildren(shape);
    }
}

void Visitor::visit(const GroupData& group) {
    if (visit_children_) {
        visitChildren(group);
    }
}

void Visitor::visit(const PropertyGroup& group) {
    if (visit_properties_ && visit_children_) {
        visitChildren(group);
    }
}

void Visitor::visit(const PropertyArray& array) {
    if (visit_properties_ && visit_children_) {
        visitChildren(array);
    }
}

void Visitor::visit(const ShapeSource& source) {
    if (visit_children_) {
        visitChildren(source);
    }
}

void Visitor::visitChildren(const Composition& composition) {
    if (!visit_children_) return;
    
    // Visit all layers in the composition
    auto layers = composition.getLayers();
    for (const auto& layer : layers) {
        visit(*layer);
    }
}

void Visitor::visitChildren(const ShapeSource& source) {
    if (!visit_children_) return;
    
    // Extract shape data and visit if available
    ShapeData shapeData;
    if (source.tryExtract(shapeData)) {
        visit(shapeData);
    }
}

void Visitor::visitChildren(const CompositionSource& source) {
    // CompositionSource contains a nested composition
    // This would require access to the internal composition, which may not be publicly accessible
    // Implementation would depend on CompositionSource providing access to its composition
}

void Visitor::visitChildren(const ShapeData& shape) {
    if (!visit_children_) return;
    
    // Visit all shape data elements
    for (const auto& shapePtr : shape.data) {
        if (shapePtr) {
            // Dispatch to specific shape data type
            if (const EllipseData* ellipse = dynamic_cast<const EllipseData*>(shapePtr.get())) {
                visit(*ellipse);
            }
            else if (const RectangleData* rectangle = dynamic_cast<const RectangleData*>(shapePtr.get())) {
                visit(*rectangle);
            }
            else if (const PolygonData* polygon = dynamic_cast<const PolygonData*>(shapePtr.get())) {
                visit(*polygon);
            }
            else if (const FillData* fill = dynamic_cast<const FillData*>(shapePtr.get())) {
                visit(*fill);
            }
            else if (const StrokeData* stroke = dynamic_cast<const StrokeData*>(shapePtr.get())) {
                visit(*stroke);
            }
            else if (const GroupData* group = dynamic_cast<const GroupData*>(shapePtr.get())) {
                visit(*group);
            }
            else if (const ShapeData* nestedShape = dynamic_cast<const ShapeData*>(shapePtr.get())) {
                visit(*nestedShape);
            }
        }
    }
}

void Visitor::visitChildren(const GroupData& group) {
    // GroupData inherits from ShapeData, so delegate to ShapeData traversal
    visitChildren(static_cast<const ShapeData&>(group));
}

void Visitor::visitChildren(const PropertyGroup& group) {
    if (!visit_properties_ || !visit_children_) return;
    
    // PropertyGroup doesn't provide public access to its properties
    // This would require extending PropertyGroup to provide iteration capabilities
    // or making this visitor a friend class
}

void Visitor::visitChildren(const PropertyArray& array) {
    if (!visit_properties_ || !visit_children_) return;
    
    // PropertyArray doesn't provide public iteration over properties
    // This would require extending PropertyArray to provide iteration capabilities
    // or making this visitor a friend class
}

}} // namespace ofx::ae
