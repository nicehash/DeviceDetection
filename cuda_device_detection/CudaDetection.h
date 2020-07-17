#pragma once

#include "CudaDevice.h"
#include <vector>
#include <string>

class CudaDetection
{
public:
	CudaDetection();
	~CudaDetection();

	bool QueryDevices();
	std::string GetDevicesJsonString(bool prettyPrint = false);
	std::string GetErrorString();
	std::string GetDriverVersion();

private:
	std::string _errorString = "";
	//std::vector<std::string> _errorMsgs;
	std::vector<CudaDevice> _cudaDevices;
	// driver version
	std::string _driverVersionStr = "";
	bool _isNvmlInitialized = false;
};

