#pragma once

#include "targetversion.h"

#define WIN32_LEAN_AND_MEAN

// Use the C++ standard templated min/max
#define NOMINMAX

// DirectX apps don't need GDI
// #define NODRAWTEXT
// #define NOGDI
// #define NOBITMAP

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
#include <d2d1_2.h>
#include <d2d1_2helper.h>
#include <dcomp.h>
#include <d3dcompiler.h>

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dcomp.lib")
#pragma comment(lib, "D3DCompiler.lib")

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

class ComError: public std::runtime_error
{
public:
    ComError(HRESULT result)
        : result(result)
        , std::runtime_error("unexpected HRESULT " + std::to_string(result)) {}

    ComError(HRESULT result, const std::string& what)
        : result(result)
        , std::runtime_error(what) {}

private:
    const HRESULT result;
};

#define OK(call) if (HRESULT result = (call); result != S_OK) { TRACE("caught invalid HRESULT: " << std::hex << result); return result; }
#define BET(call) if (BOOL result = (call); !result) { TRACE("caught invalid BOOL: " << result); return E_FAIL; }

#define OKE(call) if (HRESULT result = (call); result != S_OK) { throw ComError(result); }
#define BETE(call) if (BOOL result = (call); !result) { throw ComError(E_FAIL); }

#define GUARD(call, name) if (HRESULT name = (call); name != S_OK)

inline void ThrowIfFailed(HRESULT hr)
{
    if (FAILED(hr))
    {
        throw std::exception();
    }
}
