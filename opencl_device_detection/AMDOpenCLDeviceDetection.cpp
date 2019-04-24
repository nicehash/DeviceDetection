#include "AMDOpenCLDeviceDetection.h"

#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <utility>

#include "cl_ext.h"
#include "json_helpers.h"

using namespace std;

AMDOpenCLDeviceDetection::AMDOpenCLDeviceDetection()
{
}

AMDOpenCLDeviceDetection::~AMDOpenCLDeviceDetection()
{
}


string AMDOpenCLDeviceDetection::StringnNullTerminatorFix(const string& str) {
	return string(str.c_str(), strlen(str.c_str()));
}

bool AMDOpenCLDeviceDetection::QueryDevices() {
	int amdPlatformID = -1;
	try {
		// get platforms
		cl_uint numPlatforms = 0;
		cl_int clStatus;

		// Get platform and device information
		clStatus = clGetPlatformIDs(0, NULL, &numPlatforms);
		if (clStatus != CL_SUCCESS)
		{
			_statusString = std::string(cl_err_to_str(clStatus)) + " when calling clGetPlatformIDs for number of platforms.";
			cout << _statusString << endl;
			return false;
		}
		if (numPlatforms == 0)
		{
			_statusString = "No OpenCL platforms found";
			cout << "No OpenCL platforms found" << endl;
			return false;
		}

		std::vector<cl_platform_id> platforms;
		platforms.resize(numPlatforms);
		clStatus = clGetPlatformIDs(numPlatforms, platforms.data(), NULL);

		if (clStatus != CL_SUCCESS) {
			_statusString = std::string(cl_err_to_str(clStatus)) + " when calling clGetPlatformIDs for platform information.";
			cout << _statusString << endl;
			return false;
		}

		// iterate platforms
		for (unsigned int i = 0; i < numPlatforms; i++)
		{
			size_t infoSize;
			clGetPlatformInfo(platforms[i], CL_PLATFORM_VENDOR, 0, NULL, &infoSize);
			std::vector<char> platformNameVec(infoSize);
			clGetPlatformInfo(platforms[i], CL_PLATFORM_VENDOR, infoSize, platformNameVec.data(), NULL);
			std::string platformName(platformNameVec.data());

			size_t infoSize2;
			clGetPlatformInfo(platforms[i], CL_PLATFORM_NAME, 0, NULL, &infoSize2);
			std::vector<char> platformNameVec2(infoSize);
			clGetPlatformInfo(platforms[i], CL_PLATFORM_NAME, infoSize2, platformNameVec2.data(), NULL);
			std::string platformName2(platformNameVec2.data());

			
			

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
				std::vector<char> strBuffer(1024);
				OpenCLDevice curDevice;
				curDevice.DeviceID = (int)k;

				// CL_DEVICE_NAME
				if ((clStatus = clGetDeviceInfo(device_list[k], CL_DEVICE_NAME, strBuffer.size(), strBuffer.data(), NULL)) == CL_SUCCESS)
				{
					std::string name(strBuffer.data());
					curDevice._CL_DEVICE_NAME = StringnNullTerminatorFix(name);
				}
				else {
					// TODO log or add to error?
				}

				// CL_DEVICE_VENDOR
				bool isAMDDevice = false;
				bool isNVIDIADevice = false;
				if ((clStatus = clGetDeviceInfo(device_list[k], CL_DEVICE_VENDOR, strBuffer.size(), strBuffer.data(), NULL)) == CL_SUCCESS)
				{
					std::string vendor(strBuffer.data());
					isAMDDevice = vendor.find("Advanced Micro Devices") != std::string::npos || vendor.find("AMD") != std::string::npos;
					isNVIDIADevice = vendor.find("NVIDIA Corporation") != std::string::npos || vendor.find("NVIDIA") != std::string::npos;
					curDevice._CL_DEVICE_VENDOR = StringnNullTerminatorFix(vendor);
				}
				else {

				}
				// CL_DEVICE_VERSION
				if ((clStatus = clGetDeviceInfo(device_list[k], CL_DEVICE_VERSION, strBuffer.size(), strBuffer.data(), NULL)) == CL_SUCCESS)
				{
					std::string version(strBuffer.data());
					curDevice._CL_DEVICE_VERSION = StringnNullTerminatorFix(version);
				}
				else {
					// TODO log or add to error?
				}

				// CL_DRIVER_VERSION
				if ((clStatus = clGetDeviceInfo(device_list[k], CL_DRIVER_VERSION, strBuffer.size(), strBuffer.data(), NULL)) == CL_SUCCESS)
				{
					std::string version(strBuffer.data());
					curDevice._CL_DRIVER_VERSION = StringnNullTerminatorFix(version);
				}
				else {
					// TODO log or add to error?
				}

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

				}

				// CL_DEVICE_GLOBAL_MEM_SIZE
				if ((clStatus = clGetDeviceInfo(device_list[k], CL_DEVICE_GLOBAL_MEM_SIZE, sizeof(size_t), &(curDevice._CL_DEVICE_GLOBAL_MEM_SIZE), NULL)) != CL_SUCCESS)
				{
					// TODO log or add to error?
				}

				// AMD extensions
				if (isAMDDevice) {
					// CL_DEVICE_TOPOLOGY_AMD
					cl_device_topology_amd topology = {};
					if ((clStatus = clGetDeviceInfo(device_list[k], CL_DEVICE_TOPOLOGY_AMD, sizeof(cl_device_topology_amd), &(curDevice._CL_DEVICE_GLOBAL_MEM_SIZE), NULL)) == CL_SUCCESS)
					{
						if (topology.raw.type == CL_DEVICE_TOPOLOGY_TYPE_PCIE_AMD) {
							curDevice.AMD_BUS_ID = (int)topology.pcie.bus;
						}
					}
					else
					{
						// TODO log or add to error?
					}

					// CL_DEVICE_BOARD_NAME_AMD
					if ((clStatus = clGetDeviceInfo(device_list[k], CL_DEVICE_BOARD_NAME_AMD, strBuffer.size(), strBuffer.data(), NULL)) == CL_SUCCESS)
					{
						std::string boardName(strBuffer.data());
						curDevice._CL_DEVICE_BOARD_NAME_AMD = StringnNullTerminatorFix(boardName);
					}
				}

				current.Devices.push_back(curDevice);
			}
			_devicesPlatformsDevices.push_back(current);
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