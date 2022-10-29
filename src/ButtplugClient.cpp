#include "ButtplugClient.h"
#include "ButtplugProto.h"

#include <iostream>
#include "generated/buttplug_rs_ffi.pb.h"

inline static bool IsDeviceEvent(const Buttplug::ButtplugFFIServerMessage& msg) noexcept
{
    return msg.has_message() && msg.message().has_device_event();
}

inline static bool IsServerMsg(const Buttplug::ButtplugFFIServerMessage& msg) noexcept
{
    return msg.has_message() && msg.message().has_server_message();
}

inline static bool IsOk(const Buttplug::ButtplugFFIServerMessage& msg) noexcept
{
    return IsServerMsg(msg) && msg.message().server_message().has_ok();
}

inline static bool IsError(const Buttplug::ButtplugFFIServerMessage& msg) noexcept
{
    return IsServerMsg(msg) && msg.message().server_message().has_error();
}

inline static bool IsDisconnect(const Buttplug::ButtplugFFIServerMessage& msg) noexcept
{
    return IsServerMsg(msg) && msg.message().server_message().has_disconnect();
}

inline static bool IsScanningFinished(const Buttplug::ButtplugFFIServerMessage& msg) noexcept
{
    return IsServerMsg(msg) && msg.message().server_message().has_scanning_finished();
}

inline static bool IsDeviceAdded(const Buttplug::ButtplugFFIServerMessage& msg) noexcept
{
    return IsServerMsg(msg) && msg.message().server_message().has_device_added();
}

inline static bool IsDeviceRemoved(const Buttplug::ButtplugFFIServerMessage& msg) noexcept
{
    return IsServerMsg(msg) && msg.message().server_message().has_device_removed();
}

static void DefaultClientCallback(void* ctx, uint8_t* buf, int32_t bufLen) noexcept
{
    auto client = static_cast<Buttplug::Client*>(ctx);
    Buttplug::ButtplugFFIServerMessage response;
    if(response.ParseFromArray(buf, bufLen)) {
        client->HandleServerMessage(response);
    }
}

static void DefaultMsgCallback(void* ctx, uint8_t* buf, int32_t bufLen) noexcept
{
    auto client = static_cast<Buttplug::Client*>(ctx);
    Buttplug::ButtplugFFIServerMessage response;
    if(response.ParseFromArray(buf, bufLen)) {
        client->HandleMsgResponse(response);
    }   
}

inline static auto FalseFuture() noexcept
{
    // This seems stupid.
    std::promise<bool> promise;
    promise.set_value(false);
    return promise.get_future();
}

Buttplug::Client::Client(const std::string& clientName) : name(clientName)
{
    protocol = std::make_unique<Buttplug::Protocol>();
    clientHandle = Buttplug::FFI::CreateProtobufClient(name.c_str(), DefaultClientCallback, this);
}

Buttplug::Client::~Client()
{
    using namespace std::chrono_literals;
    Disconnect().wait();
    // HACK
    while(!devices.empty()) {
        std::this_thread::sleep_for(10ms);
    }
    Buttplug::FFI::FreeClient(clientHandle);
    clientHandle = nullptr;
}

std::promise<bool>& Buttplug::Client::addResponseHandler(uint32_t id, ServerResponseFunc&& handler)
{
    const std::lock_guard guard(responseMtx);
    auto wrapper = ServerResponseHandler{std::move(handler)};
    auto pair = std::make_pair(id, std::move(wrapper));
    auto it = responeHandlerMap.insert(std::move(pair));
    return it.first->second.Promise;
}

void Buttplug::Client::HandleMsgResponse(const Buttplug::ButtplugFFIServerMessage& resp) 
{
    if(IsError(resp)) {
        auto& error = resp.message().server_message().error();
        if(ErrorReceivedCb.has_value()) {
            ErrorReceivedCb.value()(error.message());
        }
        else {
            std:: cerr << error.message() << '\n' << error.backtrace();
        }
    }

    if(resp.id() != 0) {
        const std::lock_guard guard(responseMtx);
        auto it = responeHandlerMap.find(resp.id());
        if(it != responeHandlerMap.end()) {
            it->second.Handler(resp, it->second.Promise);
            responeHandlerMap.erase(it);
        }
    }
    else {
        std::cout << "resp: " << resp.DebugString() << '\n';
    }
}

