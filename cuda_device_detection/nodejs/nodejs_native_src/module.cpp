#include <nan.h>
#include <string>

#include "CudaDetection.h"

// using namespace v8;
using v8::FunctionTemplate;
using namespace std;
using namespace v8;
//using namespace nan;

// NativeExtension.cc represents the top level of the module.
// C++ constructs that are exposed to javascript are exported here

class CUDADetectionWorker : public Nan::AsyncWorker {
public:
    CUDADetectionWorker(Nan::Callback *callback) : Nan::AsyncWorker(callback) {
    }
    ~CUDADetectionWorker() {
    }

    void Execute () {
        CudaDetection detection;
        _isSuccess = detection.QueryDevices();
        _result = detection.GetDevicesJsonString();
    }
    void HandleOKCallback () {
        Nan::HandleScope scope;

        Local<Value> argv[] = {
            Nan::New<v8::Boolean>(_isSuccess),
            Nan::New<v8::String>(_result).ToLocalChecked()
        };

        callback->Call(2, argv);

    }

private:
    bool _isSuccess = false;
    std::string _result = "";
};

NAN_METHOD(PrintCudaDevices) {
    Nan::HandleScope scope;
    CudaDetection detection;
    if (detection.QueryDevices()) {
        detection.PrintDevicesJson();
    }
}

NAN_METHOD(GetDevicesJsonStringSync) {
    Nan::HandleScope scope;
    CudaDetection detection;
    detection.QueryDevices();
    string s = detection.GetDevicesJsonString();
    info.GetReturnValue().Set(Nan::New<String>(s.c_str()).ToLocalChecked());
}

NAN_METHOD(GetDevicesJsonString) {
    Nan::Callback *callback = new Nan::Callback(info[0].As<Function>());
    AsyncQueueWorker(new CUDADetectionWorker(callback));
}

// wrap native functions

NAN_MODULE_INIT(InitAll) {
  Nan::Set(target, Nan::New("PrintCudaDevices").ToLocalChecked(),
    Nan::GetFunction(Nan::New<FunctionTemplate>(PrintCudaDevices)).ToLocalChecked());
  Nan::Set(target, Nan::New("GetDevicesJsonStringSync").ToLocalChecked(),
    Nan::GetFunction(Nan::New<FunctionTemplate>(GetDevicesJsonStringSync)).ToLocalChecked());
  Nan::Set(target, Nan::New("GetDevicesJsonString").ToLocalChecked(),
    Nan::GetFunction(Nan::New<FunctionTemplate>(GetDevicesJsonString)).ToLocalChecked());
}

NODE_MODULE(nhm2_detect_cuda, InitAll)