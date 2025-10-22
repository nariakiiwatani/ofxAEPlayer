#include "ofxAEMaskProp.h"
#include "ofxAEVisitor.h"

// Add interpolation specialization for MaskShapeData
namespace ofx { namespace ae { namespace interpolation {

template<>
MaskShapeData linear(const MaskShapeData& value_a, const MaskShapeData& value_b, float ratio) {
    return MaskShapeData::lerp(value_a, value_b, ratio);
}

}}} // namespace ofx::ae::interpolation

namespace ofx { namespace ae {

// MaskShapeData implementation
ofPath MaskShapeData::toOfPath() const {
    ofPath path;
    
    if (vertices.empty()) {
        return path;
    }
    
    if (vertices.size() == 1) {
        // Single point
        path.moveTo(vertices[0]);
        return path;
    }
    
    // Start the path
    path.moveTo(vertices[0]);
    
    for (size_t i = 0; i < vertices.size(); ++i) {
        size_t nextIndex = (i + 1) % vertices.size();
        
        // For closed paths, connect to next vertex; for open paths, stop at last vertex
        if (!closed && nextIndex == 0) {
            break;
        }
        
        const glm::vec2& currentVertex = vertices[i];
        const glm::vec2& nextVertex = vertices[nextIndex];
        
        // Get tangents for current and next vertices
        glm::vec2 outTangent = (i < outTangents.size()) ? outTangents[i] : glm::vec2(0, 0);
        glm::vec2 inTangent = (nextIndex < inTangents.size()) ? inTangents[nextIndex] : glm::vec2(0, 0);
        
        // Check if we need to create a curve or just a line
        bool hasCurve = (glm::length(outTangent) > 0.001f || glm::length(inTangent) > 0.001f);
        
        if (hasCurve) {
            // Create Bezier curve
            glm::vec2 cp1 = currentVertex + outTangent;
            glm::vec2 cp2 = nextVertex + inTangent;
            path.bezierTo(cp1, cp2, nextVertex);
        } else {
            // Create straight line
            path.lineTo(nextVertex);
        }
    }
    
    if (closed) {
        path.close();
    }
    
    return path;
}

MaskShapeData MaskShapeData::lerp(const MaskShapeData& a, const MaskShapeData& b, float t) {
    MaskShapeData result;
    
    // Handle different vertex counts by using the larger one and padding with last vertex
    size_t maxVertices = std::max(a.vertices.size(), b.vertices.size());
    
    if (maxVertices == 0) {
        return result;
    }
    
    result.vertices.reserve(maxVertices);
    result.inTangents.reserve(maxVertices);
    result.outTangents.reserve(maxVertices);
    
    for (size_t i = 0; i < maxVertices; ++i) {
        // Get vertices with padding
        glm::vec2 vertexA = (i < a.vertices.size()) ? a.vertices[i] : a.vertices.back();
        glm::vec2 vertexB = (i < b.vertices.size()) ? b.vertices[i] : b.vertices.back();
        
        glm::vec2 inTangentA = (i < a.inTangents.size()) ? a.inTangents[i] : glm::vec2(0, 0);
        glm::vec2 inTangentB = (i < b.inTangents.size()) ? b.inTangents[i] : glm::vec2(0, 0);
        
        glm::vec2 outTangentA = (i < a.outTangents.size()) ? a.outTangents[i] : glm::vec2(0, 0);
        glm::vec2 outTangentB = (i < b.outTangents.size()) ? b.outTangents[i] : glm::vec2(0, 0);
        
        // Interpolate
        result.vertices.push_back(glm::mix(vertexA, vertexB, t));
        result.inTangents.push_back(glm::mix(inTangentA, inTangentB, t));
        result.outTangents.push_back(glm::mix(outTangentA, outTangentB, t));
    }
    
    result.closed = a.closed || b.closed; // Keep closed if either is closed
    
    return result;
}

// MaskAtomData implementation
void MaskAtomData::accept(Visitor& visitor) const {
    visitor.visit(*this);
}

// PathShapeProp implementation
MaskShapeData PathShapeProp::parse(const ofJson &json) const {
    MaskShapeData shape;
    
    if (json.is_null() || !json.is_object()) {
        return shape;
    }
    
    // Parse vertices
    if (json.contains("vertices") && json["vertices"].is_array()) {
        const auto& verticesArray = json["vertices"];
        shape.vertices.reserve(verticesArray.size());
        
        for (const auto& vertex : verticesArray) {
            if (vertex.is_array() && vertex.size() >= 2) {
                shape.vertices.emplace_back(vertex[0].get<float>(), vertex[1].get<float>());
            }
        }
    }
    
    // Parse inTangents
    if (json.contains("inTangents") && json["inTangents"].is_array()) {
        const auto& tangentsArray = json["inTangents"];
        shape.inTangents.reserve(tangentsArray.size());
        
        for (const auto& tangent : tangentsArray) {
            if (tangent.is_array() && tangent.size() >= 2) {
                shape.inTangents.emplace_back(tangent[0].get<float>(), tangent[1].get<float>());
            }
        }
    }
    
    // Parse outTangents
    if (json.contains("outTangents") && json["outTangents"].is_array()) {
        const auto& tangentsArray = json["outTangents"];
        shape.outTangents.reserve(tangentsArray.size());
        
        for (const auto& tangent : tangentsArray) {
            if (tangent.is_array() && tangent.size() >= 2) {
                shape.outTangents.emplace_back(tangent[0].get<float>(), tangent[1].get<float>());
            }
        }
    }
    
    // Parse closed flag
    if (json.contains("closed")) {
        shape.closed = json["closed"].get<bool>();
    }
    
    return shape;
}

// MaskShapeData interpolation operators
MaskShapeData MaskShapeData::operator+(const MaskShapeData& other) const {
    MaskShapeData result;
    
    size_t maxVertices = std::max(vertices.size(), other.vertices.size());
    result.vertices.resize(maxVertices);
    result.inTangents.resize(maxVertices);
    result.outTangents.resize(maxVertices);
    
    for(size_t i = 0; i < maxVertices; i++) {
        if(i < vertices.size() && i < other.vertices.size()) {
            result.vertices[i] = vertices[i] + other.vertices[i];
            result.inTangents[i] = inTangents[i] + other.inTangents[i];
            result.outTangents[i] = outTangents[i] + other.outTangents[i];
        } else if(i < vertices.size()) {
            result.vertices[i] = vertices[i];
            result.inTangents[i] = inTangents[i];
            result.outTangents[i] = outTangents[i];
        } else {
            result.vertices[i] = other.vertices[i];
            result.inTangents[i] = other.inTangents[i];
            result.outTangents[i] = other.outTangents[i];
        }
    }
    
    result.closed = closed || other.closed;
    return result;
}

MaskShapeData MaskShapeData::operator-(const MaskShapeData& other) const {
    MaskShapeData result;
    
    size_t maxVertices = std::max(vertices.size(), other.vertices.size());
    result.vertices.resize(maxVertices);
    result.inTangents.resize(maxVertices);
    result.outTangents.resize(maxVertices);
    
    for(size_t i = 0; i < maxVertices; i++) {
        if(i < vertices.size() && i < other.vertices.size()) {
            result.vertices[i] = vertices[i] - other.vertices[i];
            result.inTangents[i] = inTangents[i] - other.inTangents[i];
            result.outTangents[i] = outTangents[i] - other.outTangents[i];
        } else if(i < vertices.size()) {
            result.vertices[i] = vertices[i];
            result.inTangents[i] = inTangents[i];
            result.outTangents[i] = outTangents[i];
        } else {
            result.vertices[i] = -other.vertices[i];
            result.inTangents[i] = -other.inTangents[i];
            result.outTangents[i] = -other.outTangents[i];
        }
    }
    
    result.closed = closed;
    return result;
}

MaskShapeData MaskShapeData::operator*(float ratio) const {
    MaskShapeData result = *this;
    
    for(auto& vertex : result.vertices) {
        vertex *= ratio;
    }
    for(auto& tangent : result.inTangents) {
        tangent *= ratio;
    }
    for(auto& tangent : result.outTangents) {
        tangent *= ratio;
    }
    
    return result;
}

// MaskAtomProp implementation
MaskAtomProp::MaskAtomProp() {
    registerProperty<PathShapeProp>("/shape");
    registerProperty<VecProp<2>>("/feather");
    registerProperty<PercentProp>("/opacity");
    registerProperty<FloatProp>("/offset");
    
    registerExtractor<MaskAtomData>([this](MaskAtomData& atom) -> bool {
        bool success = true;
        
        if (!getProperty<PathShapeProp>("/shape")->tryExtract(atom.shape)) {
            ofLogWarning("PropertyExtraction") << "Failed to extract mask shape, using default";
            atom.shape = MaskShapeData();
            success = false;
        }
        
        if (!getProperty<VecProp<2>>("/feather")->tryExtract(atom.feather)) {
            ofLogWarning("PropertyExtraction") << "Failed to extract mask feather, using default";
            atom.feather = glm::vec2(0.0f, 0.0f);
            success = false;
        }
        
        if (!getProperty<FloatProp>("/opacity")->tryExtract(atom.opacity)) {
            ofLogWarning("PropertyExtraction") << "Failed to extract mask opacity, using default";
            atom.opacity = 100.0f;
            success = false;
        }
        
        if (!getProperty<FloatProp>("/offset")->tryExtract(atom.offset)) {
            ofLogWarning("PropertyExtraction") << "Failed to extract mask offset, using default";
            atom.offset = 0.0f;
            success = false;
        }
        
        return success;
    });
}

void MaskAtomProp::accept(Visitor& visitor) {
    PropertyGroup::accept(visitor);
}

MaskProp::MaskProp() {
    registerExtractor<std::vector<MaskAtomData>>([this](std::vector<MaskAtomData>& masks) -> bool {
		masks.clear();
		masks.reserve(properties_.size());

		bool success = true;

		for (const auto& prop : properties_) {
			if (auto maskAtomProp = dynamic_cast<const MaskAtomProp*>(prop.get())) {
				MaskAtomData atom;
				if (maskAtomProp->tryExtract(atom)) {
					masks.push_back(atom);
				} else {
					ofLogWarning("PropertyExtraction") << "Failed to extract mask atom, skipping";
					success = false;
				}
			} else {
				masks.push_back(MaskAtomData());
			}
		}

		return success;
    });
}

void MaskProp::setup(const ofJson &base, const ofJson &keyframes) {
    clear();
    
    if (!base.is_array()) {
        return;
    }
    
    for (size_t i = 0; i < base.size(); ++i) {
        const auto& atomBase = base[i];
        
        if (atomBase.is_null()) {
            addProperty<PropertyBase>();
            continue;
        }
        ofJson atomKeyframes = ofJson::object();
        if (keyframes.is_array() && i < keyframes.size() && !keyframes[i].is_null()) {
            atomKeyframes = keyframes[i];
        }
        
        setupMaskAtom(atomBase, atomKeyframes);
    }
}

void MaskProp::setupMaskAtom(const ofJson &atomBase, const ofJson &atomKeyframes) {
    auto maskAtom = addProperty<MaskAtomProp>();
    
    if (atomBase.contains("atom")) {
        const auto& atom = atomBase["atom"];
        
        // Setup shape property
        ofJson shapeBase = atom.contains("shape") ? atom["shape"] : ofJson::object();
        ofJson shapeKeyframes = ofJson::object();
        
        if (atomKeyframes.contains("atom") && atomKeyframes["atom"].contains("shape")) {
            shapeKeyframes = atomKeyframes["atom"]["shape"];
        }
        
        maskAtom->getProperty<PathShapeProp>("/shape")->setup(shapeBase, shapeKeyframes);
        
        // Setup other properties
        ofJson featherBase = atom.contains("feather") ? atom["feather"] : ofJson::array({0, 0});
        float opacityBase = atom.contains("opacity") ? atom["opacity"].get<float>() : 100.0f;
        float offsetBase = atom.contains("offset") ? atom["offset"].get<float>() : 0.0f;
        
        maskAtom->getProperty<VecProp<2>>("/feather")->setup(featherBase, ofJson{});
        maskAtom->getProperty<FloatProp>("/opacity")->setup(opacityBase, ofJson{});
        maskAtom->getProperty<FloatProp>("/offset")->setup(offsetBase, ofJson{});
    }
}

void MaskProp::accept(Visitor& visitor) {
    PropertyArray::accept(visitor);
}

}} // namespace ofx::ae
