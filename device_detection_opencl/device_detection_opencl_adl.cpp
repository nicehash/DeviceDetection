#include "device_detection_opencl_adl.h"

#include <windows.h>
#include <vector>
#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <utility>
#include <ctype.h>
#include <vector>
#include "include/adl_sdk.h"

#pragma region HELPER structs

// This will list OpenCL devices, but AMD will only have aditional BusID
struct OpenCLDevice {
	unsigned int DeviceID = {};
	std::string _CL_DEVICE_NAME = {};
	std::string _CL_DEVICE_TYPE = {};
	unsigned long long _CL_DEVICE_GLOBAL_MEM_SIZE = {};
	std::string _CL_DEVICE_VENDOR = {};
	std::string _CL_DEVICE_VERSION = {};
	std::string _CL_DRIVER_VERSION = {};
	int BUS_ID = -1; // -1 indicates that it is not set
	std::string _CL_DEVICE_BOARD_NAME_AMD = {};
};

// rename this to platform
struct OpenCLPlatform {
	std::string PlatformVendor = {};
	std::string PlatformName = {};
	int PlatformNum = {};
	std::vector<OpenCLDevice> Devices = {};
};

struct ADLBusIDVersionPair {
	int BUS_ID = -1;
	std::string version = "";
};

struct detection_result {
	std::string Status = "OK";
	std::string ErrorString = "";
	std::vector<OpenCLPlatform> Platforms;
	std::vector<ADLBusIDVersionPair> BusIDVersionPairs;
};


#pragma endregion HELPER structs

#pragma region nlohmann::json

#include "json.hpp"

void to_json(nlohmann::json& j, const OpenCLDevice& dev) {
	j =
	{
		{ "DeviceID", dev.DeviceID },
		{ "BUS_ID", dev.BUS_ID },
		{ "_CL_DEVICE_NAME", dev._CL_DEVICE_NAME },
		{ "_CL_DEVICE_TYPE", dev._CL_DEVICE_TYPE },
		{ "_CL_DEVICE_GLOBAL_MEM_SIZE", dev._CL_DEVICE_GLOBAL_MEM_SIZE },
		{ "_CL_DEVICE_VENDOR", dev._CL_DEVICE_VENDOR },
		{ "_CL_DEVICE_VERSION", dev._CL_DEVICE_VERSION },
		{ "_CL_DRIVER_VERSION", dev._CL_DRIVER_VERSION },
		{ "_CL_DEVICE_BOARD_NAME_AMD", dev._CL_DEVICE_BOARD_NAME_AMD},
	};
}

void to_json(nlohmann::json& j, const OpenCLPlatform& p) {
	j =
	{
		{ "PlatformVendor", p.PlatformVendor },
		{ "PlatformName", p.PlatformName },
		{ "PlatformNum", p.PlatformNum },
		{ "Devices", p.Devices }
	};
}

void to_json(nlohmann::json& j, const ADLBusIDVersionPair& a) {
	j = {
		{ "BUS_ID", a.BUS_ID },
		{ "AdrenalinVersion", a.version}
	};
}

void to_json(nlohmann::json& j, const detection_result& d) {
	j =
	{
		{ "Status", d.Status },
		{ "Platforms", d.Platforms },
		{ "ErrorString", d.ErrorString },
		{ "AMDBusIDVersionPairs", d.BusIDVersionPairs }
	};
}


#pragma endregion nlohmann::json

#pragma region OpenCL

#define __CL_ENABLE_EXCEPTIONS
#define CL_USE_DEPRECATED_OPENCL_2_0_APIS
//#define CL_VERSION_1_2
#define CL_USE_DEPRECATED_OPENCL_1_1_APIS
#include "CL/cl_ext.h"

#define ENABLE_NVIDIA_PCI_BUS_ID
#ifdef ENABLE_NVIDIA_PCI_BUS_ID
// AMD APP SDK doesn't have this
#define CL_DEVICE_PCI_BUS_ID_NV                     0x4008
#define CL_DEVICE_PCI_SLOT_ID_NV                    0x4009
#endif

namespace opencl_adl_device_detection {
	namespace open_cl_helpers {
		//C implementation
		/////////////////////////////////////////////////////////////////

