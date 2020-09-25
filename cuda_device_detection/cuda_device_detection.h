#pragma once

#include <string>
#include <tuple>

namespace cuda_nvml_detection {
	std::tuple<bool, std::string> get_devices_json_result(bool prettyPrint = false);
}

#ifdef __cplusplus
extern "C" {
#endif

__declspec(dllexport) const char* __cdecl cuda_device_detection_json_result_str(bool prettyPrint = false);
// old interface for backward compatibility 
__declspec(dllexport) char* __cdecl _GetCUDADevices(bool prettyString); 

#ifdef __cplusplus
} // extern "C"
#endif

