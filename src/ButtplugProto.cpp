#include "generated/buttplug_rs_ffi.pb.h"
#include "ButtplugProto.h"

using namespace Buttplug;
using namespace google;

inline static Buttplug::ClientMessage::FFIMessage* ffiMessage(protobuf::Arena* arena) noexcept
{
    return protobuf::Arena::CreateMessage<Buttplug::ClientMessage::FFIMessage>(arena);
}

inline static Buttplug::ClientMessage* clientMessage(protobuf::Arena* arena) noexcept
{
    return protobuf::Arena::CreateMessage<Buttplug::ClientMessage>(arena);
}

Protocol::Protocol() 
{
    GOOGLE_PROTOBUF_VERIFY_VERSION;
    arena = std::make_unique<protobuf::Arena>();
}

Protocol::~Protocol() 
{

}

DeferredSender Protocol::SendMessage(Buttplug::ClientHandle clientHandle, Buttplug::ClientMessage* msg, Buttplug::Callback cb, void* ctx)
{
    std::string sendBuffer;
    uint32_t id = msgTag++;
    msg->set_id(id);
    if(msg->SerializeToString(&sendBuffer)) {
        auto send = [=]() {
            Buttplug::FFI::ClientProtobufMessage(clientHandle, (uint8_t*)sendBuffer.data(), sendBuffer.size(), cb, ctx);
        };
        arena->Reset(); msg = nullptr;
        return Buttplug::DeferredSender{id, std::move(send)};
    }
    throw std::exception("Failed to serialize message.");
}

DeferredSender Protocol::SendConnectWebsocket(Buttplug::ClientHandle clientHandle, const std::string& address, bool bypass_ssl, Buttplug::Callback cb, void* ctx) 
{
    auto wsConnect = protobuf::Arena::CreateMessage<Buttplug::ClientMessage::ConnectWebsocket>(arena.get());
    wsConnect->set_address(address);
    wsConnect->set_bypass_cert_verification(bypass_ssl);

    auto ffi = ffiMessage(arena.get());
    ffi->set_allocated_connect_websocket(wsConnect);

    auto msg = clientMessage(arena.get());
    msg->set_allocated_message(ffi);
    return SendMessage(clientHandle, msg, cb, ctx);
}

DeferredSender Protocol::SendDisconnect(Buttplug::ClientHandle clientHandle, Buttplug::Callback cb, void* ctx) 
{
    auto disconnect = protobuf::Arena::CreateMessage<Buttplug::ClientMessage::Disconnect>(arena.get());
    auto ffi = ffiMessage(arena.get());
    ffi->set_allocated_disconnect(disconnect);
    auto msg = clientMessage(arena.get());
    msg->set_allocated_message(ffi);
    return SendMessage(clientHandle, msg, cb, ctx);
}

DeferredSender Protocol::SendStartScanning(Buttplug::ClientHandle clientHandle, Buttplug::Callback cb, void* ctx) 
{
    auto startScan = protobuf::Arena::CreateMessage<Buttplug::ClientMessage::StartScanning>(arena.get());
    auto ffi = ffiMessage(arena.get());
    ffi->set_allocated_start_scanning(startScan);
    auto msg = clientMessage(arena.get());
    msg->set_allocated_message(ffi);
    return SendMessage(clientHandle, msg, cb, ctx);
}