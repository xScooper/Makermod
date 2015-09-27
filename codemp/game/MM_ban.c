#if 0
#ifndef MM_RELEASE
#include <winsock2.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <process.h>
#include "g_local.h"

#pragma comment(lib, "ws2_32.lib")

static qboolean networkInit = qfalse;

//Mark
typedef unsigned char IP_t[4]; 

typedef struct IPData_s{ 
	IP_t ip; 
	struct { 
		char *host; 
		qboolean busy; 
	}Hostname; 
}IPData_t;

IPData_t		IPData[64];


void Lmd_IPs_Init() {
	WSADATA wsaData;
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);

	memset( IPData, 0, 64 * sizeof(IPData[0]) );

    if (iResult != 0) {
        return;
    }
	networkInit = qtrue;
}


char* Lmd_IPs_QueryHostname(IP_t ip) {
	struct hostent *remoteHost;
    struct in_addr addr;

	if(!networkInit)
		return "";

	//Might be able to directly set it to the contents of IP_t...
    addr.s_addr = inet_addr(va("%u.%u.%u.%u", ip[0], ip[1], ip[2], ip[3]));
    if (addr.s_addr == INADDR_NONE) 
		return "";

	remoteHost = gethostbyaddr((char *) &addr, 4, AF_INET);
	if(!remoteHost)
		return "";
	return remoteHost->h_name;
}

unsigned int _stdcall Lmd_IPs_IdentifyHostnameCall(IPData_t* data) {

	data->Hostname.host = G_NewString2(Lmd_IPs_QueryHostname(data->ip));

	//ServerMessage(data->Hostname.host, NULL);
	//G_Printf(data->Hostname.host);

	data->Hostname.busy = qfalse;

	return 0;
}
IPData_t *Lmd_IPs_GetIPData(IP_t ip)
{
	int i, j;

	for(i = 0; i < 64; i++)
	{
		if(IPData[i].ip[0] == 0)
		{
			memcpy(&IPData[i].ip, ip, sizeof(IP_t));
			//for(j = 0; j < 4; j++)
		//		IPData[i].ip[j] = ip[j];
			return &IPData[i];
		}
		if(((unsigned int)IPData[i].ip[0] == (unsigned int)ip[0]) &&
			((unsigned int)IPData[i].ip[1] == (unsigned int)ip[1]) &&
			((unsigned int)IPData[i].ip[2] == (unsigned int)ip[2]) &&
			((unsigned int)IPData[i].ip[3] == (unsigned int)ip[3]))
			return &IPData[i];
	}
	return NULL;
}

void Lmd_IPs_IdentifyHostname(IP_t ip) {
	IPData_t *data;

	if(ip[0] == 0) {
		return;
	}

	data = Lmd_IPs_GetIPData(ip);

	if(data->Hostname.host || data->Hostname.busy)
		return;

	data->Hostname.busy = qtrue;

//	CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Lmd_IPs_IdentifyHostnameCall, data, 0, NULL);
	_beginthreadex(NULL, 0, (unsigned int (__stdcall *)(void *))Lmd_IPs_IdentifyHostnameCall, data, 0, NULL);
}
#endif
#endif