#pragma once

/* Librairies needed to use BLE on Windows. */
#include <SDKDDKVer.h>
#include <windows.h>
#include <setupapi.h>
#include <devguid.h>
#include <regstr.h>
#include <bthdef.h>
#include <Bluetoothleapis.h>
#pragma comment(lib, "SetupAPI")
#pragma comment(lib, "BluetoothApis.lib")

/* Other librairies */
#include "crc.h"
#include <stdint.h>
#include <stdio.h>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <comdef.h>
#include <tchar.h>
#include <vector>


class BLE {
public:
	/***** Variables *****/
	/* Device */
	const wchar_t DeviceUUID[39] = TEXT("{0003CAB5-0000-1000-8000-00805F9B0131}");
	GUID Guid;
	HANDLE DeviceHandle;
	
	/* Services */
	USHORT numServices;
	PBTH_LE_GATT_SERVICE servicesBuffer = nullptr;
	
	/* Characteristics */
	USHORT numCharacteristics;
	PBTH_LE_GATT_CHARACTERISTIC characteristicsBuffer = nullptr;
	PBTH_LE_GATT_CHARACTERISTIC *notifiedCharacteristicsBuffer = nullptr;
	
	/* Descritors */
	USHORT numDescriptors;
	PBTH_LE_GATT_DESCRIPTOR descriptorsBuffer = nullptr;

	/* Data */
	struct sensors {
		uint16_t sensor1;
		//uint16_t sensor2;
		//uint16_t sensor3;
		//uint16_t sensor4;
		//uint16_t sensor5;
	};
	std::vector<sensors> CapSense;


	/***** Prototypes *****/
	static BLE *PSOC();
	void ErrorDescription(HRESULT hr);
	void Connect(void);
	void getGuid(const wchar_t *_uuid, GUID *_guid);
	void getDeviceHandle(GUID *_guid, HANDLE *_handle);
	HANDLE findDeviceHandle(GUID AGuid);
	HRESULT getListServices(void);
	HRESULT getListCharacteristics(void);
	HRESULT getListDescriptors(void);
	PBTH_LE_GATT_CHARACTERISTIC getCharacteristicWithHandle(USHORT _characteristicHandle);
	HRESULT enableNotifications(PBTH_LE_GATT_DESCRIPTOR _descriptor);
	HRESULT registerNotificationsEvent(PBTH_LE_GATT_CHARACTERISTIC *_characteristics, USHORT _numCharacteristics);
	HRESULT getCharacteristicValue(PBTH_LE_GATT_CHARACTERISTIC _characteristic, PBTH_LE_GATT_CHARACTERISTIC_VALUE *_value);
	HRESULT getDescriptorValue(PBTH_LE_GATT_DESCRIPTOR _descriptor, PBTH_LE_GATT_DESCRIPTOR_VALUE *_value);


private:
	/***** Variables *****/
	static BLE *instance;

	/***** Prototypes *****/
	static void NotificationReceived(__in BTH_LE_GATT_EVENT_TYPE EventType, __in PVOID EventOutParameter, __in PVOID Context);
};
