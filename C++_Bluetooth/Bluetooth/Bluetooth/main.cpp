#include "main.h"

#define NUM_SENSORS 5

using namespace std;

Device *PSOC = 0;

struct BLEsensor {
	uint32_t Data;
	uint16_t Value;
	uint16_t CRC;
};

struct sensors {
	uint16_t sensor[NUM_SENSORS];
};
std::vector<sensors> CapSense;

int main(void)
{	
	GUID PSOCguid;
	int i, j, k;
	char resp;
	bool quit = FALSE;

	/* INIT */
	crcInit();

	/* Create the BLE device. */
	CLSIDFromString(TEXT("{0003CAB5-0000-1000-8000-00805F9B0131}"), &PSOCguid);
	PSOC = new Device(PSOCguid);

	/* Retrieve the whole data structure from the BLE device and display it. */
	PSOC->retrieveListServices();
	for (i = 0; i < PSOC->numServices; i++) {
		cout << "Service " << i << ":" << endl;
		cout << "   Attribute Handle = " << PSOC->services[i]->AttributeHandle << endl;
		cout << "   Service UUID (short) = " << PSOC->services[i]->ServiceUuid.Value.ShortUuid << endl << endl;

		PSOC->services[i]->retrieveListCharacteristics();
		for (j = 0; j < PSOC->services[i]->numCharacteristics; j++) {
			cout << "\tCharacteristic " << j << ":" << endl;
			cout << "\t   Attribute Handle = " << PSOC->services[i]->characteristics[j]->AttributeHandle << endl;
			cout << "\t   Characteristic UUID (short) = " << PSOC->services[i]->characteristics[j]->CharacteristicUuid.Value.ShortUuid << endl;
			cout << "\t   Characteristic Value Handle = " << PSOC->services[i]->characteristics[j]->CharacteristicValueHandle << endl;
			cout << "\t   Has Extended Properties = " << PSOC->services[i]->characteristics[j]->HasExtendedProperties << endl;
			cout << "\t   Is Broadcastable = " << PSOC->services[i]->characteristics[j]->IsBroadcastable << endl;
			cout << "\t   Is Indicatable = " << PSOC->services[i]->characteristics[j]->IsIndicatable << endl;
			cout << "\t   Is Notifiable = " << PSOC->services[i]->characteristics[j]->IsNotifiable << endl;
			cout << "\t   Is Readable = " << PSOC->services[i]->characteristics[j]->IsReadable << endl;
			cout << "\t   Is Signed Writable = " << PSOC->services[i]->characteristics[j]->IsSignedWritable << endl;
			cout << "\t   Is Writable = " << PSOC->services[i]->characteristics[j]->IsWritable << endl;
			cout << "\t   Is Writable Without Response = " << PSOC->services[i]->characteristics[j]->IsWritableWithoutResponse << endl;
			cout << "\t   Service Handle = " << PSOC->services[i]->characteristics[j]->ServiceHandle << endl << endl;

			PSOC->services[i]->characteristics[j]->retrieveListDescriptors();
			for (k = 0; k < PSOC->services[i]->characteristics[j]->numDescriptors; k++)	{
				cout << "\t\tDescriptor " << k << ":" << endl;
				cout << "\t\t   Attribute Handle = " << PSOC->services[i]->characteristics[j]->descriptors[k]->AttributeHandle << endl;
				cout << "\t\t   Characteristic Handle = " << PSOC->services[i]->characteristics[j]->descriptors[k]->CharacteristicHandle << endl;
				cout << "\t\t   Descriptor Type = " << PSOC->services[i]->characteristics[j]->descriptors[k]->DescriptorType << endl;
				cout << "\t\t   Descriptor UUID (short) = " << PSOC->services[i]->characteristics[j]->descriptors[k]->DescriptorUuid.Value.ShortUuid << endl;
				cout << "\t\t   Service Handle = " << PSOC->services[i]->characteristics[j]->descriptors[k]->ServiceHandle << endl << endl;
			}
		}
	}

	cout << "Ready to start: ";
	system("pause");
	cout << endl;

	/* Subscribe to the data acquisition. */
	for (i = 0; i < PSOC->numServices; i++) {
		for (j = 0; j < PSOC->services[i]->numCharacteristics; j++) {
			if (PSOC->services[i]->characteristics[j]->IsNotifiable)
				PSOC->services[i]->characteristics[j]->subscribeToNotification(FALSE, TRUE, newSensorDataReceived);
		}
	}

	cout << "Acquiring data..........." << endl << endl;

	Sleep(1000);

	//cout << "You can stop the data acquisition at any time: ";
	//system("pause");
	//cout << endl;

	/* Unsubscribe to the data acquisition. */
	for (i = 0; i < PSOC->numServices; i++) {
		for (j = 0; j < PSOC->services[i]->numCharacteristics; j++) {
			if (PSOC->services[i]->characteristics[j]->notificationEnabled)
				PSOC->services[i]->characteristics[j]->unsubscribeToNotification();
		}
	}

	for (int i = 0; i < CapSense.size(); i++) {
		for (int j = 0; j < NUM_SENSORS; j++)
			cout << "Sensor " << j << " = " << hex << uppercase << setfill('0') << setw(4) << CapSense[i].sensor[j] << endl;
		cout << endl;
	}

	cout << "Quantity of notification received = " << CapSense.size() << endl << endl;

	system("pause");
	cout << endl;

	delete PSOC;
	CapSense.clear();
	CapSense.shrink_to_fit();
	cout << "CapSense vector deleted." << endl << endl;

	system("pause");
	return 0;
}


