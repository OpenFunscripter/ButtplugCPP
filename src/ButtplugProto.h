#pragma once

#include "ButtplugFFI.h"
#include <string>
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
    class ClientMessage;

    struct DeferredSender
    {
        uint32_t Id = 0;
        std::function<void()> Send = [](){};
    };

    class Protocol
    {
        private:
        std::unique_ptr<google::protobuf::Arena> arena;
        uint32_t msgTag = 1;

        DeferredSender SendMessage(Buttplug::ClientHandle clientHandle, Buttplug::ClientMessage* msg, Buttplug::Callback cb, void* ctx);
        public:
        Protocol();
        Protocol(const Protocol&) = delete;
        Protocol(Protocol&&) = delete;
        ~Protocol();

        DeferredSender SendConnectWebsocket(Buttplug::ClientHandle clientHandle, const std::string& address, bool bypass_ssl, Buttplug::Callback cb, void* ctx);
        DeferredSender SendDisconnect(Buttplug::ClientHandle clienthandle, Buttplug::Callback cb, void* ctx);
        DeferredSender SendStartScanning(Buttplug::ClientHandle clientHandle, Buttplug::Callback cb, void* ctx);
    };

} // namespace Buttplug
