#pragma once

#include <string>
#include <tuple>

namespace opencl_device_detection {
	std::tuple<bool, std::string> get_devices_json_result(bool prettyPrint = false);
}

#ifdef __cplusplus
extern "C" {
#endif

__declspec(dllexport) const char* __cdecl open_cl_device_detection_json_result_str(bool prettyPrint = false);
// old interface for backward compatibility 
__declspec(dllexport) char* __cdecl _GetOpenCLDevices(bool prettyString);

#ifdef __cplusplus
}
#endif