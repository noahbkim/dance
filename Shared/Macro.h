#pragma once

#include <winnt.h>
#include <exception>
#include <sstream>

#define TRACE(message) { \
	std::wostringstream stream; \
	stream << __FILE__ << "(" << __LINE__ << ") " << message << std::endl; \
	OutputDebugString(stream.str().c_str()); \
}

class ComError : public std::runtime_error
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

#define OKE(call) if (HRESULT result = (call); result != S_OK) { TRACE("caught invalid HRESULT: " << std::hex << result); throw ComError(result); }
#define BETE(call) if (BOOL result = (call); !result) { throw ComError(E_FAIL); }

#define GUARD(call, name) if (HRESULT name = (call); name != S_OK)

#define lengthof(x) (sizeof(x)/sizeof(x[0]))
