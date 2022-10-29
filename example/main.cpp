#include <thread>
#include <chrono>
#include <iostream>

#include "ButtplugClient.h"

int main(int argc, char* argv[]) 
{
	using namespace std::chrono_literals;

	// Before the client can be used the FFI functions must be loaded.
	// If this fails the client must not be used.
	if(!Buttplug::FFI::Init()) {
		std::cout << "Failed to load buttplug_rs_ffi functions.\n";
		abort();
	}
	Buttplug::FFI::ActivateEnvLogger();

	auto client = Buttplug::Client("TestClient");
	std::weak_ptr<Buttplug::Device> Device;

	client.DeviceAddedCb = [&](std::weak_ptr<Buttplug::Device> device) 
	{
		std::cout << "Device added\n";
		Device = device;
	};

	client.DeviceRemovedCb = [](std::weak_ptr<Buttplug::Device> device)
	{
		std::cout << "Device removed\n";
	};

	client.ErrorReceivedCb = [](const std::string& error) 
	{
		std::cerr << error << '\n';
	};

	client.ScanningFinishedCb = []()
	{
		std::cout << "Scanning finished\n";
	};

	client.PingTimeoutCb = []()
	{
		std::cout << "Ping timeout\n";
	};

	client.ServerDisconnectCb = []()
	{
		std::cout << "Server disconnect\n";
	};

	client.ConnectWebsocket("ws://127.0.0.1:12345/buttplug", true).wait();
	client.StartScanning().wait();


	bool waitingForDevice = true;
	while(waitingForDevice) {
		std::this_thread::sleep_for(10ms);

		if(!Device.expired()) {
			if(auto useDevice = Device.lock()) {
				#if 0
				std::cout << "Got device sending linear command\n";
				// go to max position in 1 second
				useDevice->SendLinearCmd(1000, 1.0);
				std::this_thread::sleep_for(1000ms);
				// go to min position in 1 second
				useDevice->SendLinearCmd(1000, 0.0);
				#else
				std::cout << "Got device sending vibrate command\n";
				// vibrate max speed
				useDevice->SendVibrateCmd(1.0);
				std::this_thread::sleep_for(1000ms);
				// stop vibrating
				useDevice->SendVibrateCmd(0.0);
				#endif
			}

			waitingForDevice = false;
		}
	}

	std::this_thread::sleep_for(3000ms);

	client.Disconnect().wait();

	return 0;
}
