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

namespace ofx { namespace ae {

template<typename T, typename Predicate>
class FindVisitor : public Visitor
{
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
class CollectVisitor : public Visitor
{
public:
    using Visitor::visit;
    
    void visit(const T &item) override {
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

class PathExtractionVisitor : public Visitor
{
public:
	PathExtractionVisitor();
	PathExtractionVisitor(const GroupData &group);
	~PathExtractionVisitor()=default;
    void visit(const EllipseData &ellipse) override;
    void visit(const RectangleData &rectangle) override;
    void visit(const PolygonData &polygon) override;
    void visit(const PathData &pathData) override;
    void visit(const FillData &fill) override;
    void visit(const StrokeData &stroke) override;
	void visit(const GroupData &group) override;

	struct RenderItem {
		ofMatrix4x4 transform=ofMatrix4x4::newIdentityMatrix();
		float opacity=1;
		BlendMode blend_mode=BlendMode::NORMAL;
		virtual ofRectangle getBB() const=0;
		virtual void draw(float alpha=1) const =0;
	};
	struct RenderPathItem : public RenderItem {
		RenderPathItem(const ofPath &p):path(p) {
		}
		void draw(float alpha=1) const;
		ofRectangle bounding_box;
		ofRectangle getBB() const;
		ofPath path;
	};
	struct RenderGroupItem : public RenderItem {
		std::deque<std::shared_ptr<RenderItem>> item;
		void draw(float alpha=1) const;
		ofRectangle getBB() const;

		bool needFbo() const;
		mutable ofFbo fbo;
	};
	const RenderGroupItem& getRenderer() const { return renderer_; }
	const ofPath& getPath() const { return path_; }
	const ofRectangle& getBoundingBox() const { return bounding_box_; }

private:
	ofPath path_{};
	ofRectangle bounding_box_;
	RenderGroupItem renderer_;
	RenderPathItem createPathItem(const ofPath &path) const {
		RenderPathItem ret(path);
		ret.transform = renderer_.transform;
		ret.opacity = renderer_.opacity;
		ret.blend_mode = renderer_.blend_mode;
		return ret;
	}
};

class RenderItemExtractionVisitor : public Visitor
{
public:
	void visit(const GroupData &group) override;
	ofPath getAccumulatedPath() const { return path_; }
private:
	ofPath path_;
	ofMatrix4x4 transform_;
	float opacity_=1;
	BlendMode blend_mode_=BlendMode::NORMAL;
};

template<typename T>
auto makeFindVisitor(std::function<bool(const T&)> predicate) {
	return FindVisitor<T, std::function<bool(const T&)>>(predicate);
}
template<typename T>
auto makeCollectVisitor() {
	return CollectVisitor<T>();
}

}} // namespace ofx::ae
