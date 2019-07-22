#pragma once

#define __CL_ENABLE_EXCEPTIONS
#define CL_USE_DEPRECATED_OPENCL_2_0_APIS
//#define CL_VERSION_1_2
#define CL_USE_DEPRECATED_OPENCL_1_1_APIS
#include "CL/cl_ext.h"

#include "OpenCLDevice.h"

class AMDOpenCLDeviceDetection {
public:
	AMDOpenCLDeviceDetection();
	~AMDOpenCLDeviceDetection();

	bool QueryDevices();
	std::string GetDevicesJsonString(bool prettyPrint = false);
	std::string GetErrorString();

private:
	std::vector<OpenCLPlatform> _devicesPlatformsDevices;

	std::string _errorString = "";
	std::string _statusString = "OK"; // assume ok

	void AppendToErrorString(cl_int ret);
};
