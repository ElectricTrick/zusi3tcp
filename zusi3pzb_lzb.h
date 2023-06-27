#pragma once

#ifndef ZUSI3PZB
#define MOD_PZBLZB		1

#include "zusi3tcp.h"


#define ID_PZBGRUND			0x0065
#define ID_PZB_HUPE			0x09
#define ID_PZB_LM500		0x33
#define ID_PZB_LM1000		0x2f
#define ID_PZB_LMB40		0x34
#define ID_PZB_O			0x30
#define ID_PZB_M			0x31
#define ID_PZB_U			0x32
#define PZB_SUBNODE			3

#define PATH_PZB_DATA		0x0002, 0x000A, 0x0065, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000

#define DATA_CHANGED_FLAG	0


typedef struct {
	byte lm_500hz;
	byte lm_1000hz;
	byte lm_befehl;
	byte lm_za_o;
	byte lm_za_m;
	byte lm_za_u;
	byte hupe;
} zusi_pzb_data;

z3_return_code z3_pzb_data(zusi_data* zusi, word id, dword* len);
void z3_pzb_data_callback(zusi_data* zusi);

#endif