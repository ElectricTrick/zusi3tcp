#pragma once

#ifndef MOD_TUEREN
#define MOD_TUEREN		1

#include "zusi3tcp.h"

typedef struct {
	z3_led_status lm_fl;	//Melder Freigabe links
	z3_led_status lm_fr;	//Melder Freigabe rechts
	z3_led_status lm_flr;	//Melder Freigabe links+rechts
	z3_led_status lm_zs;	//Melder Zwangsschließen
	z3_led_status lm_zfl;	//Melder zentrales öffnen links
	z3_led_status lm_zfr;	//Melder zentrales öffnen rechts
	z3_led_status lm_tav;	//Melder TAV / Grünschleife
};

#endif