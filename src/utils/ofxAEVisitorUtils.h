#pragma once

#include <vector>
#include <functional>
#include <memory>
#include "ofMain.h"
#include "glm/glm.hpp"
#include "ofxAEVisitor.h"
#include "ofxAEShapeUtils.h"
#include "ofxAEShapeProp.h"

namespace ofx { namespace ae { namespace utils {

template<typename T, typename Predicate>
class FindVisitor : public Visitor {
public:
    explicit FindVisitor(Predicate predicate) : predicate_(predicate) {}
    
    using Visitor::visit;
    
    void visit(const T& item) override {
        if (predicate_(item)) {
            results_.push_back(const_cast<T*>(&item));
        }
        Visitor::visit(item);
    }
    
    const std::vector<T*>& getResults() const {
        return results_;
    }
    
    void clear() {
        results_.clear();
    }

private:
    Predicate predicate_;
    std::vector<T*> results_;
};

template<typename T>
class CollectVisitor : public Visitor {
public:
    using Visitor::visit;
    
    void visit(const T& item) override {
        items_.push_back(const_cast<T*>(&item));
        Visitor::visit(item);
    }
    
    const std::vector<T*>& getItems() const {
        return items_;
    }
    
    void clear() {
        items_.clear();
    }

private:
    std::vector<T*> items_;
};

class PathExtractionVisitor : public Visitor {
public:
    void visit(const Layer& layer) override;
    void visit(const EllipseData& ellipse) override;
    void visit(const RectangleData& rectangle) override;
    void visit(const PolygonData& polygon) override;
    void visit(const FillData& fill) override;
    void visit(const StrokeData& stroke) override;

    const std::deque<ofPath>& getPaths() const { return result_; }

    void clear();

protected:
    void applyTransform(ofPath& path) const;
    void pushTransform(const TransformData& transform);
    void popTransform();

private:
    std::deque<ofPath> result_;
	struct Transform {
		ofMatrix4x4 mat;
		float opacity;
		Transform operator*(const TransformData &t) const {
			return {mat*t.toOf(), opacity*t.opacity};
		}
	};
    std::stack<Transform> transform_;
    ofPath path_;
};

template<typename T>
auto makeFindVisitor(std::function<bool(const T&)> predicate) {
	return FindVisitor<T, std::function<bool(const T&)>>(predicate);
}
template<typename T>
auto makeCollectVisitor() {
	return CollectVisitor<T>();
}

}}} // namespace ofx::ae::utils
