#pragma once
#include "main.h"

class Device;
class Characteristic;

class Service : public BTH_LE_GATT_SERVICE
{
public:
	Service(Device *_device, PBTH_LE_GATT_SERVICE _service);
	~Service();

	Device *parentDevice;

	std::vector<Characteristic*> characteristics;
	HRESULT retrieveListCharacteristics();
	USHORT numCharacteristics;
};

