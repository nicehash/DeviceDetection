#pragma once

#include <string>
#include <vector>
#include "CudaDevice.h"

namespace json_helpers
{
	std::string GetCUDADevicesJsonString(std::vector<CudaDevice> &cudaDevices, std::string driverVersion, std::string errorString, int nvmlLoaded, bool prettyPrint = false);
}

