#pragma once

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

	static std::string StringnNullTerminatorFix(const std::string& str);
};
