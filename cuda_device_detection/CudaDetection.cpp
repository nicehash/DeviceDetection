#include "CudaDetection.h"

#include <iostream>
#include <stdexcept>

#include "nvidia_nvml_helper.h"
#include "cuda_helper.h"
#include "json_helpers.h"

using namespace std;

CudaDetection::CudaDetection() { }
CudaDetection::~CudaDetection() { }

#define PCI_BUS_LEN 64

bool CudaDetection::QueryDevices(bool useNvmlFallback) {
	try {
		int device_count;
		CUDA_SAFE_CALL(cudaGetDeviceCount(&device_count));
		_isNvmlInitialized = nvidia_nvml_helper::SafeNVMLInit();
		if (!_isNvmlInitialized && useNvmlFallback) {
			_isNvmlInitializedFallback = nvidia_nvml_helper::SafeNVMLInitFallback();
		}
		for (int i = 0; i < device_count; ++i) {
			CudaDevice cudaDevice;

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

			_cudaDevices.push_back(cudaDevice);
		}
		_driverVersionStr = nvidia_nvml_helper::GetDriverVersionSafe();
		nvidia_nvml_helper::SafeNVMLShutdown();
	}
	catch (runtime_error &err) {
		_errorString = err.what();
		//_errorMsgs.push_back(err.what());
		return false;
	}
	return true;
}


string CudaDetection::GetDevicesJsonString(bool prettyPrint) {
	return json_helpers::GetCUDADevicesJsonString(_cudaDevices, _driverVersionStr, _errorString, _isNvmlInitialized, _isNvmlInitializedFallback, prettyPrint);
}

string CudaDetection::GetErrorString() {
	return _errorString;
}

std::string CudaDetection::GetDriverVersion() {
	return _driverVersionStr;
}
