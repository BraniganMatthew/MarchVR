#include <iostream>
#include <queue>
#include <future>
#include <experimental/coroutine>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Devices.Bluetooth.h>
#include <winrt/Windows.Devices.Bluetooth.GenericAttributeProfile.h>
#include "winrt/Windows.Devices.Bluetooth.Advertisement.h"


using namespace winrt;
using namespace Windows::Devices::Bluetooth;
using namespace Windows::Devices::Enumeration;
using namespace Windows::Devices::Bluetooth::GenericAttributeProfile;
using namespace Windows::Devices::Bluetooth::Advertisement;

const char* esp32macaddress1 = "e8:9f:6d:26:9f:1a";
const char* esp32macaddress2 = "e8:9f:6d:2f:27:d6";
uint64_t esp32macaddress1_uint64 = 0xe89f6d269f1a;
uint64_t esp32macaddress2_uint64 = 0xe89f6d2f27d6;

int main() {

	// initialize runtime environment
	init_apartment();

	auto device = BluetoothLEDevice::FromBluetoothAddressAsync(esp32macaddress2_uint64).get();

	if (device == nullptr) {
		std::cout << "Device cannot be found or accessed" << std::endl;
		return 1;
	}

	GattDeviceServicesResult result = device.GetGattServicesAsync().get();

    if (result.Status() == GattCommunicationStatus::Success)
    {
        //auto services = result.Services();

        //if (services.Size() > 0)
        //{
        //    auto service = services.GetAt(0);
        //    auto characteristics = service.GetCharacteristicsAsync().get();

        //    //for (auto characteristic : characteristics.Characteristics())
        //    //{
        //        // Access and interact with characteristics here
        //    //}
        //}
        //else
        //{
        //    std::cout << "No GATT services found." << std::endl;
        //}
        std::cout << "Gatt communication success!" << std::endl;
    }
    else
    {
        std::cout << "Failed to establish a GATT connection." << std::endl;
    }

	return 0;
}
