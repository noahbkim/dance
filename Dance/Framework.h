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
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

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

#include <mmdeviceapi.h>
#include <audioclient.h>
#include <audiopolicy.h>
#include <mmreg.h>
#include <functiondiscoverykeys_devpkey.h> 

#include <string>
#include <iostream>
#include <sstream>

#ifdef _DEBUG
#include <dxgidebug.h>
#endif

#define TRACE(message) { \
	std::wostringstream stream; \
	stream << __FILE__ << "(" << __LINE__ << ") " << message << std::endl; \
	OutputDebugStringW(stream.str().c_str()); \
}

#define SUCCEEDING(call) if (HRESULT result = call) { TRACE("caught invalid HRESULT: " << result); exit(result); }
#define GUARD(call, name) if (HRESULT name = call)

inline void ThrowIfFailed(HRESULT hr)
{
    if (FAILED(hr))
    {
        throw std::exception();
    }
}
