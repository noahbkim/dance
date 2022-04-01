#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define NOMCX
#define NOSERVICE
#define NOHELP

#include <dxgi1_2.h>
#include <d3d11_4.h>
#include <d3dcompiler.h>
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "D3DCompiler.lib")

#ifdef _DEBUG
#include <dxgidebug.h>
#endif

#include "Macro.h"
#include "Pointer.h"
