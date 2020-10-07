rmdir /s /q device_detection_x64 
mkdir device_detection_x64
mkdir device_detection_x64\OpenCL

copy .\3rdPartyLibs\OpenCL\OpenCL.dll .\device_detection_x64\OpenCL\OpenCL.dll
copy .\3rdPartyLibs\OpenCL\README.md .\device_detection_x64\OpenCL\README.md

copy .\scripts\device_detection.bat .\device_detection_x64\device_detection.bat

copy .\x64\Release\device_detection.exe .\device_detection_x64\device_detection.exe
copy .\x64\Release\device_detection_cpu.dll .\device_detection_x64\device_detection_cpu.dll
copy .\x64\Release\device_detection_cuda_nvml.dll .\device_detection_x64\device_detection_cuda_nvml.dll
copy .\x64\Release\device_detection_opencl.dll .\device_detection_x64\device_detection_opencl.dll
