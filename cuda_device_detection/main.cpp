#include "cuda_dev_detection.h"
#include <iostream>

int main(int argc, char* argv[]) {
	const auto prettyPrint = argc >= 2; 
	const auto nvmlFallback = argc >= 3;
	std::cout << _GetCUDADevices(prettyPrint, nvmlFallback);
	return 0;
}

