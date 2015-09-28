#pragma once

#include <stdio.h>
#include <tchar.h>
#include <stdint.h>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>

#include "Descriptor.h"
#include "Characteristic.h"
#include "Service.h"
#include "Device.h"
#include "Bluetooth.h"

extern int notificationsReceived;

void ErrorDescription(HRESULT hr);
