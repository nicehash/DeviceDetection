#include "cuda_dev_detection.h"
#include <iostream>

int main(int argc, char* argv[]) {
	const auto prettyPrint = argc >= 2; 
	std::cout << _GetCUDADevices(prettyPrint);
	return 0;
}

