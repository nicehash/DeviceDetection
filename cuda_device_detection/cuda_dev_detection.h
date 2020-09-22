// if this is C99
#ifndef __cplusplus
#include <stdbool.h>
#endif

#include <Windows.h>

#ifdef __cplusplus
extern "C" {
#endif

__declspec(dllexport) const char* __cdecl _GetCUDADevices(bool prettyString);

#ifdef __cplusplus
} // extern "C"
#endif