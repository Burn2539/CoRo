#include "Characteristic.h"

using namespace std;


Characteristic::Characteristic(Service *_service, PBTH_LE_GATT_CHARACTERISTIC _characteristic)
{
	parentService = _service;
}


Characteristic::~Characteristic()
{
}


/*************************************************************************
*
* Function:		retrieveListDescriptors()
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
HRESULT Characteristic::retrieveListDescriptors()
{
	PBTH_LE_GATT_DESCRIPTOR descriptorsBuffer;
	USHORT descriptorBufferSize;

	/* Determine the size of the buffer required. */
	USHORT descriptorBufferSize;
	HRESULT hr = BluetoothGATTGetDescriptors(
		parentService->parentDevice->Handle,
		this,
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
			malloc(descriptorBufferSize * sizeof(BTH_LE_GATT_DESCRIPTOR));

		if (NULL == descriptorsBuffer) {
			cout << "\tERROR while allocating space for the descriptors buffer." << endl;
		}
		else {
		RtlZeroMemory(descriptorsBuffer, descriptorBufferSize);
		}

		/* Retrieve the list of descriptors. */
		hr = BluetoothGATTGetDescriptors(
			parentService->parentDevice->Handle,
			this,
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

		/* Create the descriptors. */
		for (int i = 0; i < numDescriptors; i++)
			descriptors.push_back(new Descriptor(this, &descriptorsBuffer[i]));

		free(descriptorsBuffer);
	}
	
	return hr;
}