		// cl error to string
		const char* cl_err_to_str(cl_int ret)
		{
			switch (ret)
			{
			case CL_SUCCESS:
				return "CL_SUCCESS";
			case CL_DEVICE_NOT_FOUND:
				return "CL_DEVICE_NOT_FOUND";
			case CL_DEVICE_NOT_AVAILABLE:
				return "CL_DEVICE_NOT_AVAILABLE";
			case CL_COMPILER_NOT_AVAILABLE:
				return "CL_COMPILER_NOT_AVAILABLE";
			case CL_MEM_OBJECT_ALLOCATION_FAILURE:
				return "CL_MEM_OBJECT_ALLOCATION_FAILURE";
			case CL_OUT_OF_RESOURCES:
				return "CL_OUT_OF_RESOURCES";
			case CL_OUT_OF_HOST_MEMORY:
				return "CL_OUT_OF_HOST_MEMORY";
			case CL_PROFILING_INFO_NOT_AVAILABLE:
				return "CL_PROFILING_INFO_NOT_AVAILABLE";
			case CL_MEM_COPY_OVERLAP:
				return "CL_MEM_COPY_OVERLAP";
			case CL_IMAGE_FORMAT_MISMATCH:
				return "CL_IMAGE_FORMAT_MISMATCH";
			case CL_IMAGE_FORMAT_NOT_SUPPORTED:
				return "CL_IMAGE_FORMAT_NOT_SUPPORTED";
			case CL_BUILD_PROGRAM_FAILURE:
				return "CL_BUILD_PROGRAM_FAILURE";
			case CL_MAP_FAILURE:
				return "CL_MAP_FAILURE";
			case CL_MISALIGNED_SUB_BUFFER_OFFSET:
				return "CL_MISALIGNED_SUB_BUFFER_OFFSET";
			case CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST:
				return "CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST";
#ifdef CL_VERSION_1_2
			case CL_COMPILE_PROGRAM_FAILURE:
				return "CL_COMPILE_PROGRAM_FAILURE";
			case CL_LINKER_NOT_AVAILABLE:
				return "CL_LINKER_NOT_AVAILABLE";
			case CL_LINK_PROGRAM_FAILURE:
				return "CL_LINK_PROGRAM_FAILURE";
			case CL_DEVICE_PARTITION_FAILED:
				return "CL_DEVICE_PARTITION_FAILED";
			case CL_KERNEL_ARG_INFO_NOT_AVAILABLE:
				return "CL_KERNEL_ARG_INFO_NOT_AVAILABLE";
#endif
			case CL_INVALID_VALUE:
				return "CL_INVALID_VALUE";
			case CL_INVALID_DEVICE_TYPE:
				return "CL_INVALID_DEVICE_TYPE";
			case CL_INVALID_PLATFORM:
				return "CL_INVALID_PLATFORM";
			case CL_INVALID_DEVICE:
				return "CL_INVALID_DEVICE";
			case CL_INVALID_CONTEXT:
				return "CL_INVALID_CONTEXT";
			case CL_INVALID_QUEUE_PROPERTIES:
				return "CL_INVALID_QUEUE_PROPERTIES";
			case CL_INVALID_COMMAND_QUEUE:
				return "CL_INVALID_COMMAND_QUEUE";
			case CL_INVALID_HOST_PTR:
				return "CL_INVALID_HOST_PTR";
			case CL_INVALID_MEM_OBJECT:
				return "CL_INVALID_MEM_OBJECT";
			case CL_INVALID_IMAGE_FORMAT_DESCRIPTOR:
				return "CL_INVALID_IMAGE_FORMAT_DESCRIPTOR";
			case CL_INVALID_IMAGE_SIZE:
				return "CL_INVALID_IMAGE_SIZE";
			case CL_INVALID_SAMPLER:
				return "CL_INVALID_SAMPLER";
			case CL_INVALID_BINARY:
				return "CL_INVALID_BINARY";
			case CL_INVALID_BUILD_OPTIONS:
				return "CL_INVALID_BUILD_OPTIONS";
			case CL_INVALID_PROGRAM:
				return "CL_INVALID_PROGRAM";
			case CL_INVALID_PROGRAM_EXECUTABLE:
				return "CL_INVALID_PROGRAM_EXECUTABLE";
			case CL_INVALID_KERNEL_NAME:
				return "CL_INVALID_KERNEL_NAME";
			case CL_INVALID_KERNEL_DEFINITION:
				return "CL_INVALID_KERNEL_DEFINITION";
			case CL_INVALID_KERNEL:
				return "CL_INVALID_KERNEL";
			case CL_INVALID_ARG_INDEX:
				return "CL_INVALID_ARG_INDEX";
			case CL_INVALID_ARG_VALUE:
				return "CL_INVALID_ARG_VALUE";
			case CL_INVALID_ARG_SIZE:
				return "CL_INVALID_ARG_SIZE";
			case CL_INVALID_KERNEL_ARGS:
				return "CL_INVALID_KERNEL_ARGS";
			case CL_INVALID_WORK_DIMENSION:
				return "CL_INVALID_WORK_DIMENSION";
			case CL_INVALID_WORK_GROUP_SIZE:
				return "CL_INVALID_WORK_GROUP_SIZE";
			case CL_INVALID_WORK_ITEM_SIZE:
				return "CL_INVALID_WORK_ITEM_SIZE";
			case CL_INVALID_GLOBAL_OFFSET:
				return "CL_INVALID_GLOBAL_OFFSET";
			case CL_INVALID_EVENT_WAIT_LIST:
				return "CL_INVALID_EVENT_WAIT_LIST";
			case CL_INVALID_EVENT:
				return "CL_INVALID_EVENT";
			case CL_INVALID_OPERATION:
				return "CL_INVALID_OPERATION";
			case CL_INVALID_GL_OBJECT:
				return "CL_INVALID_GL_OBJECT";
			case CL_INVALID_BUFFER_SIZE:
				return "CL_INVALID_BUFFER_SIZE";
			case CL_INVALID_MIP_LEVEL:
				return "CL_INVALID_MIP_LEVEL";
			case CL_INVALID_GLOBAL_WORK_SIZE:
				return "CL_INVALID_GLOBAL_WORK_SIZE";
			case CL_INVALID_PROPERTY:
				return "CL_INVALID_PROPERTY";
#ifdef CL_VERSION_1_2
			case CL_INVALID_IMAGE_DESCRIPTOR:
				return "CL_INVALID_IMAGE_DESCRIPTOR";
			case CL_INVALID_COMPILER_OPTIONS:
				return "CL_INVALID_COMPILER_OPTIONS";
			case CL_INVALID_LINKER_OPTIONS:
				return "CL_INVALID_LINKER_OPTIONS";
			case CL_INVALID_DEVICE_PARTITION_COUNT:
				return "CL_INVALID_DEVICE_PARTITION_COUNT";
#endif
#if defined(CL_VERSION_2_0) && !defined(CONF_ENFORCE_OpenCL_1_2)
			case CL_INVALID_PIPE_SIZE:
				return "CL_INVALID_PIPE_SIZE";
			case CL_INVALID_DEVICE_QUEUE:
				return "CL_INVALID_DEVICE_QUEUE";
#endif
			default:
				return "UNKNOWN_ERROR";
			}
		}

