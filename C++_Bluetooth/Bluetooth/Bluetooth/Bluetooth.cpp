/*************************************************************************
* Source: https://social.msdn.microsoft.com/Forums/lync/en-US/bad452cb-4fc2-4a86-9b60-070b43577cc9/is-there-a-simple-example-desktop-programming-c-for-bluetooth-low-energy-devices?forum=wdk
**************************************************************************/

#include "Bluetooth.h"

using namespace std;

BLE *BLE::instance = 0; //Instance of the BLE class.


/*************************************************************************
*
* Function:		PSOC()
*
* Description:	Create then return the single instance of the BLE class.
*
* Notes:
*
* Parameters:	None.
*
* Returns:		Address of the instance BLE::PSOC.
*
**************************************************************************/
BLE *BLE::PSOC()
{
	if (!instance)
		instance = new BLE;
	return instance;
}

/*************************************************************************
*
* Function:		NotificationReceived()
*
* Description:	Receives the new value sent from the BLE device.
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
void BLE::NotificationReceived(__in BTH_LE_GATT_EVENT_TYPE EventType, __in PVOID EventOutParameter, __in PVOID Context)
{
	uint16_t dataReceived_message = 0;
	uint16_t dataReceived_crc = 0;
	PBLUETOOTH_GATT_VALUE_CHANGED_EVENT ValueChangedEventParameters = (PBLUETOOTH_GATT_VALUE_CHANGED_EVENT)EventOutParameter;

	if (0 == ValueChangedEventParameters->CharacteristicValue->DataSize) {
		
	}

	else {
		uint32_t dataReceived = 0;
		ULONG dataSize = ValueChangedEventParameters->CharacteristicValue->DataSize;

		for (uint8_t i = 0; i < dataSize; i++) {
			dataReceived = dataReceived << 8 | unsigned(ValueChangedEventParameters->CharacteristicValue->Data[i]);
		}

		//cout << hex << uppercase << setfill('0') << setw(2 * dataSize) << dataReceived;

		dataReceived_message = dataReceived >> 16 & 0xFFFF;
		dataReceived_crc = dataReceived & 0xFFFF;

		//cout << "\t" << hex << uppercase << setfill('0') << setw(4) << dataReceived_message;
		//cout << "\t" << hex << uppercase << setfill('0') << setw(4) << dataReceived_crc;

		if (verifyCRC(dataReceived_message, dataReceived_crc)) {
			PSOC()->CapSense.push_back({ dataReceived_message });
			
			//cout << " \tCapSense Proximity=0x" << hex << uppercase << setfill('0') << setw(2 * sizeof(dataReceived_message)) << dataReceived_message;
			//myFile << "Notification " << (notificationsReceived) << " = ";
			//myFile << hex << uppercase << setfill('0') << setw(2 * sizeof(dataReceived_message)) << dataReceived_message << endl;
		}
		else {
			PSOC()->CapSense.push_back({ NULL });

			//cout << "\tERROR";
			//myFile << "ERROR" << endl;
		}

		//cout << "\tCRC=0x" << hex << uppercase << setfill('0') << setw(2 * sizeof(dataReceived_crc)) << dataReceived_crc;
	}
}


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
void BLE::ErrorDescription(HRESULT hr)
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


/*************************************************************************
*
* Function:		getGuid()
*
* Description:	Convert the string containing the Device UUID into
*				a UUID type of variable.
*
* Notes:		The variable containing the string has to be in UNICODE.
*
* Parameters:	None.
*
* Returns:		None defined.
*
**************************************************************************/
void BLE::getGuid(const wchar_t *_uuid, GUID *_guid)
{
	CLSIDFromString(_uuid, _guid);
}


/*************************************************************************
*
* Function:		getDeviceHandle()
*
* Description:	Get the device handle from its GUID.
*
* Notes:		Must have called getGuid before.
*				
* Parameters:	The GUID of the device and the variable that will hold
*				the divice handle.
*
* Returns:		None defined.
*
**************************************************************************/
void BLE::getDeviceHandle(GUID *_guid, HANDLE *_handle)
{
	*_handle = findDeviceHandle(*_guid);
}


