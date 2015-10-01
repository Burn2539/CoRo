#pragma once
#include "main.h"

class Service;
class Descriptor;

class Characteristic : public BTH_LE_GATT_CHARACTERISTIC
{
public:
	Characteristic(Service *_service, PBTH_LE_GATT_CHARACTERISTIC _characteristic);
	~Characteristic();

	Service *parentService;

	std::vector<Descriptor*> descriptors;
	HRESULT retrieveListDescriptors();
	USHORT numDescriptors;

	HRESULT subscribeToNotification(__in bool _SubscritbeToNotification, __in bool _SubscribeToIndication, __in PFNBLUETOOTH_GATT_EVENT_CALLBACK _callbackFunction);
	HRESULT unsubscribeToNotification(void);
	bool notificationEnabled;
	BLUETOOTH_GATT_EVENT_HANDLE EventHandle;

	HRESULT readValue(PBTH_LE_GATT_CHARACTERISTIC_VALUE *_value);
	HRESULT writeValue(PBTH_LE_GATT_CHARACTERISTIC_VALUE _value);
};

