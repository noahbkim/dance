#pragma once

#include "Common.h"
#include "Pointer.h"

namespace Dance::Audio
{
	/// Returns a pointer to the default audio output device. CoInitialize must be invoked prior to this method, which
	/// invokes CoCreateInstance to instantiate the device interface.
	/// 
	/// @exception ComError if instantiating the device fails.
	/// @returns a ComPtr to the current default audio output device.
	ComPtr<IMMDevice> getDefaultAudioDevice();

	/// Get the ID from an audio device pointer.
	/// 
	/// @param device expects a ComPtr to a system audio device.
	/// @exception ComError if instantiating the device fails.
	/// @returns a std::wstring of the device unique ID.
	std::wstring getAudioDeviceId(ComPtr<IMMDevice> device);

	/// Get the name of the audio device the user would see.
	/// 
	/// @param device expects a ComPtr to a system audio device.
	/// @exception ComError if instantiating the device fails.
	/// @returns a std::wstring of the device name.
	std::wstring getAudioDeviceFriendlyName(ComPtr<IMMDevice> device);
}
