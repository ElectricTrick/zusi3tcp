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


	put_dword(&zusi, 0x00000000);
	put_word(&zusi, 0x0001);
	put_dword(&zusi, 0x00000000);
	put_word(&zusi, 0x0002);
	put_dword(&zusi, 0x00000007);
	put_word(&zusi, ID_ZUSIVER);
	put_string(&zusi, "1.0\0");
	z3_decode(&zusi);
	put_string(&zusi, ".0\0");
	put_dword(&zusi, 0x00000004);
	put_word(&zusi, ID_ZUSIINFO);
	put_string(&zusi, "01");
	put_dword(&zusi, 0x00000003);
	put_word(&zusi, ID_HELLOACK);
	put_byte(&zusi, 00);
	put_dword(&zusi, 0xffffffff);
	put_dword(&zusi, 0xffffffff);
	z3_decode(&zusi);


	word protocol_version = 01;
	word client_type = 01;
	char* client_name = "Hallo Welt\0";
	char* client_version = "1.0\0";

	z3_write_node(&zusi, 0x0001);
	z3_write_node(&zusi, 0x0001);
	z3_write_attribute(&zusi, 0x0001, &protocol_version, sizeof(word));
	z3_write_attribute(&zusi, 0x0002, &client_type, sizeof(word));
	z3_write_attribute(&zusi, 0x0003, client_name, strlen(client_name));
	z3_write_attribute(&zusi, 0x0003, client_version, strlen(client_version));
	z3_write_node(&zusi, 0);
	z3_write_node(&zusi, 0);

	byte* data = z3_get_send_buffer(&zusi);
	dword len = z3_bytes_sent(&zusi, 0);

	// Transmit some bytes
	len = z3_bytes_sent(&zusi, 15);

	return (0);
}