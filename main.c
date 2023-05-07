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

#pragma comment(lib,"ws2_32.lib") //Winsock Library


SOCKET sock;

float speed, brake;


void on_data_arrive(word key, word id)
{
	if (id == 0x0001)
		printf("Geschw.: %.2f km/h\r\n", speed * 3.6);
	if (id == 0x0002)
		printf("Druck HL: %.3f bar\r\n", brake);
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

	while (1) 

	{
		Sleep(100);

		len = recv(s, (byte*)&buf, 1024, 0);

		if (len > 0)
			z3_put_bytes(&zusi, &buf, (word)len);

		z3_decode(&zusi);

	}

	closesocket(s);
	WSACleanup();

	// Transmit some bytes
	len = z3_bytes_sent(&zusi, 20);

	

	return (0);
}


/*
void put_byte(zusi_data* zusi, byte data)
{
	z3_put_bytes(zusi, (byte*)&data, sizeof(byte));
}

void put_word(zusi_data* zusi, word data)
{
	z3_put_bytes(zusi, (byte*)&data, sizeof(word));
}

void put_dword(zusi_data* zusi, dword data)
{
	z3_put_bytes(zusi, (byte*)&data, sizeof(dword));
}

void put_float(zusi_data* zusi, float data)
{
	z3_put_bytes(zusi, (byte*)&data, sizeof(float));
}

void put_double(zusi_data* zusi, double data)
{
	z3_put_bytes(zusi, (byte*)&data, sizeof(double));
}

void put_string(zusi_data* zusi, char* data)
{
	z3_put_bytes(zusi, (byte*)data, strlen(data));
}
*/

//Eine ACK_HELLO Nachricht
/*
put_dword(&zusi, 0x00000000);
put_word(&zusi, 0x0001);
put_dword(&zusi, 0x00000000);
put_word(&zusi, 0x0002);
put_dword(&zusi, 0x00000007);
put_word(&zusi, ID_ZUSIVER);
put_string(&zusi, "1.0");
z3_decode(&zusi);
put_string(&zusi, ".0");
put_dword(&zusi, 0x00000004);
put_word(&zusi, ID_ZUSIINFO);
put_string(&zusi, "01");
put_dword(&zusi, 0x00000003);
put_word(&zusi, ID_HELLOACK);
put_byte(&zusi, 00);
put_dword(&zusi, 0xffffffff);
put_dword(&zusi, 0xffffffff);
z3_decode(&zusi);
//Eine DATA_FTD Nachricht
put_dword(&zusi, 0x00000000);
put_word(&zusi, 0x0002);
put_dword(&zusi, 0x00000000);
put_word(&zusi, 0x000a);
put_dword(&zusi, 0x00000006);
put_word(&zusi, 0x0001);
z3_decode(&zusi);
put_float(&zusi, 123.4);
put_dword(&zusi, 0x0000000a);
put_word(&zusi, 0x0002);
z3_decode(&zusi);
put_double(&zusi, 1234.5);
put_dword(&zusi, 0xffffffff);
put_dword(&zusi, 0xffffffff);
z3_decode(&zusi);
*/