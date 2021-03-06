// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"
#pragma comment(lib, "Ws2_32.lib")
#define WIN32_LEAN_AND_MEAN    // Exclude rarely-used stuff from Windows headers
#define _WINSOCK_DEPRECATED_NO_WARNINGS

// Windows Header Files:
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <io.h>
#include <winsock2.h>
#include <time.h>
#include <new.h>

#include "sync.h"
#include "logger.h"
#include "FeedInterface.h"
#include "bases/SynteticBase.h"

#define COPY_STR(dest,src) strncpy(dest,src,sizeof(dest)-1); dest[sizeof(dest)-1]=0;
#define COUNTOF(arr) (sizeof(arr)/sizeof(arr[0]))
//+------------------------------------------------------------------+


// TODO: reference additional headers your program requires here
