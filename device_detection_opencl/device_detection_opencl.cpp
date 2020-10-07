#include "device_detection_opencl.h"

#include <windows.h>
#include <vector>
#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <utility>
#include <ctype.h>

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

struct detection_result {
	std::string Status = "OK";
	std::string ErrorString = "";
	std::vector<OpenCLPlatform> Platforms;
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

void to_json(nlohmann::json& j, const detection_result& d) {
	j =
	{
		{ "Status", d.Status },
		{ "Platforms", d.Platforms },
		{ "ErrorString", d.ErrorString }
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

namespace opencl_device_detection {
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

	std::tuple<bool, std::string> get_devices_json_result(bool prettyPrint) {
		bool ok = true;
		detection_result result;
		auto AppendToErrorString = [&result](cl_int clStatus) {
			result.ErrorString += "_err_";
			result.ErrorString += open_cl_helpers::cl_err_to_str(clStatus);
		};
		try {
			// get platforms
			cl_uint numPlatforms = 0;
			cl_int clStatus;

			// Get platform and device information
			clStatus = clGetPlatformIDs(0, NULL, &numPlatforms);
			if (clStatus != CL_SUCCESS)
			{
				throw std::runtime_error(std::string(open_cl_helpers::cl_err_to_str(clStatus)) + " when calling clGetPlatformIDs for number of platforms.");
			}
			if (numPlatforms == 0)
			{
				throw std::runtime_error("No OpenCL platforms found");
			}

			std::vector<cl_platform_id> platforms;
			platforms.resize(numPlatforms);
			clStatus = clGetPlatformIDs(numPlatforms, platforms.data(), NULL);

			if (clStatus != CL_SUCCESS) {
				throw std::runtime_error(std::string(open_cl_helpers::cl_err_to_str(clStatus)) + " when calling clGetPlatformIDs for platform information.");
			}

			// iterate platforms
			for (unsigned int i = 0; i < numPlatforms; i++)
			{
				std::string platformName = open_cl_helpers::getClGetDeviceInfoString(platforms[i], CL_PLATFORM_VENDOR);
				std::string platformName2 = open_cl_helpers::getClGetDeviceInfoString(platforms[i], CL_PLATFORM_NAME);

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
					curDevice._CL_DEVICE_NAME = open_cl_helpers::getClGetDeviceInfoString(device_list[k], CL_DEVICE_NAME);

					// CL_DEVICE_VENDOR
					std::string vendor = open_cl_helpers::getClGetDeviceInfoString(device_list[k], CL_DEVICE_VENDOR);
					bool isAMDDevice = vendor.find("Advanced Micro Devices") != std::string::npos || vendor.find("AMD") != std::string::npos;
					bool isNVIDIADevice = vendor.find("NVIDIA Corporation") != std::string::npos || vendor.find("NVIDIA") != std::string::npos;
					curDevice._CL_DEVICE_VENDOR = vendor;

					// CL_DEVICE_VERSION
					curDevice._CL_DEVICE_VERSION = open_cl_helpers::getClGetDeviceInfoString(device_list[k], CL_DEVICE_VERSION);

					// CL_DRIVER_VERSION
					curDevice._CL_DRIVER_VERSION = open_cl_helpers::getClGetDeviceInfoString(device_list[k], CL_DRIVER_VERSION);

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
						curDevice._CL_DEVICE_BOARD_NAME_AMD = open_cl_helpers::getClGetDeviceInfoString(device_list[k], CL_DEVICE_BOARD_NAME_AMD);
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

} // namespace opencl_device_detection

#pragma endregion OpenCL


const char* open_cl_device_detection_json_result_str(bool prettyString)
{
	static std::string ret;
	const auto [ok, json_str] = opencl_device_detection::get_devices_json_result(prettyString);
	ret = json_str;
	return ret.c_str();
}

char* _GetOpenCLDevices(bool prettyJSONString) {
	return (char*)open_cl_device_detection_json_result_str(prettyJSONString);
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