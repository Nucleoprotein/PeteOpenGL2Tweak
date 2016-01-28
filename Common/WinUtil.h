#pragma once

#include <string>
#include <memory>
#include "Windows.h"
#include "tchar.h"
#include "Types.h"
#include "Defines.h"

#include <Shlobj.h>
#pragma comment(lib, "shell32.lib")

template <typename T>
T clamp(const T& n, const T& lower, const T& upper) {
	return n <= lower ? lower : n >= upper ? upper : n;
}

inline HMODULE& CurrentModule()
{
	static HMODULE hModule = 0;
	if (!hModule)
		GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (LPCTSTR)&hModule, &hModule);
	return hModule;
}

inline void RemoveFileSpec(char* path)
{
	if (path[strlen(path) - 4] == '.')
	{
		char* p = strrchr(path, '\\');
		if (p) *p = '\0';
	}
}

inline void RemoveFileSpec(wchar_t* path)
{
	if (path[wcslen(path) - 4] == L'.')
	{
		wchar_t* p = wcsrchr(path, L'\\');
		if (p) *p = L'\0';
	}
}

inline void RemovePath(char* path)
{
	char* p = strrchr(path, '\\');
	if (p)
	{
		strcpy_s(path, strlen(p), p + 1);
		path[strlen(p)] = '\0';
	}
}

inline void RemovePath(wchar_t* path)
{
	wchar_t* p = wcsrchr(path, L'\\');
	if (p)
	{
		wcscpy_s(path, wcslen(p), p + 1);
		path[wcslen(p)] = L'\0';
	}
}

inline void StringPathAppend(std::string* path, const std::string& more)
{
	if (path->back() != DIR_SEP_CHR)
		path->push_back(DIR_SEP_CHR);

	path->append(more);
}

inline void StringPathAppend(std::wstring* path, const std::wstring& more)
{
	if (path->back() != _T(DIR_SEP_CHR))
		path->push_back(_T(DIR_SEP_CHR));

	path->append(more);
}

inline bool FileExists(const std::string& path)
{
	FILE* filep;
	if (fopen_s(&filep, path.c_str(), "rb") == 0)
	{
		fclose(filep);
		return true;
	}
	return false;
}

inline std::string ModulePathA(HMODULE hModule)
{
	std::unique_ptr<char[]> buffer(new char[MAX_PATH]);
	if (GetModuleFileNameA(hModule, buffer.get(), MAX_PATH))
	{
		std::string res = buffer.get();
		return std::move(res);
	}
	return "";
}

inline std::wstring ModulePathW(HMODULE hModule)
{
	std::unique_ptr<wchar_t[]> buffer(new wchar_t[MAX_PATH]);
	if (GetModuleFileNameW(hModule, buffer.get(), MAX_PATH))
	{
		std::wstring res = buffer.get();
		return std::move(res);
	}
	return L"";
}

inline std::string ModuleDirectoryA(HMODULE hModule)
{
	std::unique_ptr<char[]> buffer(new char[MAX_PATH]);
	if (GetModuleFileNameA(hModule, buffer.get(), MAX_PATH))
	{
		RemoveFileSpec(buffer.get());

		std::string res = buffer.get();
		return std::move(res);
	}
	return "";
}

inline std::wstring ModuleDirectoryW(HMODULE hModule)
{
	std::unique_ptr<wchar_t[]> buffer(new wchar_t[MAX_PATH]);
	if (GetModuleFileNameW(hModule, buffer.get(), MAX_PATH))
	{
		RemoveFileSpec(buffer.get());

		std::wstring res = buffer.get();
		return std::move(res);
	}
	return L"";
}

inline std::string ModuleNameA(HMODULE hModule)
{
	std::unique_ptr<char[]> buffer(new char[MAX_PATH]);
	if (GetModuleFileNameA(hModule, buffer.get(), MAX_PATH))
	{
		RemovePath(buffer.get());

		std::string res = buffer.get();
		return std::move(res);
	}
	return "";
}