		std::string getClGetDeviceInfoString(cl_platform_id platform, cl_platform_info  param_name)
		{
			cl_int clStatus;

			size_t infoSize;
			clStatus = clGetPlatformInfo(platform, param_name, 0, NULL, &infoSize);
			if (clStatus != CL_SUCCESS)
			{
				// error
				return std::string(cl_err_to_str(clStatus));
			}
			std::vector<char> strBuff(infoSize);
			clStatus = clGetPlatformInfo(platform, param_name, infoSize, strBuff.data(), NULL);
			if (clStatus == CL_SUCCESS)
			{
				if (strBuff.size() == 0 || strBuff.data() == nullptr) return "";
				return std::string(strBuff.data());
			}

			// error
			return std::string(cl_err_to_str(clStatus));
		}

		std::string getClGetDeviceInfoString(cl_device_id device, cl_device_info  param_name)
		{
			cl_int clStatus;

			size_t infoSize;
			clStatus = clGetDeviceInfo(device, param_name, 0, NULL, &infoSize);
			if (clStatus != CL_SUCCESS)
			{
				// error
				return std::string(cl_err_to_str(clStatus));
			}
			std::vector<char> strBuff(infoSize);
			clStatus = clGetDeviceInfo(device, param_name, infoSize, strBuff.data(), NULL);
			if (clStatus == CL_SUCCESS)
			{
				if (strBuff.size() == 0 || strBuff.data() == nullptr) return "";
				return std::string(strBuff.data());
			}

			// error
			return std::string(cl_err_to_str(clStatus));
		}

	} // namespace open_cl_helpers

} // namespace opencl_device_detection

