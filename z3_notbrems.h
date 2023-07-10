#pragma once

#ifndef MOD_NOTBREMS
#define MOD_NOTBREMS		1

#include "zusi3tcp.h"

#define ID_NB_GRUND			0x0022
#define ID_NB_STATUS		0x02
#define ID_NB_LM_SYS		0x06
#define ID_NB_LM_AKT		0x07

#define PATH_NOTBREMS_DATA	0x0002, 0x000A, 0x0022, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000

#define DATA_CHANGED_FLAG	0

typedef struct {
	z3_led_status lm_nb_sys;
	z3_led_status lb_nb_akt;
	byte status;
} zusi_notbrems_data;

z3_return_code z3_notbrems_data(zusi_data* zusi, word id, word* len);
void z3_notbrems_callback(zusi_data* zusi);

#endif