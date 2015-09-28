#pragma once
#include "main.h"

class Characteristic : BTH_LE_GATT_CHARACTERISTIC
{
public:
	Characteristic(Service *_service, PBTH_LE_GATT_CHARACTERISTIC _characteristic);
	~Characteristic();

	Service *parentService;

	std::vector<Descriptor*> descriptors;
	HRESULT retrieveListDescriptors();
	USHORT numDescriptors;

private:
	BTH_LE_GATT_CHARACTERISTIC_VALUE Value;
	bool notificationEnabled;
};

