#pragma once
#include "main.h"

class Device
{
public:
	Device(GUID _guid);
	~Device();

	HANDLE Handle;

	std::vector<Service*> services;

	HRESULT retrieveListServices();
	USHORT numServices;

private:
	GUID Guid;
	HANDLE getHandle(GUID AGuid);
};

