#include "utils/library_util.hpp"

#ifdef _WIN32
#include <string>
#include <array>
#include <iostream>
#include <windows.h>

#define LOAD_SYMBOL(lib, name) ::plib::core::utils::LoadMethod(lib, #name, name)

namespace plib::core::utils {
namespace {

constexpr auto kMaxPathLong = 32767;

__declspec(noreturn) void FatalError(
		const std::wstring &text,
		DWORD error = 0) {
	const auto lastError = error ? error : GetLastError();
	const auto full = lastError
		? (text + L"\n\nError Code: " + std::to_wstring(lastError))
		: text;
	MessageBoxW(nullptr, full.c_str(), L"Fatal Error", MB_ICONERROR);
	std::abort();
}

void CheckDynamicLibraries() {
	auto exePath = std::array<WCHAR, kMaxPathLong + 1>{ 0 };
	const auto exeLength = GetModuleFileNameW(
		nullptr,
		exePath.data(),
		kMaxPathLong + 1);
	if (!exeLength || exeLength >= kMaxPathLong + 1) {
		FatalError(L"Could not get executable path!");
	}
	const auto exe = std::wstring(exePath.data());
	const auto last1 = exe.find_last_of('\\');
	const auto last2 = exe.find_last_of('/');
	const int last = (std::max)(
		(last1 == std::wstring::npos) ? -1 : static_cast<int>(last1),
		(last2 == std::wstring::npos) ? -1 : static_cast<int>(last2));
	if (last < 0) {
		FatalError(L"Could not get executable directory!");
	}
	const auto search = exe.substr(0, last + 1) + L"*.dll";

	auto findData = WIN32_FIND_DATAW();
	const auto findHandle = FindFirstFileW(search.c_str(), &findData);
	if (findHandle == INVALID_HANDLE_VALUE) {
		const auto error = GetLastError();
		if (error == ERROR_FILE_NOT_FOUND) {
			return;
		}
		FatalError(L"Could not enumerate executable path!", error);
	}

	do {
		if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			continue;
		}
		const auto me = exe.substr(last + 1);
		FatalError(L"Unknown DLL library \"\
" + std::wstring(findData.cFileName) + L"\" found \
in the directory with " + me + L".\n\n\
This may be a virus or a malicious program. \n\n\
Please remove all DLL libraries from this directory:\n\n\
" + exe.substr(0, last) + L"\n\n\
Alternatively, you can move " + me + L" to a new directory.");
	} while (FindNextFileW(findHandle, &findData));
}

BOOL (__stdcall *SetDefaultDllDirectories)(_In_ DWORD DirectoryFlags);

} // namespace

namespace details {

void *LoadMethodRaw(HINSTANCE library, LPCSTR name, WORD id) {
	const auto result = GetProcAddress(library, name);
	return result
		? result
		: id
		? GetProcAddress(library, MAKEINTRESOURCEA(id))
		: nullptr;
}

void ReportLoadFailure(HINSTANCE library, LPCSTR name, DWORD id) {
	constexpr auto kMaxPathLong = 32767;
	auto path = std::array<WCHAR, kMaxPathLong + 1>{ 0 };
	const auto length = GetModuleFileNameW(
		library,
		path.data(),
		kMaxPathLong);
	if (length > 0 && length < kMaxPathLong) {
		if (id) {
			std::wcerr << L"DLL Error: Failed to load '" << name 
			           << L"' from '" << path.data() 
			           << L"' (ID: " << id << L")." << std::endl;
		} else {
			std::wcerr << L"DLL Error: Failed to load '" << name 
			           << L"' from '" << path.data() << L"'." << std::endl;
		}
	} else {
		if (id) {
			std::cerr << "DLL Error: Failed to load '" << name 
			          << "' from _unknown_ (ID: " << id << ")." << std::endl;
		} else {
			std::cerr << "DLL Error: Failed to load '" << name 
			          << "' from _unknown_." << std::endl;
		}
	}
}

} // namespace details

void InitDynamicLibraries() {
	static const auto Inited = [] {
		const auto kernel = LoadLibraryW(L"kernel32.dll");
		if (LOAD_SYMBOL(kernel, SetDefaultDllDirectories)) {
			SetDefaultDllDirectories(LOAD_LIBRARY_SEARCH_SYSTEM32);
		} else {
			CheckDynamicLibraries();
		}
		return true;
	}();
}

HINSTANCE SafeLoadLibrary(LPCWSTR name, bool required) {
	InitDynamicLibraries();

	if (const auto result = HINSTANCE(LoadLibraryW(name))) {
		return result;
	} else if (required) {
		FatalError(L"Could not load required DLL '"
			+ std::wstring(name)
			+ L"'!");
	} else {
		std::wcerr << L"DLL Error: Could not load '" 
		           << name << L"'." << std::endl;
	}
	return nullptr;
}

} // namespace plib::core::utils

#endif // _WIN32