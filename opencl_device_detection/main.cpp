#include "opencl_device_detection.h"
#include <iostream>

int main(int argc, char* argv[]) {
	const auto prettyPrint = argc >= 2; 
	std::cout << open_cl_device_detection_json_result_str(prettyPrint);

	return 0;
}