/*************************************************************************
*
* Function:		newSensorDataReceived()
*
* Description:	Signal sent from the BLE device when new sensor data
*				is available.
*
* Notes:
*
* Parameters:	Type of event, Data received and Additionnal information
*				you can send through the BluetoothGATTRegisterEvent
*				function.
*
* Returns:		None defined.
*
**************************************************************************/
void newSensorDataReceived(__in BTH_LE_GATT_EVENT_TYPE EventType, __in PVOID EventOutParameter, __in PVOID Context)
{
	PBLUETOOTH_GATT_VALUE_CHANGED_EVENT ValueChangedEventParameters = (PBLUETOOTH_GATT_VALUE_CHANGED_EVENT)EventOutParameter;
	Characteristic *_characteristic = (Characteristic*)Context;
	BLEsensor sensor[NUM_SENSORS];
	sensors valuesToPush;
	const uint8_t sensorDataSize = sizeof(uint32_t);

	uint16_t totalDataSize = unsigned(ValueChangedEventParameters->CharacteristicValue->DataSize);

	//cout << endl;
	
	for (int i = 0; i < NUM_SENSORS; i++) {
		for (int j = 0; j < sensorDataSize; j++)
			sensor[i].Data = sensor[i].Data << 8 | unsigned(ValueChangedEventParameters->CharacteristicValue->Data[ (sensorDataSize*i) + j ]);

		sensor[i].Value = sensor[i].Data >> 16 & 0xFFFF;
		sensor[i].CRC = sensor[i].Data & 0xFFFF;

		if (verifyCRC(sensor[i].Value, sensor[i].CRC)) {
			valuesToPush.sensor[i] = sensor[i].Value;
			//cout << "Sensor " << i << ":" << endl;
			//cout << "\t Sensor value = 0x" << hex << uppercase << setfill('0') << setw(4) << sensor[i].Value << endl;
			//cout << "\t Sensor crc = 0x" << hex << uppercase << setfill('0') << setw(4) << sensor[i].CRC << endl << endl;
		}
		else
			valuesToPush.sensor[i] = NULL;
			//cout << "Sensor " << i << ": ERROR" << endl << endl << endl << endl;
	}

	CapSense.push_back(valuesToPush);

}

