// if this is C99
#ifndef __cplusplus
#include <stdbool.h>
#endif


#ifdef __cplusplus
extern "C" {
#endif

#ifdef __linux__ 
	// useNvmlFallback is Win32 only just add it so function prototypes are same
    __attribute__((visibility("default"))) char* _GetCUDADevices(bool prettyString, bool useNvmlFallback);
#elif _WIN32
#include <Windows.h>
    __declspec(dllexport) char* __cdecl _GetCUDADevices(bool prettyString, bool useNvmlFallback);
#else
#error "Unknown platform not supported"
#endif

#ifdef __cplusplus
}
#endif