void Buttplug::Client::HandleServerMessage(const Buttplug::ButtplugFFIServerMessage& msg)
{
    if(IsError(msg)) {
        auto& error = msg.message().server_message().error();
        if(ErrorReceivedCb.has_value()) {
            ErrorReceivedCb.value()(error.message());
        }
        else {
            std:: cerr << error.message() << '\n' << error.backtrace();
        }
    }
    else if(IsDisconnect(msg)) {
        isConnected = false;
        if(ServerDisconnectCb.has_value()) {
            ServerDisconnectCb.value()();
        }
    }
    else if(IsScanningFinished(msg)) {
        if(ScanningFinishedCb.has_value()) {
            ScanningFinishedCb.value()();
        }
    }
    else if(IsDeviceAdded(msg)) {
        auto& deviceAdded = msg.message().server_message().device_added();
        std::cout << "Device:" << deviceAdded.name() << '\n';
        std::cout << "Index: " << deviceAdded.index() << '\n';
        
        for(int i=0; i < deviceAdded.message_attributes_size(); i += 1) {
            auto& attr = deviceAdded.message_attributes(i);
            std::cout << "Attributes: " << attr.DebugString() << '\n';
        }

        auto deviceHandle = Buttplug::FFI::CreateDevice(clientHandle, deviceAdded.index());
        auto device = std::make_shared<Buttplug::Device>(deviceAdded.name(), this, deviceHandle);

        if(deviceHandle) {
            auto it = devices.insert(std::make_pair(deviceAdded.index(), std::move(device)));
            if(it.second) {
                if(DeviceAddedCb.has_value()) {
                    DeviceAddedCb.value()(it.first->second);
                }
            }
        }
    }
    else if(IsDeviceRemoved(msg)) {
        auto& deviceRemoved = msg.message().server_message().device_removed();
        auto deviceIndex = deviceRemoved.index();
        auto it = std::find_if(devices.begin(), devices.end(), 
            [&](auto& item) {
                return item.first == deviceIndex;
            });
        if(it != devices.end()) {
            if(DeviceRemovedCb.has_value()) {
                DeviceRemovedCb.value()(it->second);
            }
            devices.erase(it);
        }
    }
    else {
        std::cout << "msg: " << msg.DebugString() << '\n';
    }
}

std::future<bool> Buttplug::Client::ConnectWebsocket(const std::string& address, bool bypass_ssl) 
{
    if(isConnected) return FalseFuture();
    auto sender = protocol->SendConnectWebsocket(clientHandle, address, bypass_ssl, DefaultMsgCallback, this);
    auto& result = addResponseHandler(sender.Id, [this](const ButtplugFFIServerMessage& response, std::promise<bool>& promise) {
        isConnected = IsOk(response);
        promise.set_value(isConnected);
    });
    sender.Send();
    return result.get_future();
}

std::future<bool> Buttplug::Client::StartScanning()
{
    if(!isConnected) return FalseFuture();
    auto sender = protocol->SendStartScanning(clientHandle, DefaultMsgCallback, this);
    auto& result = addResponseHandler(sender.Id, [this](const ButtplugFFIServerMessage& response, std::promise<bool>& promise) {
        bool ok = IsOk(response);
        promise.set_value(ok);

        if(ScanningFinishedCb.has_value()) {
            ScanningFinishedCb.value()();
        }
    });
    sender.Send();
    return result.get_future();
}

std::future<bool> Buttplug::Client::Disconnect() 
{
    if(!isConnected) return FalseFuture();
    auto sender = protocol->SendDisconnect(clientHandle, DefaultMsgCallback, this);
    auto& result = addResponseHandler(sender.Id, [this](const ButtplugFFIServerMessage& response, std::promise<bool>& promise) {
        bool ok = IsOk(response);
        promise.set_value(ok);
    });
    sender.Send();
    return result.get_future();
}

