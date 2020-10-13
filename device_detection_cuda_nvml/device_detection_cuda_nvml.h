#pragma once

#include <string>
#include <tuple>

namespace cuda_nvml_detection {
	std::tuple<bool, std::string> get_devices_json_result(bool pretty_print = false);
}

#ifdef __cplusplus
extern "C" {
#endif

__declspec(dllexport) const char* __cdecl cuda_device_detection_json_result_str(bool pretty_print = false);

#ifdef __cplusplus
} // extern "C"
#endif