/*************************************************************************
*
* Function:		findDeviceHandle()
*
* Description:	Get the device handle from its GUID.
*
* Notes:		Copied from:
*				http://social.msdn.microsoft.com/Forums/windowshardware/en-US/e5e1058d-5a64-4e60-b8e2-0ce327c13058/erroraccessdenied-error-when-trying-to-receive-data-from-bluetooth-low-energy-devices?forum=wdk
*
* Parameters:	The GUID of the device.
*
* Returns:		Handle of the device.
*
**************************************************************************/
HANDLE BLE::findDeviceHandle(GUID AGuid)
{
	HDEVINFO hDI;
	SP_DEVICE_INTERFACE_DATA did;
	SP_DEVINFO_DATA dd;
	GUID BluetoothInterfaceGUID = AGuid;
	HANDLE hComm = NULL;

	hDI = SetupDiGetClassDevs(&BluetoothInterfaceGUID, NULL, NULL, DIGCF_DEVICEINTERFACE | DIGCF_PRESENT);

	if (hDI == INVALID_HANDLE_VALUE) return NULL;

	did.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
	dd.cbSize = sizeof(SP_DEVINFO_DATA);

	for (DWORD i = 0; SetupDiEnumDeviceInterfaces(hDI, NULL, &BluetoothInterfaceGUID, i, &did); i++)
	{
		SP_DEVICE_INTERFACE_DETAIL_DATA DeviceInterfaceDetailData;

		DeviceInterfaceDetailData.cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

		DWORD size = 0;

		if (!SetupDiGetDeviceInterfaceDetail(hDI, &did, NULL, 0, &size, 0))
		{
			int err = GetLastError();

			if (err == ERROR_NO_MORE_ITEMS) break;

			PSP_DEVICE_INTERFACE_DETAIL_DATA pInterfaceDetailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA)GlobalAlloc(GPTR, size);

			pInterfaceDetailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

			if (!SetupDiGetDeviceInterfaceDetail(hDI, &did, pInterfaceDetailData, size, &size, &dd))
				break;

			hComm = CreateFile(
				pInterfaceDetailData->DevicePath,
				GENERIC_WRITE | GENERIC_READ,
				FILE_SHARE_READ | FILE_SHARE_WRITE,
				NULL,
				OPEN_EXISTING,
				0,
				NULL);

			GlobalFree(pInterfaceDetailData);
		}
	}

	SetupDiDestroyDeviceInfoList(hDI);
	return hComm;
}


/*************************************************************************
*
* Function:		getListServices()
*
* Description:	Retrieve the list of all the services available on the
*				BLE device.
*
* Notes:		Must have called getDeviceHandle before.
*				
*				First send 0,NULL as the parameters to
*				BluetoothGATTServices in order to get the number of
*				services in servicesBufferCount.
*				
*				BluetoothGATTGetServices:
*				https://msdn.microsoft.com/en-us/library/windows/hardware/hh450802(v=vs.85).aspx
*
* Parameters:	None.
*
* Returns:		Result of the second call of the BluetoothGATTGetServices.
*
**************************************************************************/
HRESULT BLE::getListServices(void)
{
	/* Determine the size of the buffer required. */
	USHORT serviceBufferSize;
	HRESULT hr = BluetoothGATTGetServices(
		DeviceHandle,
		0,
		NULL,
		&serviceBufferSize,
		BLUETOOTH_GATT_FLAG_NONE);

	if (HRESULT_FROM_WIN32(ERROR_MORE_DATA) != hr) {
		cout << "\tERROR while getting the size of the services buffer: ";
		ErrorDescription(hr);
		return hr;
	}

	/* Allocate space for the buffer. */
	servicesBuffer = (PBTH_LE_GATT_SERVICE)
		malloc(sizeof(BTH_LE_GATT_SERVICE) * serviceBufferSize);

	if (NULL == servicesBuffer) {
		cout << "\tERROR while allocating space for the services buffer." << endl;
	}
	else {
		RtlZeroMemory(servicesBuffer,
			sizeof(BTH_LE_GATT_SERVICE) * serviceBufferSize);
	}
	
	/* Retrieve the list of services. */
	hr = BluetoothGATTGetServices(
		DeviceHandle,
		serviceBufferSize,
		servicesBuffer,
		&numServices,
		BLUETOOTH_GATT_FLAG_NONE);

	if (FAILED(hr)) {
		cout << "\tERROR while getting the list of services: ";
		ErrorDescription(hr);
		return hr;
	}
	
	return hr;
}


