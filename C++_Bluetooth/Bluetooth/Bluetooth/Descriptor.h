#pragma once
#include "main.h"

class Descriptor : BTH_LE_GATT_DESCRIPTOR
{
public:
	Descriptor(Characteristic *_characteristic, PBTH_LE_GATT_DESCRIPTOR _descriptor);
	~Descriptor();

	Characteristic *parentCharacteristic;

private:
	BTH_LE_GATT_DESCRIPTOR_VALUE Value;
};

