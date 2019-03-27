#include "json_helpers.h"

//#define NLOHMANN_JSON

#ifdef NLOHMANN_JSON
#include "json.hpp"
// for convenience
using json = nlohmann::json;

json cuda_dev_conver_to_json(const CudaDevice& d) {
	return json{
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

std::string json_helpers::GetCUDADevicesJsonString(std::vector<CudaDevice> &cudaDevices, std::string driverVersion, std::string errorString, bool nvmlLoaded, bool prettyPrint) {
	json j = {
		{ "NvmlLoaded", nvmlLoaded },
		{ "DriverVersion", driverVersion },
		{ "CudaDevices", json::array() },
		{ "ErrorString", errorString },
	};
	for (const auto &d : cudaDevices) {
		j["CudaDevices"].push_back(cuda_dev_conver_to_json(d));
	}

	if (prettyPrint) {
		return j.dump(4);
	}
	return j.dump();
}
#else // use trivial_json_printer

#include "trivial_json_printer.hpp"

template <>
void WriteValue<CudaDevice>(std::stringstream &ss, CudaDevice d) {
	StartObject(ss);
	AddJSONPropertyAndValue(ss, "DeviceID", d.DeviceID);
	AddJSONPropertyAndValue(ss, "VendorName", d.VendorName);
	AddJSONPropertyAndValue(ss, "DeviceName", d.DeviceName);
	AddJSONPropertyAndValue(ss, "SM_major", d.SM_major);
	AddJSONPropertyAndValue(ss, "SM_minor", d.SM_minor);
	AddJSONPropertyAndValue(ss, "UUID", d.UUID);
	AddJSONPropertyAndValue(ss, "DeviceGlobalMemory", d.DeviceGlobalMemory);
	AddJSONPropertyAndValue(ss, "pciDeviceId", d.pciDeviceId);
	AddJSONPropertyAndValue(ss, "pciSubSystemId", d.pciSubSystemId);
	AddJSONPropertyAndValue(ss, "SMX", d.SMX);
	AddJSONPropertyAndValue(ss, "VendorID", d.VendorID);
	AddJSONPropertyAndValue(ss, "HasMonitorConnected", d.HasMonitorConnected);
	AddJSONPropertyAndValue(ss, "pciBusID", d.pciBusID, false); // FALSE DO NOT TERMINATE WITH COMMA
	EndObject(ss);
}

std::string json_helpers::GetCUDADevicesJsonString(std::vector<CudaDevice> &cudaDevices, std::string driverVersion, std::string errorString, bool nvmlLoaded, bool prettyPrint) {
	std::stringstream ss;
	
	StartObject(ss);
	AddJSONPropertyAndValue(ss, "NvmlLoaded", nvmlLoaded);
	AddJSONPropertyAndValue(ss, "CudaDevices", cudaDevices);
	AddJSONPropertyAndValue(ss, "DriverVersion", driverVersion);
	AddJSONPropertyAndValue(ss, "ErrorString", errorString, false); // FALSE DO NOT TERMINATE WITH COMMA
	EndObject(ss);

	if (prettyPrint) {
		return getPrettyString(ss.str(), 4);
	}
	return ss.str();
}
#endif 