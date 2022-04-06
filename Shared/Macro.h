#pragma once

#include <winnt.h>
#include <sstream>

#include "Exception.h"

#define TRACE(message) { \
	std::wostringstream stream; \
	stream << __FILE__ << "(" << __LINE__ << ") " << message << std::endl; \
	::OutputDebugString(stream.str().c_str()); \
}

#define OK(call) if (HRESULT result = (call); result != S_OK) { TRACE("caught invalid HRESULT: " << std::hex << result); return result; }
#define BET(call) if (BOOL result = (call); !result) { TRACE("caught invalid BOOL: " << result); return E_FAIL; }

#define OKE(call) if (HRESULT result = (call); result != S_OK) { TRACE("caught invalid HRESULT: " << std::hex << result); throw ComError(result); }
#define BETE(call) if (BOOL result = (call); !result) { throw ComError(E_FAIL); }

#define GUARD(call, name) if (HRESULT name = (call); name != S_OK)

#define lengthof(x) (sizeof(x)/sizeof(x[0]))
