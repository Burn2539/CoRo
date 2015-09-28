#include "Descriptor.h"

using namespace std;


Descriptor::Descriptor(Characteristic *_characteristic, PBTH_LE_GATT_DESCRIPTOR _descriptor)
{
	parentCharacteristic = _characteristic;
}


Descriptor::~Descriptor()
{
}
