#include "AMDOpenCLDeviceDetection.h"

#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <utility>
#include <ctype.h>

#include "json_helpers.h"

using namespace std;
using namespace cl;

AMDOpenCLDeviceDetection::AMDOpenCLDeviceDetection()
{
}

AMDOpenCLDeviceDetection::~AMDOpenCLDeviceDetection()
{
}

string AMDOpenCLDeviceDetection::StringnNullTerminatorFix(const string& str) {
	unsigned int s = 0;
	while (isprint(str[s])) ++s;
	return string(str.data(), s);
}

vector<Platform> AMDOpenCLDeviceDetection::getPlatforms() {
	vector<Platform> platforms;
	try {
		Platform::get(&platforms);
	}
	catch (Error const& err) {
#if defined(CL_PLATFORM_NOT_FOUND_KHR)
		if (err.err() == CL_PLATFORM_NOT_FOUND_KHR)
			throw exception("No OpenCL platforms found");
		else
#endif
			throw err;
	}
	return platforms;
}

vector<Device> AMDOpenCLDeviceDetection::getDevices(vector<Platform> const& _platforms, unsigned _platformId) {
	vector<Device> devices;
	try {
		_platforms[_platformId].getDevices(/*CL_DEVICE_TYPE_CPU| */CL_DEVICE_TYPE_GPU | CL_DEVICE_TYPE_ACCELERATOR, &devices);
	}
	catch (Error const& err) {
		// if simply no devices found return empty vector
		if (err.err() != CL_DEVICE_NOT_FOUND)
			throw err;
	}
	return devices;
}

