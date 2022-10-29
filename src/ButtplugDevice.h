#pragma once

#include "ButtplugFFI.h"

#include <string>
#include <future>
#include <memory>

namespace google
{
    namespace protobuf
    {
        class Arena;
    }
}

namespace Buttplug
{
    class Client;
    class DeviceMessage;

    class Device
    {
        private:
        Buttplug::Client* client = nullptr;
        Buttplug::DeviceHandle deviceHandle = nullptr;
        std::unique_ptr<google::protobuf::Arena> arena;
        std::string name;

        void sendMessage(Buttplug::DeviceMessage* msg);
        public:
        Device(const std::string& name, Buttplug::Client* client, Buttplug::DeviceHandle deviceHandle);
        Device(const Device&) = delete;
        Device(Device&&) = delete;
        ~Device();

        const std::string& Name() const noexcept { return name; }

        void SendLinearCmd(uint32_t durationMs, double position);
        void SendVibrateCmd(double speed);

    };
} // namespace Buttplug
