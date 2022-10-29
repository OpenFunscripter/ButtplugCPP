#include "ButtplugFFI.h"
#include "dylib.hpp"
#include <memory>

using namespace Buttplug;
static std::unique_ptr<dylib> ButtplugFFILib = nullptr;

Buttplug::CreateProtobufClient FFI::CreateProtobufClient = nullptr;
Buttplug::FreeClient FFI::FreeClient = nullptr;
Buttplug::ClientProtobufMessage FFI::ClientProtobufMessage = nullptr;
Buttplug::CreateDevice FFI::CreateDevice = nullptr;
Buttplug::FreeDevice FFI::FreeDevice = nullptr;
Buttplug::DeviceProtobufMessage FFI::DeviceProtobufMessage = nullptr;
Buttplug::ActivateEnvLogger FFI::ActivateEnvLogger = nullptr;

bool FFI::Init() 
{
    if(!ButtplugFFILib) {
        ButtplugFFILib = std::make_unique<dylib>("./", "buttplug_rs_ffi");
    }

    if(!ButtplugFFILib->has_symbol("buttplug_create_protobuf_client")) 
        return false;
    if(!ButtplugFFILib->has_symbol("buttplug_free_client")) 
        return false;
    if(!ButtplugFFILib->has_symbol("buttplug_client_protobuf_message")) 
        return false;
    if(!ButtplugFFILib->has_symbol("buttplug_create_device")) 
        return false;
    if(!ButtplugFFILib->has_symbol("buttplug_device_protobuf_message")) 
        return false;
    if(!ButtplugFFILib->has_symbol("buttplug_free_device")) 
        return false;
    if(!ButtplugFFILib->has_symbol("buttplug_activate_env_logger")) 
        return false;

    CreateProtobufClient = reinterpret_cast<Buttplug::CreateProtobufClient>(
        ButtplugFFILib->get_symbol("buttplug_create_protobuf_client")
    );
    FreeClient = reinterpret_cast<Buttplug::FreeClient>(
        ButtplugFFILib->get_symbol("buttplug_free_client")
    );
    ClientProtobufMessage = reinterpret_cast<Buttplug::ClientProtobufMessage>(
        ButtplugFFILib->get_symbol("buttplug_client_protobuf_message")
    );

    CreateDevice = reinterpret_cast<Buttplug::CreateDevice>(
        ButtplugFFILib->get_symbol("buttplug_create_device")
    );
    FreeDevice = reinterpret_cast<Buttplug::FreeDevice>(
        ButtplugFFILib->get_symbol("buttplug_free_device")
    );
    DeviceProtobufMessage = reinterpret_cast<Buttplug::DeviceProtobufMessage>(
        ButtplugFFILib->get_symbol("buttplug_device_protobuf_message")
    );
    ActivateEnvLogger = reinterpret_cast<Buttplug::ActivateEnvLogger>(
        ButtplugFFILib->get_symbol("buttplug_activate_env_logger")
    );
    return true;
}
