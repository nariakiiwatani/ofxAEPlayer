#pragma once

#include <string>
#include <memory>

namespace ofx { namespace ae {
class LayerSource;
class LayerSourceFactory {
public:
	static std::unique_ptr<LayerSource> createSourceOfType(std::string type);
};

}} // namespace ofx::ae
