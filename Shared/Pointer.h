#pragma once

#include <combaseapi.h>
#include <wrl.h>

/// Deletion struct for memory allocated by CoTaskMemAlloc.
template<typename T>
struct CoTaskDeleter
{
	using pointer = T*;

	/// Free the allocated memory.
	void operator()(T* t) const
	{
		::CoTaskMemFree(t);
	}
};

/// Include this because we use it everywhere.
using Microsoft::WRL::ComPtr;

struct LibraryDeleter
{
	using Pointer = HMODULE;

	void operator()(HMODULE instance) const
	{
		::FreeLibrary(instance);
	}
};

using LibraryHandle = std::unique_ptr<HINSTANCE__, LibraryDeleter>;