#pragma endregion OpenCL

#pragma region ADL
namespace adl_device_detection {
	bool initialized = false;
	int  iNumberAdapters;

	typedef struct ADLVersionsInfoX2
	{
		/// Driver Release (Packaging) Version (e.g. "16.20.1035-160621a-303814C")
		char strDriverVer[256];
		/// Catalyst Version(e.g. "15.8").
		char strCatalystVersion[256];
		/// Crimson Version(e.g. "16.6.2").
		char strCrimsonVersion[256];
		/// Web link to an XML file with information about the latest AMD drivers and locations (e.g. "http://support.amd.com/drivers/xml/driver_09_us.xml" )
		char strCatalystWebLink[256];

	} ADLVersionsInfoX2, * LPADLVersionsInfoX2;

	typedef int(*ADL2_GRAPHICS_VERSIONSX3_GET)  (ADL_CONTEXT_HANDLE, int, ADLVersionsInfoX2*);
	typedef int(*ADL_MAIN_CONTROL_CREATE)(ADL_MAIN_MALLOC_CALLBACK, int);
	typedef int(*ADL_MAIN_CONTROL_DESTROY)();
	typedef int(*ADL_ADAPTER_ADAPTERINFO_GET) (LPAdapterInfo, int);
	typedef int(*ADL_ADAPTER_NUMBEROFADAPTERS_GET) (int*);


	HINSTANCE hDLL;
	LPAdapterInfo lpAdapterInfo = NULL;
	ADL_CONTEXT_HANDLE context = NULL;


	ADL2_GRAPHICS_VERSIONSX3_GET ADL2_Graphics_VersionsX3_Get = NULL;
	ADL_MAIN_CONTROL_CREATE ADL_Main_Control_Create = NULL;
	ADL_MAIN_CONTROL_DESTROY ADL_Main_Control_Destroy = NULL;
	ADL_ADAPTER_ADAPTERINFO_GET ADL_Adapter_AdapterInfo_Get = NULL;
	ADL_ADAPTER_NUMBEROFADAPTERS_GET ADL_Adapter_NumberOfAdapters_Get = NULL;



	// Memory allocation function
	void* __stdcall ADL_Main_Memory_Alloc(int iSize)
	{
		void* lpBuffer = malloc(iSize);
		return lpBuffer;
	}

	// Optional Memory de-allocation function
	void __stdcall ADL_Main_Memory_Free(void** lpBuffer)
	{
		if (NULL != *lpBuffer)
		{
			free(*lpBuffer);
			*lpBuffer = NULL;
		}
	}

	int initializeADL()
	{
		char path_buffer[MAX_PATH];
		DWORD ret = GetEnvironmentVariableA("windir", path_buffer, MAX_PATH - 36);
		std::string adlxxInstall = std::string(path_buffer) + "\\System32\\atiadlxx.dll";
		hDLL = LoadLibraryA(adlxxInstall.c_str());
		if (hDLL == NULL)
		{
			// A 32 bit calling application on 64 bit OS will fail to LoadLibrary.
			// Try to load the 32 bit library (atiadlxy.dll) instead
			std::string adlxyInstall = std::string(path_buffer) + "\\System32\\atiadlxy.dll";
			hDLL = LoadLibraryA(adlxyInstall.c_str());
		}

		if (NULL == hDLL)
		{
			//nhm_amd_log("Failed to load ADL library");
			return FALSE;
		}
		ADL_Main_Control_Create = (ADL_MAIN_CONTROL_CREATE)GetProcAddress(hDLL, "ADL_Main_Control_Create");
		ADL_Main_Control_Destroy = (ADL_MAIN_CONTROL_DESTROY)GetProcAddress(hDLL, "ADL_Main_Control_Destroy");
		ADL_Adapter_NumberOfAdapters_Get = (ADL_ADAPTER_NUMBEROFADAPTERS_GET)GetProcAddress(hDLL, "ADL_Adapter_NumberOfAdapters_Get");
		ADL_Adapter_AdapterInfo_Get = (ADL_ADAPTER_ADAPTERINFO_GET)GetProcAddress(hDLL, "ADL_Adapter_AdapterInfo_Get");
		ADL2_Graphics_VersionsX3_Get = (ADL2_GRAPHICS_VERSIONSX3_GET)GetProcAddress(hDLL, "ADL2_Graphics_VersionsX3_Get");

		if (NULL == ADL_Main_Control_Create ||
			NULL == ADL_Main_Control_Destroy ||
			NULL == ADL2_Graphics_VersionsX3_Get ||
			NULL == ADL_Adapter_AdapterInfo_Get ||
			NULL == ADL_Adapter_NumberOfAdapters_Get
			)
		{
			//nhm_amd_log("Failed to get ADL function pointers");
			return FALSE;
		}

		if (ADL_OK != ADL_Main_Control_Create(ADL_Main_Memory_Alloc, 1))
		{
			printf("Failed to initialize nested ADL2 context");
			return ADL_ERR;
		}
		return TRUE;
	}