/*************************************************************************
*
* Function:		getListCharacteristics()
*
* Description:	Retrieve the list of all the characteristis available on
*				the BLE device.
*
* Notes:		Must have called getListServices before because 
*				servicesBuffer is needed.
*
*				First send 0,NULL as the parameters to
*				BluetoothGATTGetCharacteristics in order to get the
*				number of characteristics in characteristicBufferSize.
*
*				BluetoothGATTGetCharacteristics:
*				https://msdn.microsoft.com/en-us/library/windows/hardware/hh450795(v=vs.85).aspx
*
* Parameters:	None.
*
* Returns:		Result of the second call of the
*				BluetoothGATTGetCharacteristics.
*
**************************************************************************/
HRESULT BLE::getListCharacteristics(void)
{
	/* Determine the size of the buffer required. */
	USHORT characteristicBufferSize;
	HRESULT hr = BluetoothGATTGetCharacteristics(
		DeviceHandle,
		servicesBuffer,
		0,
		NULL,
		&characteristicBufferSize,
		BLUETOOTH_GATT_FLAG_NONE);

	if (HRESULT_FROM_WIN32(ERROR_MORE_DATA) != hr) {
		cout << "\tERROR while getting the size of the characteristics buffer: ";
		ErrorDescription(hr);
		return hr;
	}

	/* Allocate space for the buffer. */
	if (characteristicBufferSize > 0)
	{
		characteristicsBuffer = (PBTH_LE_GATT_CHARACTERISTIC)
			malloc(characteristicBufferSize * sizeof(BTH_LE_GATT_CHARACTERISTIC));

		if (NULL == characteristicsBuffer) {
			cout << "\tERROR while allocating space for the characteristics buffer." << endl;
		}
		else {
			RtlZeroMemory(characteristicsBuffer,
				characteristicBufferSize * sizeof(BTH_LE_GATT_CHARACTERISTIC));
		}

		notifiedCharacteristicsBuffer = (PBTH_LE_GATT_CHARACTERISTIC*)
			malloc(characteristicBufferSize * sizeof(PBTH_LE_GATT_CHARACTERISTIC));

		if (NULL == notifiedCharacteristicsBuffer) {
			cout << "\tERROR while allocating space for the notified characteristics buffer." << endl;
		}
		else {
			RtlZeroMemory(notifiedCharacteristicsBuffer,
				characteristicBufferSize * sizeof(PBTH_LE_GATT_CHARACTERISTIC));
		}

		/* Retrieve the list of characteristics. */
		hr = BluetoothGATTGetCharacteristics(
			DeviceHandle,
			servicesBuffer,
			characteristicBufferSize,
			characteristicsBuffer,
			&numCharacteristics,
			BLUETOOTH_GATT_FLAG_NONE);

		if (FAILED(hr)) {
			cout << "\tERROR while getting the list of characteristics: ";
			ErrorDescription(hr);
			return hr;
		}

		if (numCharacteristics != characteristicBufferSize) {
			cout << "\tWARNING - Mismatch between the size of the buffer and the number of characteristics." << endl;
		}
	}

	return hr;
}


