#include "json_helpers.h"

#include "json.hpp"
// for convenience
using json = nlohmann::json;

void to_json(nlohmann::json& j, const CudaDevice& d) {
	j =
	{
		{ "DeviceID", d.DeviceID },
		{ "VendorName", d.VendorName },
		{ "DeviceName", d.DeviceName },
		{ "SM_major", d.SM_major },
		{ "SM_minor", d.SM_minor },
		{ "UUID", d.UUID },
		{ "DeviceGlobalMemory", d.DeviceGlobalMemory },
		{ "pciDeviceId", d.pciDeviceId },
		{ "pciSubSystemId", d.pciSubSystemId },
		{ "SMX", d.SMX },
		{ "VendorID", d.VendorID },
		{ "HasMonitorConnected", d.HasMonitorConnected },
		{ "pciBusID", d.pciBusID },
	};
}

std::string json_helpers::GetCUDADevicesJsonString(std::vector<CudaDevice> &cudaDevices, std::string driverVersion, std::string errorString, bool nvmlLoaded, bool nvmlLoadedFallback, bool prettyPrint) {
	json j = {
		{ "NvmlLoaded", nvmlLoaded },
		{ "NvmlLoadedFallback", nvmlLoadedFallback },
		{ "DriverVersion", driverVersion },
		{ "CudaDevices", cudaDevices },
		{ "ErrorString", errorString },
	};

	if (prettyPrint) {
		return j.dump(4);
	}
	return j.dump();
}
