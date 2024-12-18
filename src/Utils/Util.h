#pragma once
#include <algorithm>
#include <string>
#include <vector>

namespace jse {

template <typename T>
inline bool FindVector(std::vector<T> const& vec, T const& value) {
    return std::find(vec.begin(), vec.end(), value) != vec.end();
}

inline std::string ReplaceStr(const std::string& str, char const& from, char const& to) {
    std::string fixedStr = str;
    std::replace(fixedStr.begin(), fixedStr.end(), from, to);
    return fixedStr;
}

inline std::string ReplaceWinPath(const std::string& path) { return ReplaceStr(path, '\\', '/'); }


} // namespace jse