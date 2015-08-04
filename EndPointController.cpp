// EndPointController.cpp : Defines the entry point for the console application.
//
#include <stdio.h>
#include <wchar.h>
#include <tchar.h>
#include "windows.h"
#include "Mmdeviceapi.h"
#include "PolicyConfig.h"
#include "Propidl.h"
#include "Functiondiscoverykeys_devpkey.h"

HRESULT SetDefaultAudioPlaybackDevice(LPCWSTR devID)
{	
	IPolicyConfigVista *pPolicyConfig;
	ERole reserved = eConsole;

    HRESULT hr = CoCreateInstance(__uuidof(CPolicyConfigVistaClient), 
		NULL, CLSCTX_ALL, __uuidof(IPolicyConfigVista), (LPVOID *)&pPolicyConfig);
	if (SUCCEEDED(hr))
	{
		hr = pPolicyConfig->SetDefaultEndpoint(devID, reserved);
		pPolicyConfig->Release();
	}
	return hr;
}

// EndPointController.exe [NewDefaultDeviceID]
int _tmain(int argc, _TCHAR* argv[])
{
	// read the command line option, -1 indicates list devices.
	int option = -1;
	LPWSTR wstrOption = NULL;
	size_t strlenOption = 0;
	if(argc == 2) {
		wstrOption = argv[1];
		strlenOption = wcslen(argv[1]);
		int i = 0;
		while(wstrOption[i] != NULL && iswdigit(wstrOption[i]))
			i++;
		if(wstrOption[i] == NULL) {
			// argument is a digit
			option = _wtoi(argv[1]);
		}
	}

	HRESULT hr = CoInitialize(NULL);
	if (SUCCEEDED(hr))
	{
		IMMDeviceEnumerator *pEnum = NULL;
		// Create a multimedia device enumerator.
		hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL,
			CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void**)&pEnum);
		if (SUCCEEDED(hr))
		{
			IMMDeviceCollection *pDevices;
			// Enumerate the output devices.
			hr = pEnum->EnumAudioEndpoints(eRender, DEVICE_STATE_ACTIVE, &pDevices);
			if (SUCCEEDED(hr))
			{
				UINT count;
				pDevices->GetCount(&count);
				if (SUCCEEDED(hr))
				{
					for (UINT i = 0; i < count; i++)
					{
						IMMDevice *pDevice;
						hr = pDevices->Item(i, &pDevice);
						if (SUCCEEDED(hr))
						{
							LPWSTR wstrID = NULL;
							hr = pDevice->GetId(&wstrID);
							if (SUCCEEDED(hr))
							{
								IPropertyStore *pStore;
								hr = pDevice->OpenPropertyStore(STGM_READ, &pStore);
								if (SUCCEEDED(hr))
								{
									PROPVARIANT friendlyName;
									PropVariantInit(&friendlyName);
									hr = pStore->GetValue(PKEY_Device_FriendlyName, &friendlyName);
									if (SUCCEEDED(hr))
									{
										// if no options, print the device
										// otherwise, find the selected device and set it to be default
										if (i+1 == option) 
										{
											SetDefaultAudioPlaybackDevice(wstrID);
											printf("Audio Device %d: %ws selected\n",i+1, friendlyName.pwszVal);
										}
										else if (argc == 2 && wcsncmp(friendlyName.pwszVal, wstrOption, strlenOption) == 0) 
										{
											SetDefaultAudioPlaybackDevice(wstrID);
											printf("Audio Device %d: %ws selected\n",i+1, friendlyName.pwszVal);
										}
										else
										{
											printf("Audio Device %d: %ws\n",i+1, friendlyName.pwszVal);
										}
										PropVariantClear(&friendlyName);
									}
									pStore->Release();
								}
							}
							pDevice->Release();
						}
					}
				}
				pDevices->Release();
			}
			pEnum->Release();
		}
	}
	return hr;
}