bool AMDOpenCLDeviceDetection::QueryDevices() {
	try {
		// get platforms
		auto platforms = getPlatforms();
		if (platforms.empty()) {
			_statusString = "No OpenCL platforms found";
			cout << "No OpenCL platforms found" << endl;
			return false;
		}
		else {
			for (auto i_pId = 0u; i_pId < platforms.size(); ++i_pId) {
				string platformName = StringnNullTerminatorFix(platforms[i_pId].getInfo<CL_PLATFORM_NAME>());
				string platformVendor = StringnNullTerminatorFix(platforms[i_pId].getInfo<CL_PLATFORM_VENDOR>());
				OpenCLPlatform current;
				// new
				current.PlatformVendor = platformVendor;
				current.PlatformName = platformName;
				current.PlatformNum = i_pId;

				// not the best way but it should work
				auto clDevs = getDevices(platforms, i_pId);
				for (auto i_devId = 0u; i_devId < clDevs.size(); ++i_devId) {
					OpenCLDevice curDevice;
					curDevice.DeviceID = i_devId;
					curDevice._CL_DEVICE_NAME = StringnNullTerminatorFix(clDevs[i_devId].getInfo<CL_DEVICE_NAME>());
					switch (clDevs[i_devId].getInfo<CL_DEVICE_TYPE>()) {
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

					curDevice._CL_DEVICE_GLOBAL_MEM_SIZE = clDevs[i_devId].getInfo<CL_DEVICE_GLOBAL_MEM_SIZE>();
					curDevice._CL_DEVICE_VENDOR = StringnNullTerminatorFix(clDevs[i_devId].getInfo<CL_DEVICE_VENDOR>());
					curDevice._CL_DEVICE_VERSION = StringnNullTerminatorFix(clDevs[i_devId].getInfo<CL_DEVICE_VERSION>());
					curDevice._CL_DRIVER_VERSION = StringnNullTerminatorFix(clDevs[i_devId].getInfo<CL_DRIVER_VERSION>());

					bool isAMDDevice = curDevice._CL_DEVICE_VENDOR.find("Advanced Micro Devices") != std::string::npos || curDevice._CL_DEVICE_VENDOR.find("AMD") != std::string::npos;

					// AMD topology get Bus No
					if (isAMDDevice) {
						cl_device_topology_amd topology = clDevs[i_devId].getInfo<CL_DEVICE_TOPOLOGY_AMD>();
						if (topology.raw.type == CL_DEVICE_TOPOLOGY_TYPE_PCIE_AMD) {
							curDevice.AMD_BUS_ID = (int)topology.pcie.bus;
						}
						curDevice._CL_DEVICE_BOARD_NAME_AMD = clDevs[i_devId].getInfo<CL_DEVICE_BOARD_NAME_AMD>();
					}

					current.Devices.push_back(curDevice);
				}
				_devicesPlatformsDevices.push_back(current);
			}
		}
	}
	catch (exception &ex) {
		_errorString = ex.what();
		_statusString = "Error";
		return false;
	}

	return true;
}

string AMDOpenCLDeviceDetection::GetDevicesJsonString(bool prettyPrint) {
	return json_helpers::GetPlatformDevicesJsonString(_devicesPlatformsDevices, _statusString, _errorString, prettyPrint);
}

string AMDOpenCLDeviceDetection::GetErrorString() {
	return _errorString;
}

// C implementation
///////////////////////////////////////////////////////////////////

//// cl error to string
//const char* cl_err_to_str(cl_int ret)
//{
//	switch (ret)
//	{
//	case CL_SUCCESS:
//		return "CL_SUCCESS";
//	case CL_DEVICE_NOT_FOUND:
//		return "CL_DEVICE_NOT_FOUND";
//	case CL_DEVICE_NOT_AVAILABLE:
//		return "CL_DEVICE_NOT_AVAILABLE";
//	case CL_COMPILER_NOT_AVAILABLE:
//		return "CL_COMPILER_NOT_AVAILABLE";
//	case CL_MEM_OBJECT_ALLOCATION_FAILURE:
//		return "CL_MEM_OBJECT_ALLOCATION_FAILURE";
//	case CL_OUT_OF_RESOURCES:
//		return "CL_OUT_OF_RESOURCES";
//	case CL_OUT_OF_HOST_MEMORY:
//		return "CL_OUT_OF_HOST_MEMORY";
//	case CL_PROFILING_INFO_NOT_AVAILABLE:
//		return "CL_PROFILING_INFO_NOT_AVAILABLE";
//	case CL_MEM_COPY_OVERLAP:
//		return "CL_MEM_COPY_OVERLAP";
//	case CL_IMAGE_FORMAT_MISMATCH:
//		return "CL_IMAGE_FORMAT_MISMATCH";
//	case CL_IMAGE_FORMAT_NOT_SUPPORTED:
//		return "CL_IMAGE_FORMAT_NOT_SUPPORTED";
//	case CL_BUILD_PROGRAM_FAILURE:
//		return "CL_BUILD_PROGRAM_FAILURE";
//	case CL_MAP_FAILURE:
//		return "CL_MAP_FAILURE";
//	case CL_MISALIGNED_SUB_BUFFER_OFFSET:
//		return "CL_MISALIGNED_SUB_BUFFER_OFFSET";
//	case CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST:
//		return "CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST";
//#ifdef CL_VERSION_1_2
//	case CL_COMPILE_PROGRAM_FAILURE:
//		return "CL_COMPILE_PROGRAM_FAILURE";
//	case CL_LINKER_NOT_AVAILABLE:
//		return "CL_LINKER_NOT_AVAILABLE";
//	case CL_LINK_PROGRAM_FAILURE:
//		return "CL_LINK_PROGRAM_FAILURE";
//	case CL_DEVICE_PARTITION_FAILED:
//		return "CL_DEVICE_PARTITION_FAILED";
//	case CL_KERNEL_ARG_INFO_NOT_AVAILABLE:
//		return "CL_KERNEL_ARG_INFO_NOT_AVAILABLE";
//#endif
//	case CL_INVALID_VALUE:
//		return "CL_INVALID_VALUE";
//	case CL_INVALID_DEVICE_TYPE:
//		return "CL_INVALID_DEVICE_TYPE";
//	case CL_INVALID_PLATFORM:
//		return "CL_INVALID_PLATFORM";
//	case CL_INVALID_DEVICE:
//		return "CL_INVALID_DEVICE";
//	case CL_INVALID_CONTEXT:
//		return "CL_INVALID_CONTEXT";
//	case CL_INVALID_QUEUE_PROPERTIES:
//		return "CL_INVALID_QUEUE_PROPERTIES";
//	case CL_INVALID_COMMAND_QUEUE:
//		return "CL_INVALID_COMMAND_QUEUE";
//	case CL_INVALID_HOST_PTR:
//		return "CL_INVALID_HOST_PTR";
//	case CL_INVALID_MEM_OBJECT:
//		return "CL_INVALID_MEM_OBJECT";
//	case CL_INVALID_IMAGE_FORMAT_DESCRIPTOR:
//		return "CL_INVALID_IMAGE_FORMAT_DESCRIPTOR";
//	case CL_INVALID_IMAGE_SIZE:
//		return "CL_INVALID_IMAGE_SIZE";
//	case CL_INVALID_SAMPLER:
//		return "CL_INVALID_SAMPLER";
//	case CL_INVALID_BINARY:
//		return "CL_INVALID_BINARY";
//	case CL_INVALID_BUILD_OPTIONS:
//		return "CL_INVALID_BUILD_OPTIONS";
//	case CL_INVALID_PROGRAM:
//		return "CL_INVALID_PROGRAM";
//	case CL_INVALID_PROGRAM_EXECUTABLE:
//		return "CL_INVALID_PROGRAM_EXECUTABLE";
//	case CL_INVALID_KERNEL_NAME:
//		return "CL_INVALID_KERNEL_NAME";
//	case CL_INVALID_KERNEL_DEFINITION:
//		return "CL_INVALID_KERNEL_DEFINITION";
//	case CL_INVALID_KERNEL:
//		return "CL_INVALID_KERNEL";
//	case CL_INVALID_ARG_INDEX:
//		return "CL_INVALID_ARG_INDEX";
//	case CL_INVALID_ARG_VALUE:
//		return "CL_INVALID_ARG_VALUE";
//	case CL_INVALID_ARG_SIZE:
//		return "CL_INVALID_ARG_SIZE";
//	case CL_INVALID_KERNEL_ARGS:
//		return "CL_INVALID_KERNEL_ARGS";
//	case CL_INVALID_WORK_DIMENSION:
//		return "CL_INVALID_WORK_DIMENSION";
//	case CL_INVALID_WORK_GROUP_SIZE:
//		return "CL_INVALID_WORK_GROUP_SIZE";
//	case CL_INVALID_WORK_ITEM_SIZE:
//		return "CL_INVALID_WORK_ITEM_SIZE";
//	case CL_INVALID_GLOBAL_OFFSET:
//		return "CL_INVALID_GLOBAL_OFFSET";
//	case CL_INVALID_EVENT_WAIT_LIST:
//		return "CL_INVALID_EVENT_WAIT_LIST";
//	case CL_INVALID_EVENT:
//		return "CL_INVALID_EVENT";
//	case CL_INVALID_OPERATION:
//		return "CL_INVALID_OPERATION";
//	case CL_INVALID_GL_OBJECT:
//		return "CL_INVALID_GL_OBJECT";
//	case CL_INVALID_BUFFER_SIZE:
//		return "CL_INVALID_BUFFER_SIZE";
//	case CL_INVALID_MIP_LEVEL:
//		return "CL_INVALID_MIP_LEVEL";
//	case CL_INVALID_GLOBAL_WORK_SIZE:
//		return "CL_INVALID_GLOBAL_WORK_SIZE";
//	case CL_INVALID_PROPERTY:
//		return "CL_INVALID_PROPERTY";
//#ifdef CL_VERSION_1_2
//	case CL_INVALID_IMAGE_DESCRIPTOR:
//		return "CL_INVALID_IMAGE_DESCRIPTOR";
//	case CL_INVALID_COMPILER_OPTIONS:
//		return "CL_INVALID_COMPILER_OPTIONS";
//	case CL_INVALID_LINKER_OPTIONS:
//		return "CL_INVALID_LINKER_OPTIONS";
//	case CL_INVALID_DEVICE_PARTITION_COUNT:
//		return "CL_INVALID_DEVICE_PARTITION_COUNT";
//#endif
//#if defined(CL_VERSION_2_0) && !defined(CONF_ENFORCE_OpenCL_1_2)
//	case CL_INVALID_PIPE_SIZE:
//		return "CL_INVALID_PIPE_SIZE";
//	case CL_INVALID_DEVICE_QUEUE:
//		return "CL_INVALID_DEVICE_QUEUE";
//#endif
//	default:
//		return "UNKNOWN_ERROR";
//	}
//}


//std::string getClGetDeviceInfoString(cl_platform_id platform, cl_platform_info  param_name)
//{
//	cl_int clStatus;
//
//	size_t infoSize;
//	clStatus = clGetPlatformInfo(platform, param_name, 0, NULL, &infoSize);
//	if (clStatus != CL_SUCCESS)
//	{
//		// error
//		return std::string(cl_err_to_str(clStatus));
//	}
//	std::vector<char> strBuff(infoSize);
//	clStatus = clGetPlatformInfo(platform, param_name, infoSize, strBuff.data(), NULL);
//	if (clStatus == CL_SUCCESS)
//	{
//		if (strBuff.size() == 0 || strBuff.data() == nullptr) return "";
//		return std::string(strBuff.data());
//	}
//
//	// error
//	return std::string(cl_err_to_str(clStatus));
//}
//
//std::string getClGetDeviceInfoString(cl_device_id device, cl_device_info  param_name)
//{
//	cl_int clStatus;
//
//	size_t infoSize;
//	clStatus = clGetDeviceInfo(device, param_name, 0, NULL, &infoSize);
//	if (clStatus != CL_SUCCESS)
//	{
//		// error
//		return std::string(cl_err_to_str(clStatus));
//	}
//	std::vector<char> strBuff(infoSize);
//	clStatus = clGetDeviceInfo(device, param_name, infoSize, strBuff.data(), NULL);
//	if (clStatus == CL_SUCCESS)
//	{
//		if (strBuff.size() == 0 || strBuff.data() == nullptr) return "";
//		return std::string(strBuff.data());
//	}
//
//	// error
//	return std::string(cl_err_to_str(clStatus));
//}
//
//bool AMDOpenCLDeviceDetection::QueryDevices() {
//	int amdPlatformID = -1;
//	try {
//		// get platforms
//		cl_uint numPlatforms = 0;
//		cl_int clStatus;
//
//		// Get platform and device information
//		clStatus = clGetPlatformIDs(0, NULL, &numPlatforms);
//		if (clStatus != CL_SUCCESS)
//		{
//			_statusString = std::string(cl_err_to_str(clStatus)) + " when calling clGetPlatformIDs for number of platforms.";
//			cout << _statusString << endl;
//			return false;
//		}
//		if (numPlatforms == 0)
//		{
//			_statusString = "No OpenCL platforms found";
//			cout << "No OpenCL platforms found" << endl;
//			return false;
//		}
//
//		std::vector<cl_platform_id> platforms;
//		platforms.resize(numPlatforms);
//		clStatus = clGetPlatformIDs(numPlatforms, platforms.data(), NULL);
//
//		if (clStatus != CL_SUCCESS) {
//			_statusString = std::string(cl_err_to_str(clStatus)) + " when calling clGetPlatformIDs for platform information.";
//			cout << _statusString << endl;
//			return false;
//		}
//
//		// iterate platforms
//		for (unsigned int i = 0; i < numPlatforms; i++)
//		{
//			std::string platformName = getClGetDeviceInfoString(platforms[i], CL_PLATFORM_VENDOR);
//			std::string platformName2 = getClGetDeviceInfoString(platforms[i], CL_PLATFORM_NAME);
//
//			bool isAMDOpenCL = platformName.find("Advanced Micro Devices") != std::string::npos ||
//				platformName.find("Apple") != std::string::npos ||
//				platformName.find("Mesa") != std::string::npos;
//			bool isNVIDIADevice = platformName.find("NVIDIA Corporation") != std::string::npos || platformName.find("NVIDIA") != std::string::npos;
//
//			OpenCLPlatform current;
//			current.PlatformName = platformName2;
//			current.PlatformVendor = platformName;
//			current.PlatformNum = i;
//
//			// devices per plaftorm
//			cl_uint num_devices;
//			std::vector<cl_device_id> device_list;
//			if ((clStatus = clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_GPU, 0, NULL, &num_devices)) != CL_SUCCESS)
//			{
//				continue;
//			}
//			device_list.resize(num_devices);
//			if ((clStatus = clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_GPU, num_devices, device_list.data(), NULL)) != CL_SUCCESS)
//			{
//				continue;
//			}
//			// iterate platform devs
//			for (size_t k = 0; k < num_devices; k++)
//			{
//				OpenCLDevice curDevice;
//				curDevice.DeviceID = (int)k;
//
//				// CL_DEVICE_NAME
//				curDevice._CL_DEVICE_NAME = getClGetDeviceInfoString(device_list[k], CL_DEVICE_NAME);
//
//				// CL_DEVICE_VENDOR
//				std::string vendor = getClGetDeviceInfoString(device_list[k], CL_DEVICE_VENDOR);
//				bool isAMDDevice = vendor.find("Advanced Micro Devices") != std::string::npos || vendor.find("AMD") != std::string::npos;
//				bool isNVIDIADevice = vendor.find("NVIDIA Corporation") != std::string::npos || vendor.find("NVIDIA") != std::string::npos;
//				curDevice._CL_DEVICE_NAME = vendor;
//				
//				// CL_DEVICE_VERSION
//				curDevice._CL_DEVICE_VERSION = getClGetDeviceInfoString(device_list[k], CL_DEVICE_VERSION);
//
//				// CL_DRIVER_VERSION
//				curDevice._CL_DRIVER_VERSION = getClGetDeviceInfoString(device_list[k], CL_DRIVER_VERSION);
//
//				// CL_DEVICE_TYPE
//				cl_device_type type;
//				if ((clStatus = clGetDeviceInfo(device_list[k], CL_DEVICE_TYPE, sizeof(cl_device_type), &type, NULL)) == CL_SUCCESS)
//				{
//					switch (type) { 
//					case CL_DEVICE_TYPE_CPU:
//						curDevice._CL_DEVICE_TYPE = "CPU";
//						break;
//					case CL_DEVICE_TYPE_GPU:
//						curDevice._CL_DEVICE_TYPE = "GPU";
//						break;
//					case CL_DEVICE_TYPE_ACCELERATOR:
//						curDevice._CL_DEVICE_TYPE = "ACCELERATOR";
//						break;
//					default:
//						curDevice._CL_DEVICE_TYPE = "DEFAULT";
//						break;
//					}
//				}
//				else {
//					// TODO log or add to error?
//				}
//
//				// CL_DEVICE_GLOBAL_MEM_SIZE
//				if ((clStatus = clGetDeviceInfo(device_list[k], CL_DEVICE_GLOBAL_MEM_SIZE, sizeof(size_t), &(curDevice._CL_DEVICE_GLOBAL_MEM_SIZE), NULL)) != CL_SUCCESS)
//				{
//					// TODO log or add to error?
//				}
//
//				// AMD extensions
//				if (isAMDDevice) {
//					// CL_DEVICE_TOPOLOGY_AMD
//					cl_device_topology_amd topology = {};
//					if ((clStatus = clGetDeviceInfo(device_list[k], CL_DEVICE_TOPOLOGY_AMD, sizeof(cl_device_topology_amd), &(curDevice._CL_DEVICE_GLOBAL_MEM_SIZE), NULL)) == CL_SUCCESS)
//					{
//						if (topology.raw.type == CL_DEVICE_TOPOLOGY_TYPE_PCIE_AMD) {
//							curDevice.AMD_BUS_ID = (int)topology.pcie.bus;
//						}
//					}
//					else
//					{
//						// TODO log or add to error?
//					}
//
//					// CL_DEVICE_BOARD_NAME_AMD
//					curDevice._CL_DEVICE_BOARD_NAME_AMD = getClGetDeviceInfoString(device_list[k], CL_DEVICE_BOARD_NAME_AMD);
//				}
//
//				current.Devices.push_back(curDevice);
//			}
//			_devicesPlatformsDevices.push_back(current);
//		}
//	}
//	catch (exception &ex) {
//		_errorString = ex.what();
//		_statusString = "Error";
//		return false;
//	}
//
//	return true;
//}