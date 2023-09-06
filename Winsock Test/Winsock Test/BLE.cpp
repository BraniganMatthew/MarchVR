#include <iostream>
#include <queue>
#include <future>
#include <experimental/coroutine>
//#include <unknwn.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Devices.Bluetooth.h>
#include <winrt/Windows.Devices.Bluetooth.GenericAttributeProfile.h>
#include <winrt/windows.foundation.collections.h>
#include <winrt/windows.storage.streams.h>
//#include "winrt/Windows.Devices.Bluetooth.GenericAttributeProfile.GattDeviceService.h"
#include <winrt/Windows.Devices.Enumeration.h>
#include "winrt/Windows.Devices.Bluetooth.Advertisement.h"
#include <rpc.h>


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
    // create serviceUUID 4fafc201-1fb5-459e-8fcc-c5c9c331914b
    std::array<uint8_t, 8> byteArrayServUUID = { 0x8f, 0xcc, 0xc5, 0xc9, 0xc3, 0x31, 0x91, 0x4b };
    winrt::guid serviceUUID((uint32_t)0x4fafc201, (uint16_t)0x1fb5, (uint16_t)0x459e, byteArrayServUUID);

    // create characteristicUUID beb5483e-36e1-4688-b7f5-ea07361b26a8
    std::array<uint8_t, 8> byteArrayCharUUID = { 0xb7, 0xf5, 0xea, 0x07, 0x36, 0x1b, 0x26, 0xa8 };
    winrt::guid characteristicUUID((uint32_t)0xbeb5483e, (uint16_t)0x36e1, (uint16_t)0x4688, byteArrayCharUUID);

	// initialize runtime environment
	init_apartment();

	//auto device = BluetoothLEDevice::FromBluetoothAddressAsync(esp32macaddress2_uint64).get();
    auto devices = DeviceInformation::FindAllAsync(BluetoothLEDevice::GetDeviceSelector()).get();

	for (auto deviceInfo : devices) {
		auto device = BluetoothLEDevice::FromIdAsync(deviceInfo.Id()).get();

		if (device != nullptr) {
			auto servicesResult = device.GetGattServicesForUuidAsync(serviceUUID, BluetoothCacheMode::Cached).get();

			if (servicesResult.Status() == GattCommunicationStatus::Success) {
				auto services = servicesResult.Services();
				if (services.Size() > 0) {
					for (auto service : services) {
						winrt::guid tempServiceUuid = service.Uuid();
						GUID toprintServiceUuid = reinterpret_cast<GUID&>(tempServiceUuid);
						OLECHAR* guidString;
						StringFromCLSID(toprintServiceUuid, &guidString);
						std::cout << "Service UUID: " << guidString << std::endl;
						::CoTaskMemFree(guidString);

						auto characteristicsResult = service.GetCharacteristicsForUuidAsync(characteristicUUID, BluetoothCacheMode::Cached).get();

						if (characteristicsResult.Status() == GattCommunicationStatus::Success) {
							auto characteristics = characteristicsResult.Characteristics();

							if (characteristics.Size() > 0) {
								//auto characteristic = characteristics.GetAt(0);

								for (auto characterisitic : characteristics) {
									GattReadResult readResult = characterisitic.ReadValueAsync().get();
									if (readResult.Status() == GattCommunicationStatus::Success) {
										auto data = readResult.Value();
										auto reader = Windows::Storage::Streams::DataReader::FromBuffer(data);
										hstring receivedString = reader.ReadString(data.Length());
										std::wcout << L"Data received: " << receivedString.c_str() << std::endl;
									}
									else {
										std::cout << "Failed to read characteristic value." << std::endl;
									}
								}
								//GattReadResult readResult = characteristic.ReadValueAsync().get();
								//if (readResult.Status() == GattCommunicationStatus::Success) {
								//	// Access the data from the read result
								//	//auto reader = Windows::Storage::Streams::DataReader::FromBuffer(readResult.Value());

								//	// Depending on the data format, you can read data here
								//	// Example:
								//	auto data = readResult.Value();
								//	auto reader = Windows::Storage::Streams::DataReader::FromBuffer(data);
								//	hstring receivedString = reader.ReadString(data.Length());
								//	std::wcout << L"Data received: " << receivedString.c_str() << std::endl;
								//}
								//else {
								//	std::cout << "Failed to read characteristic value." << std::endl;
								//}
								//std::cout << "Here" << std::endl; 
							}
							else {
								std::cout << "No characteristics found." << std::endl;
							}
						}
						else {
							std::cout << "Failed to discover characteristics." << std::endl;
						}
					}
				}
				else {
					std::cout << "No GATT services found." << std::endl;
				}
				
			}
			else {
				std::cout << "Failed to establish a GATT connection." << std::endl;
			}
		}
	}
	//if (device == nullptr) {
	//	std::cout << "Device cannot be found or accessed" << std::endl;
	//	return 1;
	//}

	//GattDeviceServicesResult result = device.GetGattServicesAsync().get();


 //   if (result.Status() == GattCommunicationStatus::Success) {
 //       auto services = result.Services();

 //       if (services.Size() > 0) {
 //           for (auto service : services) {
 //               winrt::guid tempServiceUuid = service.Uuid();
 //               GUID toprintServiceUuid = reinterpret_cast<GUID&>(tempServiceUuid);
 //               OLECHAR* guidString;
 //               StringFromCLSID(toprintServiceUuid, &guidString);

 //               std::cout << "Service UUID: " << guidString << std::endl;

 //               ::CoTaskMemFree(guidString);

 //               //auto service = services.GetAt(0);
 //               auto characteristicsResult = service.GetCharacteristicsAsync().get();

 //               if (characteristicsResult.Status() == GattCommunicationStatus::Success) {
 //                   auto characteristics = characteristicsResult.Characteristics();
 //                   

 //                   if (characteristics.Size() > 0) {
 //                       //auto characteristic = characteristics.GetAt(0);
 //                       for (auto characteristic : characteristics) {
 //                           winrt::guid tempCharUuid = characteristic.Uuid();
 //                           GUID toprintCharUuid = reinterpret_cast<GUID&>(tempCharUuid);
 //                           OLECHAR* guidStringChar;
 //                           StringFromCLSID(toprintCharUuid, &guidStringChar);

 //                           std::cout << "Characteristic UUID: " << guidStringChar << std::endl;

 //                           ::CoTaskMemFree(guidStringChar);
 //                           //GattReadResult readResult = characteristic.ReadValueAsync().get();
 //                           //if (readResult.Status() == GattCommunicationStatus::Success) {
 //                           //    // Access the data from the read result
 //                           //    auto reader = Windows::Storage::Streams::DataReader::FromBuffer(readResult.Value());
 //                           //    // Depending on the data format, you can read data here
 //                           //    // Example: 
 //                           //    uint32_t data;
 //                           //    data = reader.ReadUInt32();
 //                           //    std::cout << "Data received: " << data << std::endl;
 //                           //}
 //                           //else {
 //                           //    std::cout << "Failed to read characteristic value." << std::endl;
 //                           //}
 //                       }
 //                   }
 //                   else {
 //                       std::cout << "No characteristics found." << std::endl;
 //                   }
 //               }
 //               else {
 //                   std::cout << "Failed to discover characteristics." << std::endl;
 //               }
 //           }
 //       }
 //       else {
 //           std::cout << "No GATT services found." << std::endl;
 //       }
 //   }
 //   else {
 //       std::cout << "Failed to establish a GATT connection." << std::endl;
 //   }

	return 0;
}
