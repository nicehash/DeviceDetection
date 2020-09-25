#include "cuda_device_detection.h"

#include <windows.h>
#include <iostream>
#include <stdexcept>
#include <vector>
#include <optional>
#include <map>
#include <string>
#include <string_view>
#include <cstdint>

#pragma region HELPER structs

struct cuda_nvml_device {
	unsigned int DeviceID = {};
	int pciBusID = {};
	std::string VendorName = {};
	std::string DeviceName = {};
	int SM_major = {};
	int SM_minor = {};
	std::string UUID = {};
	size_t DeviceGlobalMemory = {};
	unsigned long pciDeviceId = {};    //!< The combined 16-bit device id and 16-bit vendor id
	unsigned int pciSubSystemId = {}; //!< The 32-bit Sub System Device ID
	int SMX = {};
	int VendorID = {};
	int HasMonitorConnected = {};
};

struct detection_result {
	int NvmlLoaded = {};
	int NvmlInitialized = {}; // new
	std::string DriverVersion = {};
	std::vector<cuda_nvml_device> CudaDevices = {};
	std::string ErrorString = {};
};

#pragma endregion HELPER structs

#pragma region nlohmann::json

#include "json.hpp"

void to_json(nlohmann::json& j, const cuda_nvml_device& d) {
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

void to_json(nlohmann::json& j, const detection_result& d) {
	j =
	{
		{ "NvmlLoaded", d.NvmlLoaded },
		{ "NvmlInitialized", d.NvmlInitialized},
		{ "DriverVersion", d.DriverVersion },
		{ "CudaDevices", d.CudaDevices },
		{ "ErrorString", d.ErrorString },
	};
}

#pragma endregion nlohmann::json

#pragma region CUDA

#include <cuda.h>
#include <cuda_runtime.h>

#define CUDA_SAFE_CALL(call)								\
do {														\
	cudaError_t err = call;									\
	if (cudaSuccess != err) {								\
		const char * errorString = cudaGetErrorString(err);	\
		fprintf(stderr,										\
			"CUDA error in func '%s' at line %i : %s.\n",	\
			__FUNCTION__, __LINE__, errorString);			\
		throw std::runtime_error(errorString);				\
				}											\
} while (0)

#define PCI_BUS_LEN 64

#pragma endregion CUDA

#pragma region NVML
#include <nvml.h>

#define NVML_NOT_LOADED -1
#define NVML_LOADED 0
#define NVML_DCH_LOADED 1


namespace nvidia_nvml_helper  {
	
	typedef int(*nvml_Init)(void);
	typedef int(*nvml_Shutdown)(void);
	typedef int(*nvml_DeviceGetHandleByPciBusId)(const char*, nvmlDevice_t*);
	typedef int(*nvml_DeviceGetUUID)(nvmlDevice_t, char*, unsigned int);
	typedef nvmlReturn_t(*nvml_DeviceGetPciInfo)(nvmlDevice_t device, nvmlPciInfo_t* pci);
	typedef nvmlReturn_t(*nvml_DeviceGetDisplayActive)(nvmlDevice_t device, nvmlEnableState_t* isActive);
	typedef nvmlReturn_t(*nvml_SystemGetDriverVersion)(char*, unsigned);

	nvml_Init NVMLInit = 0;
	nvml_Shutdown NVMLShutdown = 0;
	nvml_DeviceGetHandleByPciBusId NVMLDeviceGetHandleByPciBusId = 0;
	nvml_DeviceGetUUID NVMLDeviceGetUUID = 0;
	nvml_DeviceGetPciInfo NVMLDeviceGetPciInfo = 0;
	nvml_DeviceGetDisplayActive NVMLDeviceGetDisplayActive = 0;
	nvml_SystemGetDriverVersion NVMLSystemGetDriverVersion = 0;

	using get_path_t = std::optional<std::string>;

	get_path_t _enviorment_dll_path(LPCSTR lpName, std::string_view dllSubPath) {
		char path_buffer[MAX_PATH];
		auto reservedLength = (dllSubPath.length() + 1);
		DWORD ret = GetEnvironmentVariableA(lpName, path_buffer, MAX_PATH - reservedLength);

		// error getting path
		if (ret == 0 || ret >= (MAX_PATH - reservedLength)) return std::nullopt;

		std::string dllPath = std::string(path_buffer);
		dllPath.append(dllSubPath);
		return dllPath;
	}

	get_path_t _nvml_dll_path() {
		return _enviorment_dll_path("ProgramFiles", "\\NVIDIA Corporation\\NVSMI\\nvml.dll");
	}

	get_path_t _dch_nvml_dll_path() {
		return _enviorment_dll_path("windir", "\\System32\\nvml.dll");
	}

	std::tuple<HMODULE, int> _load_NVML() {
		// 'Old' non DCH drivers
		if (auto pathOpt = _nvml_dll_path(); pathOpt.has_value()) {
			HMODULE hmod = LoadLibraryA(pathOpt.value().c_str());
			if (hmod != NULL) return { hmod, NVML_LOADED };
		}
		// DCH drivers
		if (auto pathOpt = _dch_nvml_dll_path(); pathOpt.has_value()) {
			HMODULE hmod = LoadLibraryA(pathOpt.value().c_str());
			if (hmod != NULL) return { hmod, NVML_DCH_LOADED };
		}
		return { NULL, NVML_NOT_LOADED };
	}

	std::tuple<uint16_t, std::string> _get_vendor_id_and_name(nvmlPciInfo_t& nvmlPciInfo) {
		static const std::map<std::uint16_t, std::string> VENDOR_NAMES = {
			{ 0x1043, "ASUS" },
			{ 0x107D, "Leadtek" },
			{ 0x10B0, "Gainward" },
			{ 0x10DE, "NVIDIA" },
			{ 0x1458, "Gigabyte" },
			{ 0x1462, "MSI" },
			{ 0x154B, "PNY" },
			{ 0x1682, "XFX" },
			{ 0x196D, "Club3D" },
			{ 0x19DA, "Zotac" },
			{ 0x19F1, "BFG" },
			{ 0x1ACC, "PoV" },
			{ 0x1B4C, "KFA2" },
			{ 0x3842, "EVGA" },
			{ 0x7377, "Colorful" },
			{ 0x1569, "Palit" },
			{ 0x1849, "ASRock" },
			{ 0x1043, "ASUSTeK" },
			{ 0x148C, "PowerColor" },
			{ 0x1DA2, "Sapphire" },
			{ 0, "" }
		};
		uint16_t vendorId = 0;
		vendorId = nvmlPciInfo.pciDeviceId & 0xFFFF;
		if (vendorId == 0x10DE && nvmlPciInfo.pciSubSystemId) {
			vendorId = nvmlPciInfo.pciSubSystemId & 0xFFFF;
		}
		for (const auto [compareId, name] : VENDOR_NAMES) {
			if (compareId == vendorId) {
				return { vendorId , name };
			}
		}
		return { vendorId , "UNKNOWN" };;
	}

	std::tuple<int, int> SafeNVMLInit() {
		if (auto [hmod, code] = _load_NVML(); hmod != NULL && code != NVML_NOT_LOADED) {
			NVMLInit = (nvml_Init)GetProcAddress(hmod, "nvmlInit_v2");
			NVMLShutdown = (nvml_Shutdown)GetProcAddress(hmod, "nvmlShutdown");
			NVMLDeviceGetHandleByPciBusId = (nvml_DeviceGetHandleByPciBusId)GetProcAddress(hmod, "nvmlDeviceGetHandleByPciBusId_v2");
			NVMLDeviceGetUUID = (nvml_DeviceGetUUID)GetProcAddress(hmod, "nvmlDeviceGetUUID");
			NVMLDeviceGetPciInfo = (nvml_DeviceGetPciInfo)GetProcAddress(hmod, "nvmlDeviceGetPciInfo_v2");
			NVMLDeviceGetDisplayActive = (nvml_DeviceGetDisplayActive)GetProcAddress(hmod, "nvmlDeviceGetDisplayActive");
			NVMLSystemGetDriverVersion = (nvml_SystemGetDriverVersion)GetProcAddress(hmod, "nvmlSystemGetDriverVersion");

			if (NVMLInit == NULL) {
				return { -2, code };
			}
			int initStatus = NVMLInit();
			// first 0 is OK
			return { initStatus, code };
		}
		else {
			return { -1, code };
		}
	}

	void SetCudaDeviceAttributes(const char* pciBusID, cuda_nvml_device& cudaDevice) {

		// serial stuff
		nvmlPciInfo_t pciInfo;
		nvmlDevice_t device_t;
		char uuid[NVML_DEVICE_UUID_BUFFER_SIZE];
		nvmlEnableState_t isEnabled = NVML_FEATURE_DISABLED;

		if (NVMLDeviceGetHandleByPciBusId && NVMLDeviceGetHandleByPciBusId(pciBusID, &device_t) == 0) {
			// init uuid
			if (NVMLDeviceGetUUID && NVMLDeviceGetUUID(device_t, uuid, NVML_DEVICE_UUID_BUFFER_SIZE) == 0) {
				cudaDevice.UUID = uuid;
			}
			// init device info
			if (NVMLDeviceGetPciInfo && NVMLDeviceGetPciInfo(device_t, &pciInfo) == 0) {
				const auto [vendorId, vendorName] = _get_vendor_id_and_name(pciInfo);
				cudaDevice.VendorName = vendorName;
				cudaDevice.pciDeviceId = pciInfo.pciDeviceId;
				cudaDevice.pciSubSystemId = pciInfo.pciSubSystemId;
				cudaDevice.VendorID = vendorId;
			}

			if (NVMLDeviceGetDisplayActive && NVMLDeviceGetDisplayActive(device_t, &isEnabled) == 0) {
				cudaDevice.HasMonitorConnected = isEnabled;
			}
		}
	}

	std::string GetDriverVersionSafe() {
		char buffer[NVML_SYSTEM_DRIVER_VERSION_BUFFER_SIZE];
		if (NVMLSystemGetDriverVersion && NVML_SUCCESS == NVMLSystemGetDriverVersion(buffer, NVML_SYSTEM_DRIVER_VERSION_BUFFER_SIZE)) {
			return std::string(buffer);
		}
		return "";
	}


	void SafeNVMLShutdown() {
		if (NVMLShutdown) NVMLShutdown();
	}
}

#pragma endregion NVML


std::tuple<bool, std::string> cuda_nvml_detection::get_devices_json_result(bool prettyPrint) {
	bool ok = true;
	detection_result result;
	
	try {
		int device_count;
		CUDA_SAFE_CALL(cudaGetDeviceCount(&device_count));

		const auto [init_code, nvml_load_code] = nvidia_nvml_helper::SafeNVMLInit();
		result.NvmlLoaded = nvml_load_code;
		result.NvmlInitialized = init_code;

		for (int i = 0; i < device_count; ++i) {
			cuda_nvml_device cudaDevice;

			cudaDeviceProp props;
			CUDA_SAFE_CALL(cudaGetDeviceProperties(&props, i));
			char pciBusID[PCI_BUS_LEN];
			CUDA_SAFE_CALL(cudaDeviceGetPCIBusId(pciBusID, PCI_BUS_LEN, i));

			// init device CUDA info
			cudaDevice.DeviceID = i;
			cudaDevice.pciBusID = props.pciBusID;
			cudaDevice.DeviceName = props.name;
			cudaDevice.SM_major = props.major;
			cudaDevice.SM_minor = props.minor;
			cudaDevice.DeviceGlobalMemory = props.totalGlobalMem;
			cudaDevice.SMX = props.multiProcessorCount;

			// init NVML info [UUID, VendorName, pciDeviceId, pciSubSystemId, VendorID, HasMonitorConnected]
			nvidia_nvml_helper::SetCudaDeviceAttributes(pciBusID, cudaDevice);

			result.CudaDevices.push_back(cudaDevice);
		}
		result.DriverVersion = nvidia_nvml_helper::GetDriverVersionSafe();
		nvidia_nvml_helper::SafeNVMLShutdown();
	}
	catch (std::runtime_error& err) {
		result.ErrorString = err.what();
		//_errorMsgs.push_back(err.what());
		ok = false;
	}

	nlohmann::json j;
	to_json(j, result);
	auto json_str = prettyPrint ? j.dump(4) : j.dump();
	return { ok, json_str };
}



const char* cuda_device_detection_json_result_str(bool prettyString)
{
	static std::string ret;
	const auto [ok, json_str] = cuda_nvml_detection::get_devices_json_result(prettyString);
	ret = json_str;
	return ret.c_str();
}

char* _GetCUDADevices(bool prettyString) {
	return (char*)cuda_device_detection_json_result_str(prettyString);
}


BOOL WINAPI DllMain(
	HINSTANCE hinstDLL,  // handle to DLL module
	DWORD fdwReason,     // reason for calling function
	LPVOID lpReserved)  // reserved
{
	// Perform actions based on the reason for calling.
	switch (fdwReason)
	{
	case DLL_PROCESS_ATTACH:
		// Initialize once for each new process.
		// Return FALSE to fail DLL load.
		break;

	case DLL_THREAD_ATTACH:
		// Do thread-specific initialization.
		break;

	case DLL_THREAD_DETACH:
		// Do thread-specific cleanup.
		break;

	case DLL_PROCESS_DETACH:
		// Perform any necessary cleanup.
		break;
	}
	return TRUE;  // Successful DLL_PROCESS_ATTACH.
}
