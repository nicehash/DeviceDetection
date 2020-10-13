#include <iostream>
#include <cstring>
#include <string>
#include <string_view>
#include <memory>

#include <windows.h>

#include "json.hpp"

enum class DetectionType {
	UNKNOWN,
	CPU,
	CUDA,
	OPEN_CL,
	ALL
};

void show_help() {
	std::cout << "Error usage [cpu|ocl|cuda|all] [-p] [-n]" << std::endl;
}

DetectionType get_detection_type(const std::string_view first_arg) {
	if (first_arg.find("cpu") != std::string_view::npos) return DetectionType::CPU;
	if (first_arg.find("cuda") != std::string_view::npos) return DetectionType::CUDA;
	if (first_arg.find("ocl") != std::string_view::npos) return DetectionType::OPEN_CL;
	if (first_arg.find("all") != std::string_view::npos) return DetectionType::ALL;
	return DetectionType::UNKNOWN;
}

bool contains_param(const std::string_view arg, const std::string_view sub) {
	return arg.find(sub) != std::string_view::npos;
}

class DetectCpu
{
	typedef const char* (*cpu_detection_json_result_str_t)(bool pretty_print);
private:
	HINSTANCE dll = NULL;
	bool try_load = false;
	cpu_detection_json_result_str_t cpu_detection_json_result_str = nullptr;
public:
	~DetectCpu() {
		if (dll != NULL) FreeLibrary(dll);
	}
	std::string detect(bool pretty_print) {
		if (!try_load) {
			try_load = true;
			dll = LoadLibraryA("device_detection_cpu.dll");
			if (dll == NULL) return "Cannot load 'device_detection_cpu.dll'";
			cpu_detection_json_result_str = (cpu_detection_json_result_str_t)GetProcAddress(dll, "cpu_detection_json_result_str");
		}
		if (try_load && cpu_detection_json_result_str != nullptr) return cpu_detection_json_result_str(pretty_print);
		return "Loaded library 'device_detection_cpu.dll' missing 'cpu_detection_json_result_str'";
	}
};

std::string detect_cpu(bool pretty_print) {
	DetectCpu d;
	return d.detect(pretty_print);
}

class DetectCuda
{
	typedef const char* (*cuda_device_detection_json_result_str_t)(bool pretty_print);
private:
	HINSTANCE dll = NULL;
	bool try_load = false;
	cuda_device_detection_json_result_str_t cuda_device_detection_json_result_str = nullptr;
public:
	~DetectCuda() {
		if (dll != NULL) FreeLibrary(dll);
	}
	std::string detect(bool pretty_print) {
		if (!try_load) {
			try_load = true;
			dll = LoadLibraryA("device_detection_cuda_nvml.dll");
			if (dll == NULL) return "Cannot load 'device_detection_cuda_nvml.dll'";
			cuda_device_detection_json_result_str = (cuda_device_detection_json_result_str_t)GetProcAddress(dll, "cuda_device_detection_json_result_str");
		}
		if (try_load && cuda_device_detection_json_result_str != nullptr) return cuda_device_detection_json_result_str(pretty_print);
		return "Loaded library 'device_detection_cuda_nvml.dll' missing 'cuda_device_detection_json_result_str'";
	}
};

std::string detect_cuda(bool pretty_print) {
	DetectCuda d;
	return d.detect(pretty_print);
}

class DetectOpenCL
{
	typedef const char* (*open_cl_device_detection_json_result_str_t)(bool pretty_print);
private:
	HINSTANCE dll = NULL;
	bool try_load = false;
	open_cl_device_detection_json_result_str_t open_cl_device_detection_json_result_str = nullptr;
public:
	~DetectOpenCL() {
		if (dll != NULL) FreeLibrary(dll);
	}
	std::string detect(bool pretty_print) {
		if (!try_load) {
			try_load = true;
			dll = LoadLibraryA("device_detection_opencl.dll");
			if (dll == NULL) return "Cannot load 'device_detection_opencl.dll'";
			open_cl_device_detection_json_result_str = (open_cl_device_detection_json_result_str_t)GetProcAddress(dll, "open_cl_device_detection_json_result_str");
		}
		if (try_load && open_cl_device_detection_json_result_str != nullptr) return open_cl_device_detection_json_result_str(pretty_print);
		return "Loaded library 'device_detection_opencl.dll' missing 'open_cl_device_detection_json_result_str'";
	}
};
std::string detect_opencl(bool pretty_print) {
	DetectOpenCL d;
	return d.detect(pretty_print);
}

int main(int argc, char* argv[]) {
	auto detect = argc < 2 ? DetectionType::UNKNOWN : get_detection_type(argv[1]);
	if (detect == DetectionType::UNKNOWN) {
		show_help();
		return 0;
	}

	bool is_pretty_print = false;
	bool is_no_newline = false;
	for (auto i = 2; i < argc; i++) {
		if (contains_param(argv[i], "-p")) is_pretty_print = true;
		if (contains_param(argv[i], "-n")) is_no_newline = true;
	}

	if (detect == DetectionType::CPU) {
		std::cout << detect_cpu(is_pretty_print);
	}
	if (detect == DetectionType::CUDA) {
		std::cout << detect_cuda(is_pretty_print);
	}
	if (detect == DetectionType::OPEN_CL) {
		std::cout << detect_opencl(is_pretty_print);
	}
	if (detect == DetectionType::ALL) {
		nlohmann::json all = nlohmann::json::array();
		auto all_append = [&all](std::string& j) {
			try
			{
				auto add = nlohmann::json::parse(j);
				all.emplace_back(add);
			}
			catch (const std::exception&)
			{
				all.emplace_back(j);
			}
		};
		all_append(detect_cpu(false));
		all_append(detect_cuda(false));
		all_append(detect_opencl(false));
		auto json_str = is_pretty_print ? all.dump(4) : all.dump();
		std::cout << json_str;
	}
	if (!is_no_newline) std::cout << std::endl;
	return 0;
}

