#pragma once

#include "Target.h"

#define WIN32_LEAN_AND_MEAN

// Use the C++ standard templated min/max
#define NOMINMAX

// Include <mcx.h> if you need this
#define NOMCX

// Include <winsvc.h> if you need this
#define NOSERVICE

// WinHelp is deprecated
#define NOHELP

// Core windows includes
#include <windows.h>
#include <windowsx.h>

// Graphics
#include <wingdi.h>
#include <dxgi1_2.h>
#include <d3d11_4.h>
#include <d2d1_2.h>
#include <d2d1_2helper.h>
#include <dcomp.h>
#include <d3dcompiler.h>
#include <dwmapi.h>
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dcomp.lib")
#pragma comment(lib, "D3DCompiler.lib")
#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "dxguid.lib")

// Audio
#include <wrl.h>
#include <wrl/client.h>
#include <CommCtrl.h>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <audiopolicy.h>
#include <mmreg.h>
#include <functiondiscoverykeys_devpkey.h> 
#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "Winmm.lib")

// Useful stuff we use frequently
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <exception>
#include <memory>
#include <string>
#include <vector>
#include <iostream>
#include <sstream>

// IDs
#include "Resource.h"

#ifdef _DEBUG
#include <dxgidebug.h>
#endif

// Shared headers
#include "Macro.h"
#include "Pointer.h"

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

/// We don't own this so we don't use any memory tricks.
using InstanceHandle = HINSTANCE;
