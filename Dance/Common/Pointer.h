#pragma once

#include "Framework.h"
#include <memory>

struct HWNDDeleter
{
	using pointer = HWND;
	void operator()(HWND handle) const
	{
		::DestroyWindow(handle);
	}
};

using WindowHandle = std::unique_ptr<HWND, HWNDDeleter>;

template<typename T>
struct CoTaskDeleter
{
	using pointer = T*;
	void operator()(T* t) const
	{
		::CoTaskMemFree(t);
	}
};

using InstanceHandle = HINSTANCE;  // We don't own this

using Microsoft::WRL::ComPtr;
