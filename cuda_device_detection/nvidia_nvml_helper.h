#pragma once

#include <map>
#include <string>
#include <cstdint>
#include <tuple>

#include <nvml.h>
#include "CudaDevice.h"

#define OLD_STANDARD_DRIVERS 1
#define DCH_DRIVERS 2

class nvidia_nvml_helper
{
private:
	nvidia_nvml_helper(); // no instances
public:
	static std::tuple<int, int> SafeNVMLInit();
	// set UUID, VendorID and VendorName
	static void SetCudaDeviceAttributes(const char *pciBusID, CudaDevice &cudaDevice);
	static void SafeNVMLShutdown();

	static std::string GetDriverVersionSafe();

private:
	// helpers
	static std::map<std::uint16_t, std::string> _VENDOR_NAMES;
	static std::uint16_t getVendorId(nvmlPciInfo_t &nvmlPciInfo);
	static std::string getVendorString(nvmlPciInfo_t &nvmlPciInfo);
};

