// if this is C99
#ifndef __cplusplus
#include <stdbool.h>
#endif


#ifdef __cplusplus
extern "C" {
#endif

#ifdef __linux__ 
    __attribute__((visibility("default"))) char* _GetCUDADevices(bool prettyString);
#elif _WIN32
#include <Windows.h>
    __declspec(dllexport) char* __cdecl _GetCUDADevices(bool prettyString);
#else
#error "Unknown platform not supported"
#endif

#ifdef __cplusplus
}
#endif