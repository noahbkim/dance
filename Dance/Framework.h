// header.h : include file for standard system include files,
// or project specific include files
//

#pragma once

#include "targetversion.h"

#define WIN32_LEAN_AND_MEAN

// Use the C++ standard templated min/max
#define NOMINMAX

// DirectX apps don't need GDI
#define NODRAWTEXT
#define NOGDI
#define NOBITMAP

// Include <mcx.h> if you need this
#define NOMCX

// Include <winsvc.h> if you need this
#define NOSERVICE

// WinHelp is deprecated
#define NOHELP

#include <windows.h>
#include <windowsx.h>
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

#include <wingdi.h>
#include <dxgi1_2.h>
#include <d3d11_4.h>
#include <d3dcompiler.h>

#include <pix.h>
#include <directxcolors.h>
#include <directxmath.h>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <exception>
#include <iterator>
#include <memory>
#include <stdexcept>

#include <wrl.h>
#include <wrl/client.h>

#include <CommCtrl.h>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <audiopolicy.h>
#include <mmreg.h>
#include <functiondiscoverykeys_devpkey.h> 

#include <string>
#include <iostream>
#include <sstream>

#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "gdi32.lib")

#include "Resource.h"

#ifdef _DEBUG
#include <dxgidebug.h>
#endif

#define TRACE(message) { \
	std::wostringstream stream; \
	stream << __FILE__ << "(" << __LINE__ << ") " << message << std::endl; \
	OutputDebugStringW(stream.str().c_str()); \
}

#define SUCCEEDING(call) if (HRESULT result = (call); result != S_OK) { TRACE("caught invalid HRESULT: " << std::hex << result); exit(result); }
#define OK(call) if (HRESULT result = (call); result != S_OK) { TRACE("caught invalid HRESULT: " << std::hex << result); return result; }
#define BET(call) if (BOOL result = (call); !result) { TRACE("caught invalid BOOL: " << result); return E_FAIL; }

#define GUARD(call, name) if (HRESULT name = (call); name != result)

inline void ThrowIfFailed(HRESULT hr)
{
    if (FAILED(hr))
    {
        throw std::exception();
    }
}