/*************************************************************************
*
* Function:		getListDescriptors()
*
* Description:	Retrieve the list of all the descriptors available on
*				the BLE device.
*
* Notes:		Must have called getListCharacteristics before because
*				characteristicsBuffer is needed.
*
*				First send 0,NULL as the parameters to
*				BluetoothGATTGetDescriptors in order to get the
*				number of descriptors in descriptorBufferSize.
*
*				BluetoothGATTGetDescriptors:
*				https://msdn.microsoft.com/en-us/library/windows/hardware/hh450797(v=vs.85).aspx
*
* Parameters:	None.
*
* Returns:		Result of the second call of the
*				BluetoothGATTGetDescriptors.
*
**************************************************************************/
HRESULT BLE::getListDescriptors(void)
{
	/* Determine the size of the buffer required. */
	USHORT descriptorBufferSize;
	HRESULT hr = BluetoothGATTGetDescriptors(
		DeviceHandle,
		characteristicsBuffer,
		0,
		NULL,
		&descriptorBufferSize,
		BLUETOOTH_GATT_FLAG_NONE);

	if (HRESULT_FROM_WIN32(ERROR_MORE_DATA) != hr) {
		cout << "\tERROR while getting the size of the descriptors buffer: ";
		ErrorDescription(hr);
		return hr;
	}

	/* Allocate space for the buffer. */
	if (descriptorBufferSize > 0) {
		descriptorsBuffer = (PBTH_LE_GATT_DESCRIPTOR)
			malloc(descriptorBufferSize	* sizeof(BTH_LE_GATT_DESCRIPTOR));

		if (NULL == descriptorsBuffer) {
			cout << "\tERROR while allocating space for the descriptors buffer." << endl;
		}
		else {
			RtlZeroMemory(descriptorsBuffer, descriptorBufferSize);
		}

		/* Retrieve the list of descriptors. */
		hr = BluetoothGATTGetDescriptors(
			DeviceHandle,
			characteristicsBuffer,
			descriptorBufferSize,
			descriptorsBuffer,
			&numDescriptors,
			BLUETOOTH_GATT_FLAG_NONE);

		if (FAILED(hr)) {
			cout << "\tERROR while getting the list of descriptors: ";
			ErrorDescription(hr);
			return hr;
		}

		if (numDescriptors != descriptorBufferSize) {
			cout << "\tWARNING - Mismatch between the size of the buffer and the number of characteristics." << endl;
		}
	}
	
	return hr;
}


/*************************************************************************
*
* Function:		getCharacteristic()
*
* Description:	Get the address of one of the characteristic within the
*				characteristicBuffer using its handle.
*
* Notes:		Must have called getListCharacteristics before because
*				characteristicsBuffer is needed.
*
* Parameters:	The handle of the characteristic to recover.
*
* Returns:		Address of the characteristic.
*
**************************************************************************/
PBTH_LE_GATT_CHARACTERISTIC BLE::getCharacteristicWithHandle(USHORT _characteristicHandle)
{
	for (int i = 0; i < numCharacteristics; i++) {
		if (characteristicsBuffer[i].AttributeHandle == _characteristicHandle)
			return &characteristicsBuffer[i];
	}

	return NULL;
}


/*************************************************************************
*
* Function:		enableNotifications()
*
* Description:	Enable the notifications for one descriptor.
*
* Notes:		First send 0,NULL as the parameters to
*				BluetoothGATTGetDescriptorValue in order to get the
*				data size of the descriptor's value in descValueDataSize.
*
*				BluetoothGATTGetDescriptorValue:
*				https://msdn.microsoft.com/en-us/library/windows/hardware/hh450798(v=vs.85).aspx
*
* Parameters:	None.
*
* Returns:		Result of the second call of the
*				BluetoothGATTGetDescriptorValue.
*
**************************************************************************/
HRESULT BLE::enableNotifications(PBTH_LE_GATT_DESCRIPTOR _descriptor)
{
	BTH_LE_GATT_DESCRIPTOR_VALUE newValue;

	RtlZeroMemory(&newValue, sizeof(newValue));

	newValue.DescriptorType = ClientCharacteristicConfiguration;
	newValue.ClientCharacteristicConfiguration.IsSubscribeToNotification = TRUE;

	HRESULT hr = BluetoothGATTSetDescriptorValue(
		DeviceHandle,
		_descriptor,
		&newValue,
		BLUETOOTH_GATT_FLAG_NONE);

	if (FAILED(hr)) {
		cout << "\tERROR while enabling the notifications on characteristic " << _descriptor->CharacteristicHandle << ": ";
		ErrorDescription(hr);
		return hr;
	}

	return hr;
}


