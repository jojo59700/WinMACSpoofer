
#define MALLOC(x) HeapAlloc(GetProcessHeap(), 0, (x))
#define FREE(x) HeapFree(GetProcessHeap(), 0, (x))
#define WORKING_BUFFER_SIZE 15000
#define MAX_TRIES 3
#define MAX_KEY_LENGTH 255
#define MAX_VALUE_NAME 16383

#define TOTALBYTES 8192
#define BYTEINCREMENT 4096

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <iostream>
#include <string>
#include <Windows.h>
#include <winsock2.h>
#include <iphlpapi.h>
#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>
#include <malloc.h>

#pragma comment(lib,"IPHlpApi.Lib")

using namespace std;

void getNetworkInfo();//Locate the active "Wi-Fi" adapter
void setNewMac(); //Set the "Wi-Fi" adapter's new mac address
void revertToOriginalMac(); //change the mac Address back to the original
void setQueryKey();
void QueryKey(HKEY);
void queryRegValue(string); //Find the subkey where the where the active "Wi-Fi" is located

int main(){

	//setQueryKey();
	//getNetworkInfo();
	//setNewMac();
	//revertToOriginalMac();

	cout << endl;
	system("pause");
}

//Update the 
void setNewMac(){
	HKEY hKey;
	LPCTSTR sk = TEXT("SYSTEM\\CurrentControlSet\\Control\\Class\\{4d36e972-e325-11ce-bfc1-08002be10318}\\0000");
	LPCTSTR keyData = TEXT("0A7777777777");
	LONG retval = RegOpenKeyEx(HKEY_LOCAL_MACHINE, sk, 0, KEY_ALL_ACCESS, &hKey);

	if (retval == ERROR_SUCCESS) {
		printf("Success opening key.");
	}
	else {
		printf("Error opening key.");
	}
	retval = (RegSetValueEx(hKey, TEXT("NetworkAddress"), 0, REG_SZ, (BYTE*)keyData, lstrlen(keyData) + 1));

	if (retval == ERROR_SUCCESS){
		printf("Success setting key.");
	}
	else {
		printf("Error setting key.");
	}
	retval = RegCloseKey(hKey);


	system("netsh interface set interface ""Wi-Fi"" DISABLED");
	system("netsh interface set interface ""Wi-Fi"" ENABLED");
	//system("netsh wlan connect name=DG1670AD2");



}
//Delete the regkey "NetworkAddress" from the registry
void revertToOriginalMac(){

	HKEY hKey;
	LPCTSTR sk = TEXT("SYSTEM\\CurrentControlSet\\Control\\Class\\{4d36e972-e325-11ce-bfc1-08002be10318}\\0000");
	LONG retval = RegOpenKeyEx(HKEY_LOCAL_MACHINE, sk, 0, KEY_ALL_ACCESS, &hKey);

	if (retval == ERROR_SUCCESS) {
		printf("Success opening key.");
	}
	else {
		printf("Error opening key.");
	}
	retval = RegDeleteValue(hKey, "NetworkAddress");

	retval = RegCloseKey(hKey);


	system("netsh interface set interface ""Wi-Fi"" DISABLED");
	system("netsh interface set interface ""Wi-Fi"" ENABLED");
	//system("netsh wlan connect name=DG1670AD2");

}

//Access the network Information (operStatus, friendlyname, adaptername)
void getNetworkInfo(){
	
	
	/* Declare and initialize variables */

	PWCHAR networkAdap = NULL;
	PCHAR driverDesc = NULL;

	DWORD dwSize = 0;
	DWORD dwRetVal = 0;

	unsigned int i = 0;

	// Set the flags to pass to GetAdaptersAddresses
	ULONG flags = GAA_FLAG_INCLUDE_PREFIX;

	// default to unspecified address family (both)
	ULONG family = AF_UNSPEC;

	LPVOID lpMsgBuf = NULL;

	PIP_ADAPTER_ADDRESSES pAddresses = NULL;
	ULONG outBufLen = 0;
	ULONG Iterations = 0;

	PIP_ADAPTER_ADDRESSES pCurrAddresses = NULL;

	// Allocate a 15 KB buffer to start with.
	outBufLen = WORKING_BUFFER_SIZE;

	do {

		pAddresses = (IP_ADAPTER_ADDRESSES *)MALLOC(outBufLen);
		if (pAddresses == NULL) {
			printf
				("Memory allocation failed for IP_ADAPTER_ADDRESSES struct\n");
			exit(1);
		}

		dwRetVal =
			GetAdaptersAddresses(family, flags, NULL, pAddresses, &outBufLen);

		if (dwRetVal == ERROR_BUFFER_OVERFLOW) {
			FREE(pAddresses);
			pAddresses = NULL;
		}
		else {
			break;
		}

		Iterations++;

	} while ((dwRetVal == ERROR_BUFFER_OVERFLOW) && (Iterations < MAX_TRIES));

	if (dwRetVal == NO_ERROR) {
		// If successful, output some information from the data we received
		pCurrAddresses = pAddresses;

		//Cycles through the Network Adapters, ouputs name, type and operational status
		while (pCurrAddresses) {
			printf("\tAdapter name: %s\n", pCurrAddresses->AdapterName);
			printf("\tDescription: %wS\n", pCurrAddresses->Description);
			printf("\tFriendly name: %wS\n", pCurrAddresses->FriendlyName);

			if (pCurrAddresses->PhysicalAddressLength != 0) {
				printf("\tPhysical address: ");
				for (i = 0; i < (int)pCurrAddresses->PhysicalAddressLength;
					i++) {
					if (i == (pCurrAddresses->PhysicalAddressLength - 1))
						printf("%.2X\n",
						(int)pCurrAddresses->PhysicalAddress[i]);
					else
						printf("%.2X-",
						(int)pCurrAddresses->PhysicalAddress[i]);
				}
			}

			//check to see if network adapter is "Wi-fi"
			printf("\tOperStatus: %ld\n", pCurrAddresses->OperStatus);
			networkAdap = pCurrAddresses->FriendlyName;
			if (pCurrAddresses->OperStatus == 1 && *networkAdap == 87){ //87 == "Wi-Fi"
				printf("\t***Success******8:  Friendly name == %wS\n", pCurrAddresses->FriendlyName);
				driverDesc = pCurrAddresses->AdapterName;
			}
			pCurrAddresses = pCurrAddresses->Next;
		}
		
	}
	else {
		printf("Call to GetAdaptersAddresses failed with error: %d\n",
			dwRetVal);
		if (dwRetVal == ERROR_NO_DATA)
			printf("\tNo addresses were found for the requested parameters\n");
		else {

			if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
				FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
				NULL, dwRetVal, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
				// Default language
				(LPTSTR)& lpMsgBuf, 0, NULL)) {
				printf("\tError: %s", lpMsgBuf);
				LocalFree(lpMsgBuf);
				if (pAddresses)
					FREE(pAddresses);
				exit(1);
			}
		}
	}

	if (pAddresses) {
		FREE(pAddresses);
	}


}

