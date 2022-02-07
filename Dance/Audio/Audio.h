#pragma once

#include "Framework.h"
#include "Common/Pointer.h"

ComPtr<IMMDevice> getDefaultAudioDevice();
std::wstring getAudioDeviceId(ComPtr<IMMDevice> device);
std::wstring getAudioDeviceFriendlyName(ComPtr<IMMDevice> device);
