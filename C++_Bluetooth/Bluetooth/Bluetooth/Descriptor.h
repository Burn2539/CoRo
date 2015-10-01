#pragma once
#include "main.h"

class Characteristic;

class Descriptor : public BTH_LE_GATT_DESCRIPTOR
{
public:
	Descriptor(Characteristic *_characteristic, PBTH_LE_GATT_DESCRIPTOR _descriptor);
	~Descriptor();

	Characteristic *parentCharacteristic;

	HRESULT readValue(PBTH_LE_GATT_DESCRIPTOR _descriptor, PBTH_LE_GATT_DESCRIPTOR_VALUE *_value);
	HRESULT writeValue(PBTH_LE_GATT_DESCRIPTOR_VALUE *_value);
};

