#pragma once

#ifndef MOD_TUEREN
#define MOD_TUEREN		1

#include "zusi3tcp.h"

#define ID_TUEREN_GRUND		0x0066
#define ID_TUEREN_FL		0x08
#define ID_TUEREN_FR		0x09
#define ID_TUEREN_FLR		0x0D
#define ID_TUEREN_ZS		0x0B
#define ID_TUEREN_ZFL		0x10
#define ID_TUEREN_ZFR		0x11
#define ID_TUEREN_TAV		0x13

#define PATH_TUEREN_DATA	0x0002, 0x000A, 0x0066, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000

#define DATA_CHANGED_FLAG	0

typedef struct {
	z3_led_status lm_fl;	//Melder Freigabe links
	z3_led_status lm_fr;	//Melder Freigabe rechts
	z3_led_status lm_flr;	//Melder Freigabe links+rechts
	z3_led_status lm_zs;	//Melder Zwangsschlie�en
	z3_led_status lm_zfl;	//Melder zentrales �ffnen links
	z3_led_status lm_zfr;	//Melder zentrales �ffnen rechts
	z3_led_status lm_tav;	//Melder TAV / Gr�nschleife
} zusi_tueren_data;

z3_return_code z3_tueren_data(zusi_data* zusi, word id, word* len);
void z3_tueren_callback(zusi_data* zusi);

#endif