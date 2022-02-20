#pragma once

#include "Framework.h"
#include <memory>

/// Deletion struct for a window handle
struct HWNDDeleter
{
	using pointer = HWND;

	/// We're actually in charge of destroying our own window handle once the app is done. This is a good solution if
	/// you don't need to share the pointer, but since we do, we destroy our own window handle manually.
	void operator()(HWND handle) const
	{
		::DestroyWindow(handle);
	}
};

/// Unique pointer for a HWND.
using WindowHandle = std::unique_ptr<HWND, HWNDDeleter>;

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

/// We don't own this so we don't use any memory tricks.
using InstanceHandle = HINSTANCE;

/// Include this because we use it everywhere.
using Microsoft::WRL::ComPtr;