//int main(void)
//{
//	int timeElapsed = 0;
//	int lastQuantity = 0;
//	HRESULT result;
//
//	crcInit();
//
//	/* Connect to the BLE device. */
//	BLE::PSOC()->getGuid(BLE::PSOC()->DeviceUUID, &BLE::PSOC()->Guid);
//	cout << "Guid = " << BLE::PSOC()->Guid.Data1 << "-" << BLE::PSOC()->Guid.Data2 << "-" << BLE::PSOC()->Guid.Data3 << "-" << BLE::PSOC()->Guid.Data4 << endl;
//
//	BLE::PSOC()->getDeviceHandle(&BLE::PSOC()->Guid, &BLE::PSOC()->DeviceHandle);
//	cout << "Device handle = " << BLE::PSOC()->DeviceHandle << endl << endl;
//
//	/* Retrieve the list of services. */
//	result = BLE::PSOC()->getListServices();
//	if (SUCCEEDED(result))
//		cout << "Successfully retrieved the list of services." << endl;
//
//	/* Retrieve the list of characteristics. */
//	result = BLE::PSOC()->getListCharacteristics();
//	if (SUCCEEDED(result))
//		cout << "Successfully retrieved the list of characteristics." << endl;
//
//	/* Retrieve the list of descriptors. */
//	result = BLE::PSOC()->getListDescriptors();
//	if (SUCCEEDED(result))
//		cout << "Successfully retrieved the list of descriptors." << endl << endl;
//
//	/* Display the list of services. */
//	PBTH_LE_GATT_SERVICE service;
//	for (int iServices = 0; iServices < BLE::PSOC()->numServices; iServices++)
//	{
//		service = &BLE::PSOC()->servicesBuffer[iServices];
//		cout << "Service " << iServices << ":" << endl;
//		cout << "\tAttribute Handle = " << service->AttributeHandle << endl;
//		cout << "\tService UUID (short) = " << service->ServiceUuid.Value.ShortUuid << endl << endl;
//	}
//
//	/* Display the list of characteristics. */
//	PBTH_LE_GATT_CHARACTERISTIC characteristic;
//	PBTH_LE_GATT_DESCRIPTOR descriptor;
//	for (int iCharacteristics = 0; iCharacteristics < BLE::PSOC()->numCharacteristics; iCharacteristics++)
//	{
//		characteristic = &BLE::PSOC()->characteristicsBuffer[iCharacteristics];
//		cout << "Characteristic " << iCharacteristics << ":" << endl;
//		cout << "\tAttribute Handle = " << characteristic->AttributeHandle << endl;
//		cout << "\tCharacteristic UUID (short) = " << characteristic->CharacteristicUuid.Value.ShortUuid << endl;
//		cout << "\tCharacteristic Value Handle = " << characteristic->CharacteristicValueHandle << endl;
//		cout << "\tHas Extended Properties = " << characteristic->HasExtendedProperties << endl;
//		cout << "\tIs Broadcastable = " << characteristic->IsBroadcastable << endl;
//		cout << "\tIs Indicatable = " << characteristic->IsIndicatable << endl;
//		cout << "\tIs Notifiable = " << characteristic->IsNotifiable << endl;
//		cout << "\tIs Readable = " << characteristic->IsReadable << endl;
//		cout << "\tIs Signed Writable = " << characteristic->IsSignedWritable << endl;
//		cout << "\tIs Writable = " << characteristic->IsWritable << endl;
//		cout << "\tIs Writable Without Response = " << characteristic->IsWritableWithoutResponse << endl;
//		cout << "\tService Handle = " << characteristic->ServiceHandle << endl;
//	
//
//		/* Display the list of descriptors for each characteristic. */
//		for (int iDescriptors = 0; iDescriptors < BLE::PSOC()->numDescriptors; iDescriptors++)
//		{
//			descriptor = &BLE::PSOC()->descriptorsBuffer[iDescriptors];
//			cout << "Descriptor " << iDescriptors << ":" << endl;
//			cout << "\tAttribute Handle = " << descriptor->AttributeHandle << endl;
//			cout << "\tCharacteristic Handle = " << descriptor->CharacteristicHandle << endl;
//			cout << "\tDescriptor Type = " << descriptor->DescriptorType << endl;
//			cout << "\tDescriptor UUID (short) = " << descriptor->DescriptorUuid.Value.ShortUuid << endl;
//			cout << "\tService Handle = " << descriptor->ServiceHandle << endl << endl;
//		}
//	}
//	/* Enable notifications on all characteristics. */
//	BOOL notificationEnabled = FALSE;
//	USHORT numNotifiedCharacteristics = 0;
//	characteristic = nullptr;
//	descriptor = nullptr;
//
//	for (int i = 0; i < BLE::PSOC()->numDescriptors; i++)
//	{
//		descriptor = &BLE::PSOC()->descriptorsBuffer[i];
//
//		if (NULL != BLE::PSOC()->getCharacteristicWithHandle(descriptor->CharacteristicHandle)) {
//			characteristic = BLE::PSOC()->getCharacteristicWithHandle(descriptor->CharacteristicHandle);
//
//			if (characteristic->IsNotifiable)
//			{
//				if (descriptor->DescriptorType == ClientCharacteristicConfiguration)
//				{
//					result = BLE::PSOC()->enableNotifications(descriptor);
//					if (SUCCEEDED(result)) {
//						cout << "Successfully enabled the notifications for characteristic " << characteristic->AttributeHandle << "." << endl;
//						notificationEnabled = TRUE;
//						BLE::PSOC()->notifiedCharacteristicsBuffer[numNotifiedCharacteristics] = characteristic;
//						numNotifiedCharacteristics++;
//					}
//				}
//			}
//		}
//	}
//
//	/* Register the callback event. */
//	if (notificationEnabled) {
//		result = BLE::PSOC()->registerNotificationsEvent(BLE::PSOC()->notifiedCharacteristicsBuffer, numNotifiedCharacteristics);
//
//		if (SUCCEEDED(result)) {
//			cout << "Successfully registered the callback event for the characteristics";
//			for (int i = 0; i < numNotifiedCharacteristics; i++)
//				cout << " " << BLE::PSOC()->notifiedCharacteristicsBuffer[i]->AttributeHandle;
//			cout << "." << endl << endl;
//		}
//	}
//
//	/*
//	while (1)
//		Sleep(1000);
//	*/
//
//	/*
//	cout << "Acquiring data from the PSOC";
//	for (int i = 0; i < 10; i++) {
//		Sleep(500);
//		cout << ".";
//	}
//	cout << endl;
//	
//	cout << "Finished acquiring data." << endl << "Displaying the data received:" << endl;
//	
//	for (int i = 0; i < BLE::PSOC()->CapSense.size(); i++) {
//		cout << endl << "Data " << i << ": 0x" << hex << uppercase << setfill('0') << setw(4) << BLE::PSOC()->CapSense[i].sensor1;
//	}
//	*/
//
//	cout << endl;
//	system("pause");
//	
//	BLE::PSOC()->freeMemory();
//
//	return 0;
//}


/*************************************************************************
*
* Function:		ErrorDescription()
*
* Description:	Print a message explaining the HRESULT error code.
*
* Notes:		https://msdn.microsoft.com/en-us/library/windows/desktop/ms687061(v=vs.85).aspx
*
* Parameters:	HRESULT error code.
*
* Returns:		None defined.
*
**************************************************************************/
void ErrorDescription(HRESULT hr)
{
	if (FACILITY_WINDOWS == HRESULT_FACILITY(hr))
		hr = HRESULT_CODE(hr);
	TCHAR* szErrMsg;

	if (FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, hr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&szErrMsg, 0, NULL) != 0)
	{
		_tprintf(TEXT("%s"), szErrMsg);
		LocalFree(szErrMsg);
	}
	else
		_tprintf(TEXT("[Could not find a description for error # %#x.]\n"), hr);
}