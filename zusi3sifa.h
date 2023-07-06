#pragma once

#ifndef MOD_SIFA
#define MOD_SIFA		1

#include "zusi3tcp.h"

#define ID_SIFA_GRUND		0x0064
#define ID_SIFA_MELDER		0x02
#define ID_SIFA_HUPE		0x03

#define PATH_SIFA_DATA	0x0002, 0x000A, 0x0064, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000

#define DATA_CHANGED_FLAG	0

typedef struct {
	z3_led_status lm_sifa;
	byte hupe;
} zusi_sifa_data;

z3_return_code z3_sifa_data(zusi_data* zusi, word id, dword* len);
void z3_sifa_callback(zusi_data* zusi);

#endif