/*************************************************************************
*
* Function:		registerNotificationsEvent()
*
* Description:	Register the function to call when we receive a
*				notification.
*
* Notes:		Characteristics must have had their notifications
*				enabled before calling this function.
*				
*				BluetoothGATTRegisterEvent:
*				https://msdn.microsoft.com/en-us/library/windows/hardware/hh450804(v=vs.85).aspx
*
* Parameters:	List of characteristics and their quantity that has been
*				notified.
*
* Returns:		Result of the second call of the
*				BluetoothGATTGetDescriptorValue.
*
**************************************************************************/
HRESULT BLE::registerNotificationsEvent(PBTH_LE_GATT_CHARACTERISTIC *_characteristics, USHORT _numCharacteristics) {
	BLUETOOTH_GATT_EVENT_HANDLE EventHandle;
	BTH_LE_GATT_EVENT_TYPE EventType = CharacteristicValueChangedEvent;
	BLUETOOTH_GATT_VALUE_CHANGED_EVENT_REGISTRATION EventParameterIn;

	EventParameterIn.NumCharacteristics = _numCharacteristics;
	EventParameterIn.Characteristics[0] = **_characteristics;
	
	HRESULT hr = BluetoothGATTRegisterEvent(
		DeviceHandle,
		EventType,
		&EventParameterIn,
		PSOC()->NotificationReceived,
		NULL,
		&EventHandle,
		BLUETOOTH_GATT_FLAG_NONE);

	if (FAILED(hr)) {
		cout << "\tERROR while registering the event callback: ";
		ErrorDescription(hr);
		return hr;
	}

	return hr;
}


/*************************************************************************
*
* Function:		getCharacteristicValue()
*
* Description:	Read the value of a characteristic.
*
* Notes:		BluetoothGATTGetCharacteristicValue:
*				https://msdn.microsoft.com/en-us/library/windows/hardware/hh450796(v=vs.85).aspx
*
* Parameters:	The address of the characteristic which it reads the value
*				and the address of the variable that will hold the value.
*
* Returns:		Result of the second call of the
*				BluetoothGATTGetCharacteristicValue.
*
**************************************************************************/
HRESULT BLE::getCharacteristicValue(PBTH_LE_GATT_CHARACTERISTIC _characteristic, PBTH_LE_GATT_CHARACTERISTIC_VALUE *_value)
{
	/* Determine the size of the buffer required. */
	USHORT characteristicValueDataSize;
	HRESULT hr = BluetoothGATTGetCharacteristicValue(
		DeviceHandle,
		_characteristic,
		0,
		NULL,
		&characteristicValueDataSize,
		BLUETOOTH_GATT_FLAG_NONE);

	if (HRESULT_FROM_WIN32(ERROR_MORE_DATA) != hr) {
		cout << "\tERROR while getting the size of the characteristic's value: ";
		ErrorDescription(hr);
		return hr;
	}

	/* Allocate space for the buffer. */
	*_value = (PBTH_LE_GATT_CHARACTERISTIC_VALUE)malloc(characteristicValueDataSize);

	if (NULL == *_value) {
		cout << "\tERROR while allocating space for the characteristic's value." << endl;
	}
	else {
		RtlZeroMemory(*_value, characteristicValueDataSize);
	}

	/* Retrieve the characteristic's value. */
	hr = BluetoothGATTGetCharacteristicValue(
		DeviceHandle,
		_characteristic,
		(ULONG)characteristicValueDataSize,
		*_value,
		NULL,
		BLUETOOTH_GATT_FLAG_NONE);

	if (FAILED(hr)) {
		cout << "\tERROR while getting the characteristic's value: ";
		ErrorDescription(hr);
		return hr;
	}

	return hr;
}


