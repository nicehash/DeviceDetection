# opencl_device_detection

opencl_device_detection is a win32 program that prints OpenCL platform numbers and available OpenCL GPU devices in JSON format, with aditional AMD topology detection. In order to build this project you should have [AMD APP SDK 3.0](https://developer.amd.com/tools-and-sdks/opencl-zone/amd-accelerated-parallel-processing-app-sdk/) installed. Supports Windows x86, x64. Make sure that the binary can link to **OpenCL.dll**.

## TODO:
  - fix linux build
  - fix nodejs bindings
  - load OpenCL dynamically