#pragma once
#include <cstdint>

namespace Buttplug
{
    using Callback = void(*)(void* ctx, uint8_t* buf, int32_t bufLen);
    using ClientHandle = void*;
    using DeviceHandle = void*;

    using CreateProtobufClient = Buttplug::ClientHandle(*)(const char* clientName, Buttplug::Callback cb, void* ctx);
    using FreeClient = void(*)(Buttplug::ClientHandle clientHandle);
    using ClientProtobufMessage = void(*)(Buttplug::ClientHandle clientHandle, uint8_t* buf, int32_t bufLen, Buttplug::Callback cb, void* ctx);
    
    using CreateDevice = Buttplug::DeviceHandle(*)(Buttplug::ClientHandle clientHandle, uint32_t deviceIndex);
    using FreeDevice = void(*)(Buttplug::DeviceHandle deviceHandle);
    using DeviceProtobufMessage = void(*)(Buttplug::DeviceHandle deviceHandle, uint8_t* buf, int32_t bufLen);
    
    using ActivateEnvLogger = void(*)();

    struct FFI
    {
        static Buttplug::CreateProtobufClient CreateProtobufClient;
        static Buttplug::FreeClient FreeClient;
        static Buttplug::ClientProtobufMessage ClientProtobufMessage;

        static Buttplug::CreateDevice CreateDevice;
        static Buttplug::FreeDevice FreeDevice;
        static Buttplug::DeviceProtobufMessage DeviceProtobufMessage;

        static Buttplug::ActivateEnvLogger ActivateEnvLogger;

        static bool Init();
    };
} // namespace Buttplug
