/** 
 * @Author: running-code-pp 3320996652@qq.com
 * @Date: 2025-11-01 23:20:52
 * @LastEditors: running-code-pp 3320996652@qq.com
 * @LastEditTime: 2025-11-01 23:21:40
 * @FilePath: \plib\src\core\include\utils\library_uitl.hpp
 * @Description: 
 * @Copyright: Copyright (c) 2025 by ${git_name}, All Rights Reserved.
 */
#ifndef PLIB_CORE_UTILS_LIBRARY_UTIL_HPP_
#define PLIB_CORE_UTILS_LIBRARY_UTIL_HPP_
#include <windows.h>

// We try to keep this module free of external dependencies.

namespace plib::core::utils {
namespace details {

[[nodiscard]] void *LoadMethodRaw(HINSTANCE library, LPCSTR name, WORD id);
void ReportLoadFailure(HINSTANCE library, LPCSTR name, DWORD id);

} // namespace details

void InitDynamicLibraries();

HINSTANCE SafeLoadLibrary(LPCWSTR name, bool required = false);

template <typename Function>
bool LoadMethod(HINSTANCE library, LPCSTR name, Function &f, WORD id = 0) {
	if (!library) {
		return false;
	} else if (const auto ptr = details::LoadMethodRaw(library, name, id)) {
		f = reinterpret_cast<Function>(ptr);
		return true;
	}
	f = nullptr;
	details::ReportLoadFailure(library, name, id);
	return false;
}

} // namespace plib::core::utils

#endif // PLIB_CORE_UTILS_LIBRARY_UTIL_HPP_