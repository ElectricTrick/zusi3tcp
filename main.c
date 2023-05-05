#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "zusi3tcp.h"

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

void put_string(zusi_data* zusi, char* data)
{
	z3_put_bytes(zusi, (byte*)data, strlen(data));
}

int main()
{


	zusi_data zusi;

	float a = 0.00;
	float b = 0.00;

	if (z3_init(&zusi, 4096) != z3_ok)
		printf("Failed to allocate memory!\r\n");

	z3_add_needed_data(&zusi, ZUSI_CAB_DATA, 0x0001, &b);

	//Eine ACK_HELLO Nachricht
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
	put_dword(&zusi, 0xffffffff);
	put_dword(&zusi, 0xffffffff);
	z3_decode(&zusi);

	z3_hello_msg(&zusi, 01, "Hallo Welt", "1.0");

	byte* data = z3_get_send_buffer(&zusi);
	dword len = z3_bytes_sent(&zusi, 0);

	// Transmit some bytes
	len = z3_bytes_sent(&zusi, 15);

	return (0);
}