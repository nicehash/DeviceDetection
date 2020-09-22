#include "nvidia_nvml_helper.h"

#include <windows.h>

typedef int(*nvml_Init)(void);
typedef int(*nvml_Shutdown)(void);
typedef int(*nvml_DeviceGetHandleByPciBusId)(const char *, nvmlDevice_t *);
typedef int(*nvml_DeviceGetUUID)(nvmlDevice_t, char*, unsigned int);
typedef nvmlReturn_t(*nvml_DeviceGetPciInfo)(nvmlDevice_t device, nvmlPciInfo_t *pci);
typedef nvmlReturn_t(*nvml_DeviceGetDisplayActive)(nvmlDevice_t device, nvmlEnableState_t* isActive);
typedef nvmlReturn_t (*nvml_SystemGetDriverVersion)(char *, unsigned);

nvml_Init NVMLInit = 0;
nvml_Shutdown NVMLShutdown = 0;
nvml_DeviceGetHandleByPciBusId NVMLDeviceGetHandleByPciBusId = 0;
nvml_DeviceGetUUID NVMLDeviceGetUUID = 0;
nvml_DeviceGetPciInfo NVMLDeviceGetPciInfo = 0;
nvml_DeviceGetDisplayActive NVMLDeviceGetDisplayActive = 0;
nvml_SystemGetDriverVersion NVMLSystemGetDriverVersion = 0;



bool nvidia_nvml_helper::SafeNVMLInit() {
	char path_buffer[MAX_PATH];
	DWORD ret = GetEnvironmentVariableA("ProgramFiles", path_buffer, MAX_PATH - 36);
	if (ret == 0 || ret >= (MAX_PATH - 36))
	{
		// error getting program files path
		return false;
	}

	strcat(path_buffer, "\\NVIDIA Corporation\\NVSMI\\nvml.dll");

	HMODULE hmod = LoadLibraryA(path_buffer);
	if (hmod == NULL) return false;

	NVMLInit = (nvml_Init)GetProcAddress(hmod, "nvmlInit_v2");
	NVMLShutdown = (nvml_Shutdown)GetProcAddress(hmod, "nvmlShutdown");
	NVMLDeviceGetHandleByPciBusId = (nvml_DeviceGetHandleByPciBusId)GetProcAddress(hmod, "nvmlDeviceGetHandleByPciBusId_v2");
	NVMLDeviceGetUUID = (nvml_DeviceGetUUID)GetProcAddress(hmod, "nvmlDeviceGetUUID");
	NVMLDeviceGetPciInfo = (nvml_DeviceGetPciInfo)GetProcAddress(hmod, "nvmlDeviceGetPciInfo_v2");
	NVMLDeviceGetDisplayActive = (nvml_DeviceGetDisplayActive)GetProcAddress(hmod, "nvmlDeviceGetDisplayActive");
	NVMLSystemGetDriverVersion = (nvml_SystemGetDriverVersion)GetProcAddress(hmod, "nvmlSystemGetDriverVersion");

	int initStatus = -1;
	if (NVMLInit) {
		initStatus = NVMLInit();
	}
	return NVML_SUCCESS == initStatus;
}

bool nvidia_nvml_helper::SafeNVMLInitFallback() {
	char path_buffer[MAX_PATH];
	DWORD ret = GetCurrentDirectoryA(MAX_PATH, path_buffer);
	if (ret == 0 || ret >= (MAX_PATH - 36))
	{
		// error getting program files path
		return false;
	}

	strcat(path_buffer, "\\NVIDIA\\nvml.dll");

	HMODULE hmod = LoadLibraryA(path_buffer);
	if (hmod == NULL) return false;

	NVMLInit = (nvml_Init)GetProcAddress(hmod, "nvmlInit_v2");
	NVMLShutdown = (nvml_Shutdown)GetProcAddress(hmod, "nvmlShutdown");
	NVMLDeviceGetHandleByPciBusId = (nvml_DeviceGetHandleByPciBusId)GetProcAddress(hmod, "nvmlDeviceGetHandleByPciBusId_v2");
	NVMLDeviceGetUUID = (nvml_DeviceGetUUID)GetProcAddress(hmod, "nvmlDeviceGetUUID");
	NVMLDeviceGetPciInfo = (nvml_DeviceGetPciInfo)GetProcAddress(hmod, "nvmlDeviceGetPciInfo_v2");
	NVMLDeviceGetDisplayActive = (nvml_DeviceGetDisplayActive)GetProcAddress(hmod, "nvmlDeviceGetDisplayActive");
	NVMLSystemGetDriverVersion = (nvml_SystemGetDriverVersion)GetProcAddress(hmod, "nvmlSystemGetDriverVersion");

	int initStatus = -1;
	if (NVMLInit) {
		initStatus = NVMLInit();
	}
	return NVML_SUCCESS == initStatus;
}

void nvidia_nvml_helper::SetCudaDeviceAttributes(const char *pciBusID, CudaDevice &cudaDevice) {

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
			cudaDevice.VendorName = getVendorString(pciInfo);
			cudaDevice.pciDeviceId = pciInfo.pciDeviceId;
			cudaDevice.pciSubSystemId = pciInfo.pciSubSystemId;
			cudaDevice.VendorID = getVendorId(pciInfo);
		}

		if (NVMLDeviceGetDisplayActive && NVMLDeviceGetDisplayActive(device_t, &isEnabled) == 0) {
			cudaDevice.HasMonitorConnected = isEnabled;
		}
	}
}

std::string nvidia_nvml_helper::GetDriverVersionSafe() {
	std::string buffer;
	buffer.resize(NVML_SYSTEM_DRIVER_VERSION_BUFFER_SIZE);
	if (NVMLSystemGetDriverVersion && NVML_SUCCESS == NVMLSystemGetDriverVersion(buffer.data(), NVML_SYSTEM_DRIVER_VERSION_BUFFER_SIZE)) {
		return buffer;
	}
	return "";
}


void nvidia_nvml_helper::SafeNVMLShutdown() {
	if (NVMLShutdown) NVMLShutdown();
}


std::map<uint16_t, std::string> nvidia_nvml_helper::_VENDOR_NAMES = {
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
	{ 0, "" }
};

uint16_t nvidia_nvml_helper::getVendorId(nvmlPciInfo_t &nvmlPciInfo) {
	uint16_t vendorId = 0;
	vendorId = nvmlPciInfo.pciDeviceId & 0xFFFF;
	if (vendorId == 0x10DE && nvmlPciInfo.pciSubSystemId) {
		vendorId = nvmlPciInfo.pciSubSystemId & 0xFFFF;
	}
	return vendorId;
}

std::string nvidia_nvml_helper::getVendorString(nvmlPciInfo_t &nvmlPciInfo) {
	auto venId = getVendorId(nvmlPciInfo);
	for (const auto &vpair : _VENDOR_NAMES) {
		if (vpair.first == venId) return vpair.second;
	}
	return "UNKNOWN";
}