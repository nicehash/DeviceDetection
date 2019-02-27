#include <nan.h>
#include <string>

#include "AMDOpenCLDeviceDetection.h"

// using namespace v8;
using v8::FunctionTemplate;
using namespace std;
using namespace v8;
//using namespace nan;

// NativeExtension.cc represents the top level of the module.
// C++ constructs that are exposed to javascript are exported here

class AMDDetectionWorker : public Nan::AsyncWorker {
public:
    AMDDetectionWorker(Nan::Callback *callback) : Nan::AsyncWorker(callback) {
    }
    ~AMDDetectionWorker() {
    }

    void Execute () {
        AMDOpenCLDeviceDetection AMDOpenCLDeviceDetection;
        _isSuccess = AMDOpenCLDeviceDetection.QueryDevices();
        _result = AMDOpenCLDeviceDetection.GetDevicesJsonString();
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

NAN_METHOD(PrintAMDDevices) {
    Nan::HandleScope scope;
    AMDOpenCLDeviceDetection AMDOpenCLDeviceDetection;
    if (AMDOpenCLDeviceDetection.QueryDevices()) {
        AMDOpenCLDeviceDetection.PrintDevicesJson();
    }
}

NAN_METHOD(GetAMDOpenCLDevicesJSONStringSync) {
    Nan::HandleScope scope;
    AMDOpenCLDeviceDetection AMDOpenCLDeviceDetection;
    AMDOpenCLDeviceDetection.QueryDevices();
    string s = AMDOpenCLDeviceDetection.GetDevicesJsonString();
    info.GetReturnValue().Set(Nan::New<String>(s.c_str()).ToLocalChecked());
}

NAN_METHOD(GetAMDOpenCLDevicesJSONString) {
    Nan::Callback *callback = new Nan::Callback(info[0].As<Function>());
    AsyncQueueWorker(new AMDDetectionWorker(callback));
}

// wrap native functions

NAN_MODULE_INIT(InitAll) {
  Nan::Set(target, Nan::New("PrintAMDDevices").ToLocalChecked(),
    Nan::GetFunction(Nan::New<FunctionTemplate>(PrintAMDDevices)).ToLocalChecked());
  Nan::Set(target, Nan::New("GetAMDOpenCLDevicesJSONStringSync").ToLocalChecked(),
    Nan::GetFunction(Nan::New<FunctionTemplate>(GetAMDOpenCLDevicesJSONStringSync)).ToLocalChecked());
  Nan::Set(target, Nan::New("GetAMDOpenCLDevicesJSONString").ToLocalChecked(),
    Nan::GetFunction(Nan::New<FunctionTemplate>(GetAMDOpenCLDevicesJSONString)).ToLocalChecked());
}

NODE_MODULE(nhm2_detect_amd, InitAll)