	void deinitializeADL()
	{
		ADL_Main_Control_Destroy();
		FreeLibrary(hDLL);
	}

	int init() {
		if (initialized) return 1;
		if (initializeADL())
		{
			initialized = true;
			// Obtain the number of adapters for the system
			if (int adl_ret = ADL_Adapter_NumberOfAdapters_Get(&iNumberAdapters); ADL_OK != adl_ret) {
				//nhm_amd_log("nhm_amd_init Error ADL_Adapter_NumberOfAdapters_Get: Code %d", adl_ret);
				return -1;
			}

			if (0 < iNumberAdapters) {
				lpAdapterInfo = (LPAdapterInfo)malloc(sizeof(AdapterInfo) * iNumberAdapters);
				if (lpAdapterInfo == nullptr) return -4;
				memset(lpAdapterInfo, 0, sizeof(AdapterInfo) * iNumberAdapters);

				// Get the AdapterInfo structure for all adapters in the system
				if (int adl_ret = ADL_Adapter_AdapterInfo_Get(lpAdapterInfo, sizeof(AdapterInfo) * iNumberAdapters); ADL_OK != adl_ret) {
					//nhm_amd_log("nhm_amd_init Error ADL_Adapter_AdapterInfo_Get: Code %d", adl_ret);
					return -2;
				}
			}
			return 0;
		}
		return -3;
	}

	int deinit() {
		if (initialized) {
			initialized = false;
			ADL_Main_Memory_Free((void**)&lpAdapterInfo);
			deinitializeADL();
			return 0;
		}
		return -1;
	}

	int has_adapter(const int bus_number) {
		for (int i = 0; i < iNumberAdapters; i++) {
			if (lpAdapterInfo[i].iBusNumber == bus_number) return 0;
		}
		return -1;
	}


	int get_driver_version(const int bus_number, ADLVersionsInfoX2* driverVersion) {
		if (ADL2_Graphics_VersionsX3_Get == NULL) return -1;

		bool bus_id_found = false;
		bool version_found = false;
		int ret = 0;

		for (int i = 0; i < iNumberAdapters; i++) {
			if (lpAdapterInfo[i].iBusNumber != bus_number) continue;
			//DEBUG_LOG(1, "[adapterIndex, bus_number]=[%d, %d]", i, bus_number);
			bus_id_found = true;

			//ADLVersionsInfoX2 driverVersion = { {0}, {0}, {0}, {0} };
			ret = ADL2_Graphics_VersionsX3_Get(context, lpAdapterInfo[i].iAdapterIndex, driverVersion);
			//DEBUG_LOG(1, "ADL2_Graphics_VersionsX3_Get: Code %d", ret);
			//DEBUG_LOG(2, *driverVersion);
			if (ADL_OK != ret) {
				//nhm_amd_log("Error ADL2_Graphics_VersionsX3_Get: Code %d", ret);
				break;
			}
			version_found = true;
			return 0;
		}
		if (!bus_id_found) return -1;
		if (!version_found) return -1;
		return ret;
	}

	bool vectorContainsBusNumber(std::vector<ADLBusIDVersionPair> v, int bus) {
		for (int i = 0; i < v.size(); i++) {
			if (v[i].BUS_ID == bus) {
				return true;
			}
		}
		return false;
	}

