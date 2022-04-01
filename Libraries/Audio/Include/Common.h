#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define NOMCX
#define NOSERVICE
#define NOHELP

#include <string>

#include <wrl.h>
#include <wrl/client.h>
#include <commctrl.h>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <audiopolicy.h>
#include <mmreg.h>
#include <functiondiscoverykeys_devpkey.h> 

#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "Winmm.lib")

#include "Macro.h"
#include "Pointer.h"
