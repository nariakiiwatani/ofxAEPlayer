#pragma once

namespace json {
template<typename T> bool extract(const ofJson &data, std::string key, T &dst) {
	if(!data.contains(key)) return false;
	dst = data[key].get<T>();
	return true;
}

}