	std::vector<ADLBusIDVersionPair> get_amd_device_driver_versions() {
		std::vector<ADLBusIDVersionPair> versionList;
		bool globalOk = true;
		init();
		for (int i = 0; i < iNumberAdapters; i++) {
			try {
				ADLVersionsInfoX2 temp;
				if (vectorContainsBusNumber(versionList, lpAdapterInfo[i].iBusNumber)) continue;

				int ok = get_driver_version(lpAdapterInfo[i].iBusNumber, &temp);
				std::string versionStr = std::string(temp.strCrimsonVersion);
				ADLBusIDVersionPair tempPair;
				tempPair.BUS_ID = lpAdapterInfo[i].iBusNumber;
				tempPair.version = versionStr;
				if (ok == 0 && versionStr != "") {
					versionList.push_back(tempPair);
				}
			}
			catch (std::exception ex) {
				globalOk = false;
			}
		}
		deinit();
		return versionList;
	}
}
#pragma endregion ADL

#pragma region opencl_adl_device_detection
namespace opencl_adl_device_detection {
	std::tuple<bool, std::string> get_devices_json_result(bool prettyPrint) {
		bool ok = true;
		detection_result result;
		auto AppendToErrorString = [&result](cl_int clStatus) {
			result.ErrorString += "_err_";
			result.ErrorString += opencl_adl_device_detection::open_cl_helpers::cl_err_to_str(clStatus);
		};
		try {
			// get platforms
			cl_uint numPlatforms = 0;
			cl_int clStatus;

			// Get platform and device information
			clStatus = clGetPlatformIDs(0, NULL, &numPlatforms);
			if (clStatus != CL_SUCCESS)
			{
				throw std::runtime_error(std::string(opencl_adl_device_detection::open_cl_helpers::cl_err_to_str(clStatus)) + " when calling clGetPlatformIDs for number of platforms.");
			}
			if (numPlatforms == 0)
			{
				throw std::runtime_error("No OpenCL platforms found");
			}

			std::vector<cl_platform_id> platforms;
			platforms.resize(numPlatforms);
			clStatus = clGetPlatformIDs(numPlatforms, platforms.data(), NULL);

			if (clStatus != CL_SUCCESS) {
				throw std::runtime_error(std::string(opencl_adl_device_detection::open_cl_helpers::cl_err_to_str(clStatus)) + " when calling clGetPlatformIDs for platform information.");
			}

			// iterate platforms
			for (unsigned int i = 0; i < numPlatforms; i++)
			{
				std::string platformName = opencl_adl_device_detection::open_cl_helpers::getClGetDeviceInfoString(platforms[i], CL_PLATFORM_VENDOR);
				std::string platformName2 = opencl_adl_device_detection::open_cl_helpers::getClGetDeviceInfoString(platforms[i], CL_PLATFORM_NAME);

				bool isAMDOpenCL = platformName.find("Advanced Micro Devices") != std::string::npos ||
					platformName.find("Apple") != std::string::npos ||
					platformName.find("Mesa") != std::string::npos;
				bool isNVIDIADevice = platformName.find("NVIDIA Corporation") != std::string::npos || platformName.find("NVIDIA") != std::string::npos;

				OpenCLPlatform current;
				current.PlatformName = platformName2;
				current.PlatformVendor = platformName;
				current.PlatformNum = i;

				// devices per plaftorm
				cl_uint num_devices;
				std::vector<cl_device_id> device_list;
				if ((clStatus = clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_GPU, 0, NULL, &num_devices)) != CL_SUCCESS)
				{
					continue;
				}
				device_list.resize(num_devices);
				if ((clStatus = clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_GPU, num_devices, device_list.data(), NULL)) != CL_SUCCESS)
				{
					continue;
				}
				// iterate platform devs
				for (size_t k = 0; k < num_devices; k++)
				{
					OpenCLDevice curDevice;
					curDevice.DeviceID = (int)k;

					// CL_DEVICE_NAME
					curDevice._CL_DEVICE_NAME = opencl_adl_device_detection::open_cl_helpers::getClGetDeviceInfoString(device_list[k], CL_DEVICE_NAME);

					// CL_DEVICE_VENDOR
					std::string vendor = opencl_adl_device_detection::open_cl_helpers::getClGetDeviceInfoString(device_list[k], CL_DEVICE_VENDOR);
					bool isAMDDevice = vendor.find("Advanced Micro Devices") != std::string::npos || vendor.find("AMD") != std::string::npos;
					bool isNVIDIADevice = vendor.find("NVIDIA Corporation") != std::string::npos || vendor.find("NVIDIA") != std::string::npos;
					curDevice._CL_DEVICE_VENDOR = vendor;

					// CL_DEVICE_VERSION
					curDevice._CL_DEVICE_VERSION = opencl_adl_device_detection::open_cl_helpers::getClGetDeviceInfoString(device_list[k], CL_DEVICE_VERSION);

					// CL_DRIVER_VERSION
					curDevice._CL_DRIVER_VERSION = opencl_adl_device_detection::open_cl_helpers::getClGetDeviceInfoString(device_list[k], CL_DRIVER_VERSION);

					// CL_DEVICE_TYPE
					cl_device_type type;
					if ((clStatus = clGetDeviceInfo(device_list[k], CL_DEVICE_TYPE, sizeof(cl_device_type), &type, NULL)) == CL_SUCCESS)
					{
						switch (type) {
						case CL_DEVICE_TYPE_CPU:
							curDevice._CL_DEVICE_TYPE = "CPU";
							break;
						case CL_DEVICE_TYPE_GPU:
							curDevice._CL_DEVICE_TYPE = "GPU";
							break;
						case CL_DEVICE_TYPE_ACCELERATOR:
							curDevice._CL_DEVICE_TYPE = "ACCELERATOR";
							break;
						default:
							curDevice._CL_DEVICE_TYPE = "DEFAULT";
							break;
						}
					}
					else {
						AppendToErrorString(clStatus);
					}

					// CL_DEVICE_GLOBAL_MEM_SIZE
					if ((clStatus = clGetDeviceInfo(device_list[k], CL_DEVICE_GLOBAL_MEM_SIZE, sizeof(size_t), &(curDevice._CL_DEVICE_GLOBAL_MEM_SIZE), NULL)) != CL_SUCCESS)
					{
						AppendToErrorString(clStatus);
					}

					// PCIe BUS id
					// AMD extensions
					if (isAMDDevice) {
						// CL_DEVICE_TOPOLOGY_AMD
						cl_device_topology_amd topology = {};
						if ((clStatus = clGetDeviceInfo(device_list[k], CL_DEVICE_TOPOLOGY_AMD, sizeof(cl_device_topology_amd), &topology, NULL)) == CL_SUCCESS)
						{
							if (topology.raw.type == CL_DEVICE_TOPOLOGY_TYPE_PCIE_AMD) {
								curDevice.BUS_ID = (int)topology.pcie.bus;
							}
						}
						else
						{
							AppendToErrorString(clStatus);
						}

						// CL_DEVICE_BOARD_NAME_AMD
						curDevice._CL_DEVICE_BOARD_NAME_AMD = opencl_adl_device_detection::open_cl_helpers::getClGetDeviceInfoString(device_list[k], CL_DEVICE_BOARD_NAME_AMD);
					}
					// NVIDIA extensions
					if (isNVIDIADevice) {
						int tmp;
						if ((clStatus = clGetDeviceInfo(device_list[k], CL_DEVICE_PCI_BUS_ID_NV, sizeof(int), &tmp, NULL)) == CL_SUCCESS)
						{
							curDevice.BUS_ID = tmp;
						}
						else {
							AppendToErrorString(clStatus);
							if ((clStatus = clGetDeviceInfo(device_list[k], CL_DEVICE_PCI_SLOT_ID_NV, sizeof(int), &tmp, NULL)) == CL_SUCCESS) {
								curDevice.BUS_ID = tmp;
							}
							else {
								AppendToErrorString(clStatus);
							}
						}
					}

					current.Devices.push_back(curDevice);
				}
				result.Platforms.push_back(current);
			}
			auto deviceVersionList = adl_device_detection::get_amd_device_driver_versions();
			result.BusIDVersionPairs = deviceVersionList;
		}
		catch (std::exception& ex) {
			result.Status = "Error " + std::string(ex.what());
			ok = false;
		}
		nlohmann::json j;
		to_json(j, result);
		auto json_str = prettyPrint ? j.dump(4) : j.dump();
		return { ok, json_str };
	}

}
#pragma endregion opencl_adl_device_detection




const char* open_cl_adl_device_detection_json_result_str(bool pretty_print)
{
	static bool called = false;
	static std::string ret;
	if (!called) {
		called = true;
		const auto [ok, json_str] = opencl_adl_device_detection::get_devices_json_result(pretty_print);
		ret = json_str;
	}
	return ret.c_str();
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