#include "Capture.h"

ComPtr<IMMDevice> getDefaultDevice()
{
    ComPtr<IMMDeviceEnumerator> deviceEnumerator;
    OKE(CoCreateInstance(
        __uuidof(MMDeviceEnumerator),
        NULL,
        CLSCTX_ALL,
        __uuidof(IMMDeviceEnumerator),
        (void**)deviceEnumerator.ReleaseAndGetAddressOf()));

    ComPtr<IMMDevice> device;
    OKE(deviceEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, device.ReleaseAndGetAddressOf()));
    return device;
}

std::wstring getDeviceId(ComPtr<IMMDevice> device)
{
    LPWSTR id = NULL;
    OKE(device->GetId(&id));
    std::wstring result(id);

    ::CoTaskMemFree(id);
    return result;
}

std::wstring getDeviceFriendlyName(ComPtr<IMMDevice> device)
{
    ComPtr<IPropertyStore> propertyStore;
    OKE(device->OpenPropertyStore(STGM_READ, propertyStore.ReleaseAndGetAddressOf()));

    PROPVARIANT name;
    ::PropVariantInit(&name);
    OKE(propertyStore->GetValue(PKEY_Device_FriendlyName, &name));
    std::wstring result(name.pwszVal);
    ::PropVariantClear(&name);

    return result;
}
