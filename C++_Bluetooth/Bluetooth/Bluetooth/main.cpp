#include "main.h"

using namespace std;

int main(void)
{
	int timeElapsed = 0;
	int lastQuantity = 0;
	HRESULT result;

	crcInit();

	/* Connect to the BLE device. */
	BLE::PSOC()->getGuid(BLE::PSOC()->DeviceUUID, &BLE::PSOC()->Guid);
	cout << "Guid = " << BLE::PSOC()->Guid.Data1 << "-" << BLE::PSOC()->Guid.Data2 << "-" << BLE::PSOC()->Guid.Data3 << "-" << BLE::PSOC()->Guid.Data4 << endl;

	BLE::PSOC()->getDeviceHandle(&BLE::PSOC()->Guid, &BLE::PSOC()->DeviceHandle);
	cout << "Device handle = " << BLE::PSOC()->DeviceHandle << endl << endl;

	/* Retrieve the list of services. */
	result = BLE::PSOC()->getListServices();
	if (SUCCEEDED(result))
		cout << "Successfully retrieved the list of services." << endl;

	/* Retrieve the list of characteristics. */
	result = BLE::PSOC()->getListCharacteristics();
	if (SUCCEEDED(result))
		cout << "Successfully retrieved the list of characteristics." << endl;

	/* Retrieve the list of descriptors. */
	result = BLE::PSOC()->getListDescriptors();
	if (SUCCEEDED(result))
		cout << "Successfully retrieved the list of descriptors." << endl << endl;

	/* Display the list of services. */
	PBTH_LE_GATT_SERVICE service;
	for (int iServices = 0; iServices < BLE::PSOC()->numServices; iServices++)
	{
		service = &BLE::PSOC()->servicesBuffer[iServices];
		cout << "Service " << iServices << ":" << endl;
		cout << "\tAttribute Handle = " << service->AttributeHandle << endl;
		cout << "\tService UUID (short) = " << service->ServiceUuid.Value.ShortUuid << endl << endl;
	}

	/* Display the list of characteristics. */
	PBTH_LE_GATT_CHARACTERISTIC characteristic;
	for (int iCharacteristics = 0; iCharacteristics < BLE::PSOC()->numCharacteristics; iCharacteristics++)
	{
		characteristic = &BLE::PSOC()->characteristicsBuffer[iCharacteristics];
		cout << "Characteristic " << iCharacteristics << ":" << endl;
		cout << "\tAttribute Handle = " << characteristic->AttributeHandle << endl;
		cout << "\tCharacteristic UUID (short) = " << characteristic->CharacteristicUuid.Value.ShortUuid << endl;
		cout << "\tCharacteristic Value Handle = " << characteristic->CharacteristicValueHandle << endl;
		cout << "\tHas Extended Properties = " << characteristic->HasExtendedProperties << endl;
		cout << "\tIs Broadcastable = " << characteristic->IsBroadcastable << endl;
		cout << "\tIs Indicatable = " << characteristic->IsIndicatable << endl;
		cout << "\tIs Notifiable = " << characteristic->IsNotifiable << endl;
		cout << "\tIs Readable = " << characteristic->IsReadable << endl;
		cout << "\tIs Signed Writable = " << characteristic->IsSignedWritable << endl;
		cout << "\tIs Writable = " << characteristic->IsWritable << endl;
		cout << "\tIs Writable Without Response = " << characteristic->IsWritableWithoutResponse << endl;
		cout << "\tService Handle = " << characteristic->ServiceHandle << endl << endl;
	}

	/* Display the list of descriptors. */
	PBTH_LE_GATT_DESCRIPTOR descriptor;
	for (int iDescriptors = 0; iDescriptors < BLE::PSOC()->numDescriptors; iDescriptors++)
	{
		descriptor = &BLE::PSOC()->descriptorsBuffer[iDescriptors];
		cout << "Descriptor " << iDescriptors << ":" << endl;
		cout << "\tAttribute Handle = " << descriptor->AttributeHandle << endl;
		cout << "\tCharacteristic Handle = " << descriptor->CharacteristicHandle << endl;
		cout << "\tDescriptor Type = " << descriptor->DescriptorType << endl;
		cout << "\tDescriptor UUID (short) = " << descriptor->DescriptorUuid.Value.ShortUuid << endl;
		cout << "\tService Handle = " << descriptor->ServiceHandle << endl << endl;
	}

	/* Enable notifications on all characteristics. */
	BOOL notificationEnabled = FALSE;
	USHORT numNotifiedCharacteristics = 0;
	characteristic = nullptr;
	descriptor = nullptr;

	for (int i = 0; i < BLE::PSOC()->numDescriptors; i++)
	{
		descriptor = &BLE::PSOC()->descriptorsBuffer[i];

		if (NULL != BLE::PSOC()->getCharacteristicWithHandle(descriptor->CharacteristicHandle)) {
			characteristic = BLE::PSOC()->getCharacteristicWithHandle(descriptor->CharacteristicHandle);

			if (characteristic->IsNotifiable)
			{
				if (descriptor->DescriptorType == ClientCharacteristicConfiguration)
				{
					result = BLE::PSOC()->enableNotifications(descriptor);
					if (SUCCEEDED(result)) {
						cout << "Successfully enabled the notifications for characteristic " << characteristic->AttributeHandle << "." << endl;
						notificationEnabled = TRUE;
						BLE::PSOC()->notifiedCharacteristicsBuffer[numNotifiedCharacteristics] = characteristic;
						numNotifiedCharacteristics++;
					}
				}
			}
		}
	}

	/* Register the callback event. */
	if (notificationEnabled) {
		result = BLE::PSOC()->registerNotificationsEvent(BLE::PSOC()->notifiedCharacteristicsBuffer, numNotifiedCharacteristics);

		if (SUCCEEDED(result)) {
			cout << "Successfully registered the callback event for the characteristics";
			for (int i = 0; i < numNotifiedCharacteristics; i++)
				cout << " " << BLE::PSOC()->notifiedCharacteristicsBuffer[i]->AttributeHandle;
			cout << "." << endl << endl;
		}
	}


	while (1)
		Sleep(1000);


	/*
	cout << "Acquiring data from the PSOC";
	for (int i = 0; i < 10; i++) {
		Sleep(500);
		cout << ".";
	}
	cout << endl;
	
	cout << "Finished acquiring data." << endl << "Displaying the data received:" << endl;
	
	for (int i = 0; i < BLE::PSOC()->CapSense.size(); i++) {
		cout << endl << "Data " << i << ": 0x" << hex << uppercase << setfill('0') << setw(4) << BLE::PSOC()->CapSense[i].sensor1;
	}
	*/

	cout << endl;
	system("pause");
	
}