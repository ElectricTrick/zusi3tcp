
#include "z3_pzblzb.h"


byte _pzb_flags = 0;

z3_return_code z3_pzb_data(zusi_data* zusi, word id, dword* len)
{
	z3_return_code ret = z3_ok;

	static zusi_pzb_data* var = NULL;
	if (var == NULL) {
		for (byte n = 0; n < MAX_NEEDED_DATA; n++) {
			if (zusi->map[n].key == ZUSI_CAB_DATA && zusi->map[n].id == ID_PZBGRUND) {
				var = zusi->map[n].var;
				break;
			}
		}
	}
	
	if (var == NULL)
		return (z3_not_mapped);

	switch (zusi->decode.path[PZB_SUBNODE]) {
	case 0x0003:
		switch (id) {
		case (ID_PZB_O):
			ret = z3_read_bytes(&zusi->recv, &var->lm_za_o, *len - 2);
			break;
		case (ID_PZB_M):
			ret = z3_read_bytes(&zusi->recv, &var->lm_za_m, *len - 2);
			break;
		case (ID_PZB_U):
			ret = z3_read_bytes(&zusi->recv, &var->lm_za_u, *len - 2);
			break;
		case (ID_PZB_LM1000):
			ret = z3_read_bytes(&zusi->recv, &var->lm_1000hz, *len - 2);
			break;
		case (ID_PZB_LM500):
			ret = z3_read_bytes(&zusi->recv, &var->lm_500hz, *len - 2);
			break;
		case (ID_PZB_LMB40):
			ret = z3_read_bytes(&zusi->recv, &var->lm_befehl, *len - 2);
			break;
		default:
			zusi->recv.pos += *len - 2;
			return (z3_ok);
		}
		if (ret == z3_ok)
			SETBIT(_pzb_flags, DATA_CHANGED_FLAG);
		else if (ret > z3_ok)
			return (ret);
		break;
	default:
		zusi->recv.pos += *len - 2;
	}

	
	return (z3_ok);
}

void z3_pzb_data_callback(zusi_data* zusi)
{
	if (GETBIT(_pzb_flags, DATA_CHANGED_FLAG)) {
		CLEARBIT(_pzb_flags, DATA_CHANGED_FLAG);
		zusi->data_callback(ZUSI_CAB_DATA, ID_PZBGRUND);
	}
}