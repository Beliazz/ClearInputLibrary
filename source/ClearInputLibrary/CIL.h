#pragma once

#ifdef RAWINPUT_EXPORTS
#define CIL_API __declspec(dllexport)
#else
#define CIL_API __declspec(dllimport)
#endif

#pragma warning(disable: 4251)

#include "targetver.h"

// Windows-Headerdateien:
#include <windows.h>
#include <WindowsX.h>

// TODO: Hier auf zusätzliche Header, die das Programm erfordert, verweisen.
#include <string>
#include <map>
#include <memory>
#include <vector>
#include <assert.h>
#include <ctype.h>
#include <stdlib.h>

#include "CILKeys.h"
#include "CILHashedString.h"
#include "CILHandler.h"
#include "CILInput.h"
#include "CILRawInput.h"