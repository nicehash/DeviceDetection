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

bool CudaDetection::QueryDevices() {
	try {
		int device_count;
		CUDA_SAFE_CALL(cudaGetDeviceCount(&device_count));
		nvidia_nvml_helper::SafeNVMLInit(); // NVML_SAFE_CALL(nvmlInit());
		for (int i = 0; i < device_count; ++i) {
			CudaDevice cudaDevice;

			cudaDeviceProp props;
			CUDA_SAFE_CALL(cudaGetDeviceProperties(&props, i));
			char pciBusID[PCI_BUS_LEN];
			CUDA_SAFE_CALL(cudaDeviceGetPCIBusId(pciBusID, PCI_BUS_LEN, i));

			// init serial vendor stuff
			nvidia_nvml_helper::SetCudaDeviceAttributes(pciBusID, cudaDevice);

			// init device info
			cudaDevice.DeviceID = i;
			cudaDevice.pciBusID = props.pciBusID;
			//cudaDevice.VendorName = getVendorString(pciInfo);
			cudaDevice.DeviceName = props.name;
			cudaDevice.SM_major = props.major;
			cudaDevice.SM_minor = props.minor;
			//cudaDevice.UUID = uuid;
			cudaDevice.DeviceGlobalMemory = props.totalGlobalMem;
			//cudaDevice.pciDeviceId = pciInfo.pciDeviceId;
			//cudaDevice.pciSubSystemId = pciInfo.pciSubSystemId;
			cudaDevice.SMX = props.multiProcessorCount;
			//cudaDevice.VendorID = getVendorId(pciInfo);

			_cudaDevices.push_back(cudaDevice);
		}
		_driverVersionStr = nvidia_nvml_helper::GetDriverVersionSafe();
		nvidia_nvml_helper::SafeNVMLShutdown(); // NVML_SAFE_CALL(nvmlShutdown());
	}
	catch (runtime_error &err) {
		_errorString = err.what();
		//_errorMsgs.push_back(err.what());
		return false;
	}
	return true;
}


string CudaDetection::GetDevicesJsonString(bool prettyPrint) {
	return json_helpers::GetCUDADevicesJsonString(_cudaDevices, _driverVersionStr, _errorString, prettyPrint); 
}

string CudaDetection::GetErrorString() {
	return _errorString;
}

std::string CudaDetection::GetDriverVersion() {
	return _driverVersionStr;
}
