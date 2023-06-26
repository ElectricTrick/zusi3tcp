#define _WINSOCKAPI_ 
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <tchar.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "zusi3tcp.h"
#include "zusi3pzb_lzb.h"


#pragma comment(lib,"ws2_32.lib") //Winsock Library


SOCKET sock;

float speed, brake;

zusi_pzb_data pzb90 = { 1, 1, 1, 1, 1, 1, 1 };


void on_data_arrive(word key, word id)
{
	if (id == 0x0001)
		printf("Geschw.: %.2f km/h\r\n", speed * 3.6);
	else if (id == 0x0002)
		printf("Druck HL: %.3f bar\r\n", brake);
	else if (id == 0x0065)
		printf("PZB-Melder: 85 %u 70 %u 55 %u 1000Hz %u 500Hz %u Befehl %u\r\n", pzb90.lm_za_o, pzb90.lm_za_m,
			pzb90.lm_za_u, pzb90.lm_1000hz, pzb90.lm_500hz, pzb90.lm_befehl);
}

void print_ids(word* ids)
{
	for (word n = 0; n <= 9; n++) {
		printf("id %d: %d\r\n", n, ids[n]);
	}
}


int main()
{

	zusi_data zusi;
	byte buf[1024] = { 0 };

	float a = 0.00;
	double b = 0.00;


	if (z3_init(&zusi, 4096, &on_data_arrive) != z3_ok)
		printf("Failed to allocate memory!\r\n");

	zusi_add_needed_data(&zusi, ZUSI_CAB_DATA, 0x0001, &speed);
	zusi_add_needed_data(&zusi, ZUSI_CAB_DATA, 0x0002, &brake);
	zusi_add_needed_data(&zusi, ZUSI_CAB_DATA, 0x0065, &pzb90);

	WSADATA wsa;
	SOCKADDR_IN addr;

	if (WSAStartup(MAKEWORD(2, 0), &wsa) != NO_ERROR) {
		printf("Winsock not available\n");
		return(1);
	}

	SOCKET s = socket(AF_INET, SOCK_STREAM, 0);

	if (s == INVALID_SOCKET) {
		printf("Socket could not be created\n");
		WSACleanup();
		return(2);
	}

	memset(&addr, 0, sizeof(SOCKADDR_IN));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(1436);
	InetPton(AF_INET, _T("127.0.0.1"), &addr.sin_addr.s_addr);

	printf("Verbinde mit Server...\n");

	if (connect(s, (SOCKADDR*)&addr, sizeof(SOCKADDR)) == SOCKET_ERROR) {
		printf("Unable to connect\n");
		closesocket(s);
		WSACleanup();
		return(3);
	}

	u_long iMode = 0;
	//Non-blocking mode
	if (ioctlsocket(s, FIONBIO, &iMode) != NO_ERROR) {
		printf("Non-blocking mode for socket failed\n");
	}

	printf("Anmeldung...\n");

	zusi_hello_msg(&zusi, 02, "Hallo Welt", "1.0");

	//byte* data = z3_get_send_buffer(&zusi);
	//dword len = z3_bytes_sent(&zusi, 0);

	dword len = send(s, (char*)z3_get_send_buffer(&zusi), z3_bytes_sent(&zusi, 0), 0);

	z3_bytes_sent(&zusi, len);
	
	Sleep(100);

	len = recv(s, (byte*)&buf, 1024, 0);

	z3_put_bytes(&zusi, &buf, len);
	z3_decode(&zusi);

	zusi_needed_data_msg(&zusi);

	//data = z3_get_send_buffer(&zusi);
	//len = z3_bytes_sent(&zusi, 0);

	len = send(s, (char*)z3_get_send_buffer(&zusi), z3_bytes_sent(&zusi, 0), 0);

	z3_bytes_sent(&zusi, len);

	unsigned int _tick = 0;

	u_long ba;


	while (1) 

	{
		Sleep(100);

		len = recv(s, (byte*)&buf, 1024, 0);
		if (len > 0) {
			z3_put_bytes(&zusi, &buf, (word)len);
		}


		z3_decode(&zusi);

	}

	closesocket(s);
	WSACleanup();

	// Transmit some bytes
	len = z3_bytes_sent(&zusi, 20);

	

	return (0);
}
