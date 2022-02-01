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

using InstanceHandle = HINSTANCE;  // We don't own this
using WindowHandle = std::unique_ptr<HWND, HWNDDeleter>;

using Microsoft::WRL::ComPtr;
