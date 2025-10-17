#pragma once

namespace ofx { namespace ae {
class Composition;
class Layer;
class LayerSource;
class ShapeSource;
class ContentVisitor {
public:
	virtual void visit(const Composition&) {}
	virtual void visit(const Layer&) {}
	virtual void visit(const LayerSource&) {}
	virtual void visit(const ShapeSource&) {}
};
}} // namespace ofx::ae
