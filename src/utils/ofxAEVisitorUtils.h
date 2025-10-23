#pragma once

#include <vector>
#include <functional>
#include <memory>
#include <stack>
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
	struct Context {
		ofPath path;
		ofMatrix4x4 mat;
		float opacity;
		Context():
			path{},
			mat(ofMatrix4x4::newIdentityMatrix()),
			opacity(1)
		{
			path.setStrokeWidth(0);
			path.setFilled(false);
		}

		void transform(const TransformData &t) {
			mat = t.toOf()*mat;
			opacity = t.opacity*opacity;
		}
	};

	PathExtractionVisitor();
	~PathExtractionVisitor()=default;
    void visit(const Layer& layer) override;
    void visit(const EllipseData& ellipse) override;
    void visit(const RectangleData& rectangle) override;
    void visit(const PolygonData& polygon) override;
    void visit(const PathData& pathData) override;
    void visit(const FillData& fill) override;
    void visit(const StrokeData& stroke) override;
	void visit(const GroupData& group) override;

    const std::deque<ofPath>& getPaths() const { return result_; }
	const ofPath& getPlainPath() const { return getContext().path; }

    void clear();

protected:
    void applyTransform(ofPath& path) const;

protected:
	void pushContext();
	void popContext();
	const Context& getContext() const;
	Context& getContext();

private:
    std::deque<ofPath> result_;
    std::stack<Context> ctx_;
};

class BlendModeAwarePathVisitor : public PathExtractionVisitor {
public:
	BlendModeAwarePathVisitor();
	~BlendModeAwarePathVisitor() = default;
	
	void visit(const GroupData& group) override;
	void drawGroup(const GroupData& group, int width, int height);
	
private:
	void drawGroupContents(const GroupData& group);
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
