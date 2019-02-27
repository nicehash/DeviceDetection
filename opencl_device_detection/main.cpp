#include "ocl_dev_detection.h"
#include <iostream>

int main(int argc, char* argv[]) {
	const auto prettyPrint = argc >= 2; 
	std::cout << _GetOpenCLDevices(prettyPrint);

	return 0;
}

