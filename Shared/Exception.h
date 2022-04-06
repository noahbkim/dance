#include <stdexcept>
#include <string>
#include <winnt.h>

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
