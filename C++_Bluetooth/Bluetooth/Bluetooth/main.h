#pragma once

/* Basic librairies. */
#include <stdio.h>
#include <tchar.h>
#include <stdint.h>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>

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

/* My classes. */
#include "Bluetooth.h"
#include "Descriptor.h"
#include "Characteristic.h"
#include "Service.h"
#include "Device.h"

/* Macros */
#define SENSOR0 1
#define SENSOR1 2
#define SENSOR2 3
#define SENSOR3 4
#define SENSOR4 5

extern int notificationsReceived;

void newSensorDataReceived(__in BTH_LE_GATT_EVENT_TYPE EventType, __in PVOID EventOutParameter, __in PVOID Context);
void ErrorDescription(HRESULT hr);