inline std::wstring ModuleNameW(HMODULE hModule)
{
	std::unique_ptr<wchar_t[]> buffer(new wchar_t[MAX_PATH]);
	if (GetModuleFileNameW(hModule, buffer.get(), MAX_PATH))
	{
		RemovePath(buffer.get());

		std::wstring res = buffer.get();
		return std::move(res);
	}
	return L"";
}

inline std::string CreateSystemModulePath(const char* module_name)
{
	std::unique_ptr<char[]> buffer(new char[MAX_PATH]);
	GetSystemDirectoryA(buffer.get(), MAX_PATH);

	std::string path(buffer.get());
	StringPathAppend(&path, module_name);

	return std::move(path);
}

inline std::wstring CreateSystemModulePath(const wchar_t* module_name)
{
	std::unique_ptr<wchar_t[]> buffer(new wchar_t[MAX_PATH]);
	GetSystemDirectoryW(buffer.get(), MAX_PATH);

	std::wstring path(buffer.get());
	StringPathAppend(&path, module_name);

	return std::move(path);
}

inline HMODULE LoadLibraryFromDirectory(const char* dir, const char* module_name)
{
	std::string path(dir);
	StringPathAppend(&path, module_name);
	return LoadLibraryA(path.c_str());
}

inline HMODULE LoadLibraryFromSystemDir(const char* module_name)
{
	std::unique_ptr<char[]> buffer(new char[MAX_PATH]);
	GetSystemDirectoryA(buffer.get(), MAX_PATH);
	return LoadLibraryFromDirectory(buffer.get(), module_name);
}

inline std::string FullPathFromPath(const std::string& path)
{
	// check it already full path
	if (path[1] == ':' || path[1] == '?')
		return path;

	std::string out_path = ModuleDirectoryA(CurrentModule());

	if (out_path.back() != '\\' && path.front() != '\\')
		out_path.push_back('\\');

	std::string res = out_path + path;
	return std::move(res);
}

inline void StringToGUID(GUID* id, const std::string& szBuf)
{
	const char* p = szBuf.c_str();
	if (strchr(p, '{')) p++;

	u32 d1;
	s32 d2, d3;
	s32 b[8];

	if (sscanf_s(p, "%08lX-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X",
		&d1, &d2, &d3, &b[0], &b[1], &b[2], &b[3], &b[4], &b[5], &b[6], &b[7]) != 11)
	{
		*id = GUID_NULL;
		return;
	}

	id->Data1 = d1;
	id->Data2 = (u16)d2;
	id->Data3 = (u16)d3;

	for (int i = 0; i < 8; ++i)
		id->Data4[i] = (u8)b[i];
}

inline void StringToGUID(GUID* id, const std::wstring& szBuf)
{
	const wchar_t* p = szBuf.c_str();
	if (wcschr(p, L'{')) p++;

	u32 d1;
	s32 d2, d3;
	s32 b[8];

	if (swscanf_s(p, L"%08lX-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X",
		&d1, &d2, &d3, &b[0], &b[1], &b[2], &b[3], &b[4], &b[5], &b[6], &b[7]) != 11)
	{
		*id = GUID_NULL;
		return;
	}

	id->Data1 = d1;
	id->Data2 = (u16)d2;
	id->Data3 = (u16)d3;

	for (int i = 0; i < 8; ++i)
		id->Data4[i] = (u8)b[i];
}

inline std::string GUIDtoStringA(const GUID &g)
{
	std::unique_ptr<char[]> buffer(new char[40]);
	sprintf_s(buffer.get(), 40, "{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}",
		g.Data1, g.Data2, g.Data3, g.Data4[0], g.Data4[1], g.Data4[2], g.Data4[3], g.Data4[4], g.Data4[5], g.Data4[6], g.Data4[7]);

	return buffer.get();
}

inline std::wstring GUIDtoStringW(const GUID &g)
{
	std::unique_ptr<wchar_t[]> buffer(new wchar_t[40]);
	swprintf_s(buffer.get(), 40, L"{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}",
		g.Data1, g.Data2, g.Data3, g.Data4[0], g.Data4[1], g.Data4[2], g.Data4[3], g.Data4[4], g.Data4[5], g.Data4[6], g.Data4[7]);

	return buffer.get();
}


