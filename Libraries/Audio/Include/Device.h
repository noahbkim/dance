#pragma once

#include "Common.h"

ComPtr<IMMDevice> getDefaultAudioDevice();
std::wstring getAudioDeviceId(ComPtr<IMMDevice> device);
std::wstring getAudioDeviceFriendlyName(ComPtr<IMMDevice> device);
