#include "Service.h"

using namespace std;


Service::Service(Device *_device, PBTH_LE_GATT_SERVICE _service)
{
	this->parentDevice = _device;

	this->AttributeHandle = _service->AttributeHandle;
	this->ServiceUuid = _service->ServiceUuid;
}


Service::~Service()
{
	for (int i = 0; i < this->numCharacteristics; i++)
		delete this->characteristics[i];
	cout << "Service has been deleted." << endl;
}


/*************************************************************************
*
* Function:		retrieveListCharacteristics()
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
HRESULT Service::retrieveListCharacteristics()
{
	PBTH_LE_GATT_CHARACTERISTIC characteristicsBuffer;
	USHORT characteristicBufferSize;

	/* Determine the size of the buffer required. */
	HRESULT hr = BluetoothGATTGetCharacteristics(
		this->parentDevice->Handle,
		this,
		0,
		NULL,
		&characteristicBufferSize,
		BLUETOOTH_GATT_FLAG_NONE);

	if (HRESULT_FROM_WIN32(ERROR_MORE_DATA) != hr) {
		cout << "ERROR while getting the size of the characteristics buffer: ";
		ErrorDescription(hr);
		return hr;
	}

	/* Allocate space for the buffer. */
	if (characteristicBufferSize > 0)
	{
		characteristicsBuffer = (PBTH_LE_GATT_CHARACTERISTIC)
			malloc(characteristicBufferSize * sizeof(BTH_LE_GATT_CHARACTERISTIC));

		if (NULL == characteristicsBuffer) {
			cout << "ERROR while allocating space for the characteristics buffer." << endl;
		}
		else {
			RtlZeroMemory(characteristicsBuffer,
				characteristicBufferSize * sizeof(BTH_LE_GATT_CHARACTERISTIC));
		}

		/* Retrieve the list of characteristics. */
		hr = BluetoothGATTGetCharacteristics(
			this->parentDevice->Handle,
			this,
			characteristicBufferSize,
			characteristicsBuffer,
			&this->numCharacteristics,
			BLUETOOTH_GATT_FLAG_NONE);

		if (FAILED(hr)) {
			cout << "ERROR while getting the list of characteristics: ";
			ErrorDescription(hr);
			return hr;
		}

		if (this->numCharacteristics != characteristicBufferSize)
			cout << "WARNING - Mismatch between the size of the buffer and the number of characteristics." << endl;

		/* Create the characteristics. */
		for (int i = 0; i < this->numCharacteristics; i++)
			this->characteristics.push_back( new Characteristic( this, &characteristicsBuffer[i] ) );

		free(characteristicsBuffer);
	}

	return hr;
}
