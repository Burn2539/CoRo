#pragma once
#include "main.h"

class Service : BTH_LE_GATT_SERVICE
{
public:
	Service(Device *_device, PBTH_LE_GATT_SERVICE _service);
	~Service();

	Device *parentDevice;

	std::vector<Characteristic*> characteristics;
	HRESULT retrieveListCharacteristics();
	USHORT numCharacteristics;
};