//Set the registry key
void setQueryKey(){
	HKEY hTestKey;
	LPCSTR dir = TEXT("SYSTEM\\CurrentControlSet\\Control\\Class\\{4d36e972-e325-11ce-bfc1-08002be10318}");
	LONG checkReg = RegOpenKeyEx(HKEY_LOCAL_MACHINE, dir, 0, KEY_READ, &hTestKey);
	cout << checkReg;

	if (checkReg == ERROR_SUCCESS)
	{
		QueryKey(hTestKey);
	}
	RegCloseKey(hTestKey);

}


void QueryKey(HKEY hKey)
{
	
	TCHAR    achKey[MAX_KEY_LENGTH];   // buffer for subkey name
	DWORD    cbName;                   // size of name string 
	TCHAR    achClass[MAX_PATH] = TEXT("");  // buffer for class name 
	DWORD    cchClassName = MAX_PATH;  // size of class string 
	DWORD    cSubKeys = 0;               // number of subkeys 
	DWORD    cbMaxSubKey;              // longest subkey size 
	DWORD    cchMaxClass;              // longest class string 
	DWORD    cValues;              // number of values for key 
	DWORD    cchMaxValue;          // longest value name 
	DWORD    cbMaxValueData;       // longest value data 
	DWORD    cbSecurityDescriptor; // size of security descriptor 
	FILETIME ftLastWriteTime;      // last write time 

	DWORD i, retCode;

	TCHAR  achValue[MAX_VALUE_NAME];
	DWORD cchValue = MAX_VALUE_NAME;

	// Get the class name and the value count. 
	retCode = RegQueryInfoKey(
		hKey,                    // key handle 
		achClass,                // buffer for class name 
		&cchClassName,           // size of class string 
		NULL,                    // reserved 
		&cSubKeys,               // number of subkeys 
		&cbMaxSubKey,            // longest subkey size 
		&cchMaxClass,            // longest class string 
		&cValues,                // number of values for this key 
		&cchMaxValue,            // longest value name 
		&cbMaxValueData,         // longest value data 
		&cbSecurityDescriptor,   // security descriptor 
		&ftLastWriteTime);       // last write time 

	// Enumerate the subkeys, until RegEnumKeyEx fails.

	if (cSubKeys)
	{
		printf("\nNumber of subkeys: %d\n", cSubKeys);

		for (i = 0; i < cSubKeys; i++)
		{
			cbName = MAX_KEY_LENGTH;
			retCode = RegEnumKeyEx(hKey, i, achKey, &cbName, NULL, NULL, NULL, &ftLastWriteTime);
			string subkey;

			if (retCode == ERROR_SUCCESS)
			{
				_tprintf(TEXT("(%d) %s\n"), i + 1, achKey);
				subkey = achKey;
				queryRegValue(subkey);
			};
		}
	}
}

//Find the subkey where the where the active "Wi-Fi" is located
void queryRegValue(string subKey){

	//set up our variables and buffers.
	DWORD dwType = REG_SZ;
	unsigned char szVersion[1024];
	DWORD dwDataSize;
	memset(szVersion, 0, 32);
	string key = "SYSTEM\\CurrentControlSet\\Control\\Class\\{4d36e972-e325-11ce-bfc1-08002be10318}\\" + subKey;

	//Convert string to windows data type
	LPCTSTR findKey = key.c_str();

	// open the key for reading.
	HKEY hkeyDXVer;
	long lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, findKey,
		0, KEY_READ, &hkeyDXVer);

	if (ERROR_SUCCESS == lResult)
	{

		// read the version value
		lResult = RegQueryValueEx(hkeyDXVer, "NetCfgInstanceId", 0, &dwType, (unsigned char*)&szVersion, &dwDataSize);
		if (ERROR_SUCCESS == lResult)
		{
			cout << "NetCfgInstanceId = " << szVersion <<endl;
		}
	}

}