/*************************************************************************
*
* Function:		getDescriptorValue()
*
* Description:	Read the value of a descriptor.
*
* Notes:		BluetoothGATTGetDescriptorValue:
*				https://msdn.microsoft.com/en-us/library/windows/hardware/hh450797(v=vs.85).aspx
*
* Parameters:	The address of the descriptor which it reads the value
*				and the address of the variable that will hold the value.
*
* Returns:		Result of the second call of the
*				BluetoothGATTGetDescriptorValue.
*
**************************************************************************/
HRESULT BLE::getDescriptorValue(PBTH_LE_GATT_DESCRIPTOR _descriptor, PBTH_LE_GATT_DESCRIPTOR_VALUE *_value)
{
	/* Determine the size of the buffer required. */
	USHORT descriptorValueDataSize;
	HRESULT hr = BluetoothGATTGetDescriptorValue(
		DeviceHandle,
		_descriptor,
		0,
		NULL,
		&descriptorValueDataSize,
		BLUETOOTH_GATT_FLAG_NONE);

	if (HRESULT_FROM_WIN32(ERROR_MORE_DATA) != hr) {
		cout << "\tERROR while getting the size of the descriptor's value: ";
		ErrorDescription(hr);
		return hr;
	}

	/* Allocate space for the buffer. */
	*_value = (PBTH_LE_GATT_DESCRIPTOR_VALUE)malloc(descriptorValueDataSize);

	if (NULL == *_value) {
		cout << "\tERROR while allocating space for the descriptor's value." << endl;
	}
	else {
		RtlZeroMemory(*_value, descriptorValueDataSize);
	}

	/* Retrieve the characteristic's value. */
	hr = BluetoothGATTGetDescriptorValue(
		DeviceHandle,
		_descriptor,
		(ULONG)descriptorValueDataSize,
		*_value,
		NULL,
		BLUETOOTH_GATT_FLAG_NONE);

	if (FAILED(hr)) {
		cout << "\tERROR while getting the descriptor's value: ";
		ErrorDescription(hr);
		return hr;
	}

	return hr;
}


/*************************************************************************
*
* Function:		Connect()
*
* Description:	Main routine that calls every functions that allow us
*				to connect to the BLE device.
*
* Notes:		
*
* Parameters:	None.
*
* Returns:		None defined.
*
**************************************************************************/
void BLE::Connect(void)
{
	HRESULT result;

	/* Find de device GUID from its UUID. */
	getGuid(DeviceUUID, &Guid);

	/* Find the BLE device handle from its GUID. */
	getDeviceHandle(&Guid, &DeviceHandle);

	/* Get the list of services the device advertise. */
	result = getListServices();
	if (FAILED(result))
		exit(1);

	/* Get the list of characteristics. */
	result = getListCharacteristics();
	if (FAILED(result))
		exit(2);

	/* Get the list of descriptors. */
	result = getListDescriptors();
	if (FAILED(result))
		exit(3);

	/* Enable notifications on all characteristics. */
	BOOL notificationEnabled = FALSE;
	USHORT numNotifiedCharacteristics = 0;
	PBTH_LE_GATT_CHARACTERISTIC characteristic;
	PBTH_LE_GATT_DESCRIPTOR descriptor;

	for (int i = 0; i < numDescriptors; i++)
	{
		descriptor = &descriptorsBuffer[i];

		if (NULL == getCharacteristicWithHandle(descriptor->CharacteristicHandle)) {
			characteristic = getCharacteristicWithHandle(descriptor->CharacteristicHandle);

			if (characteristic->IsNotifiable)
			{
				if (descriptor->DescriptorType == ClientCharacteristicConfiguration)
				{
					result = enableNotifications(descriptor);
					if (SUCCEEDED(result)) {
						cout << "Successfully enabled the notifications for characteristic " << characteristic->AttributeHandle << "." << endl;
						notificationEnabled = TRUE;
						notifiedCharacteristicsBuffer[numNotifiedCharacteristics] = characteristic;
						numNotifiedCharacteristics++;
					}
				}
			}
		}
	}

	/* Register the callback event. */
	if (notificationEnabled) {
		result = registerNotificationsEvent(notifiedCharacteristicsBuffer, numNotifiedCharacteristics);

		if (SUCCEEDED(result)) {
			cout << "Successfully registered the callback event for the characteristics";
			for (int i = 0; i < numNotifiedCharacteristics; i++)
				cout << " " << notifiedCharacteristicsBuffer[i]->AttributeHandle;
			cout << "." << endl;
		}
	}






	


/*
			// Free before going to next iteration, or memory leak.
			free(pCharValueBuffer);
			pCharValueBuffer = NULL;
		}

	}




/*	
	CloseHandle(hLEDevice);



	if (GetLastError() != NO_ERROR &&
		GetLastError() != ERROR_NO_MORE_ITEMS)
	{
		// Insert error handling here.
		return 1;
	}

	return 0;
	*/
}
