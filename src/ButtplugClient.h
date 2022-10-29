#pragma once
#include "ButtplugFFI.h"
#include "ButtplugDevice.h"

#include <string>
#include <memory>
#include <map>
#include <functional>
#include <mutex>
#include <future>
#include <optional>

namespace Buttplug
{
    class Protocol;
    class ButtplugFFIServerMessage;
    class Client;

    using ServerResponseFunc = std::function<void(const Buttplug::ButtplugFFIServerMessage&, std::promise<bool>&)>;

    // Callbacks
    using DeviceAddedCallback = std::function<void(std::weak_ptr<Buttplug::Device> device)>;
    using DeviceRemovedCallback = std::function<void(std::weak_ptr<Buttplug::Device> device)>;
    using ErrorReceivedCallback = std::function<void(const std::string& error)>;
    using ScanningFinishedCallback = std::function<void()>;
    using PingTimeoutCallback = std::function<void()>;
    using ServerDisconnectCallback = std::function<void()>;

    struct ServerResponseHandler
    {
        ServerResponseFunc Handler = [](const Buttplug::ButtplugFFIServerMessage&, std::promise<bool>& p) { p.set_value(true); };
        std::promise<bool> Promise;
    };

    class Client
    {
        private:
        friend class Buttplug::Device;

        std::unique_ptr<Buttplug::Protocol> protocol;
        ClientHandle clientHandle = nullptr;
        std::map<uint32_t, std::shared_ptr<Buttplug::Device>> devices;

        std::mutex responseMtx;
        std::map<uint32_t, ServerResponseHandler> responeHandlerMap;
        std::string name;
        bool isConnected = false;

        std::promise<bool>& addResponseHandler(uint32_t id, ServerResponseFunc&& handler);
        public:
        Client(const std::string& clientName);
        Client(const Client&) = delete;
        Client(Client&&) = delete;
        ~Client();

        std::future<bool> ConnectWebsocket(const std::string& address, bool bypass_ssl);
        std::future<bool> StartScanning();
        std::future<bool> Disconnect();

        const std::string& Name() const noexcept { return name; }
        bool Connected() const noexcept { return isConnected; }

        uint32_t DeviceCount() const noexcept { return devices.size(); }

        void HandleServerMessage(const Buttplug::ButtplugFFIServerMessage& msg);
        void HandleMsgResponse(const Buttplug::ButtplugFFIServerMessage& resp);

        std::optional<DeviceAddedCallback> DeviceAddedCb;
        std::optional<DeviceRemovedCallback> DeviceRemovedCb;
        std::optional<ErrorReceivedCallback> ErrorReceivedCb;
        std::optional<ScanningFinishedCallback> ScanningFinishedCb;
        std::optional<PingTimeoutCallback> PingTimeoutCb;
        std::optional<ServerDisconnectCallback> ServerDisconnectCb;
    };
} // namespace Buttplug
