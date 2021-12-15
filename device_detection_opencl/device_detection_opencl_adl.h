#pragma once

#include <string>
#include <tuple>
#include <vector>
#include "include/adl_sdk.h"


namespace GlobalFunction {
	std::tuple<bool, std::string> get_devices_json_result(bool pretty_print = false);
}

//namespace adl_device_detection {
//	std::tuple<bool, std::vector<std::tuple<int, std::string>>> get_amd_device_driver_versions(bool pretty_print);
//}

#ifdef __cplusplus
extern "C" {
#endif

__declspec(dllexport) const char* __cdecl open_cl_adl_device_detection_json_result_str(bool pretty_print = false);

#ifdef __cplusplus
}
#endif