#include "z3_tueren.h"


byte _tueren_flags = 0;

z3_return_code z3_tueren_data(zusi_data* zusi, word id, word* len)
{
	z3_return_code ret = z3_ok;

	static zusi_tueren_data* var = NULL;
	if (var == NULL) {
		for (byte n = 0; n < MAX_NEEDED_DATA; n++) {
			if (zusi->map[n].key == ZUSI_CAB_DATA && zusi->map[n].id == ID_TUEREN_GRUND) {
				var = zusi->map[n].var;
				break;
			}
		}
	}

	if (var == NULL)
		return (z3_not_mapped);

	switch (id) {
	case (ID_TUEREN_FL):
		ret = z3_read_bytes(&zusi->recv, &var->lm_fl, *len - 2);
		break;
	case (ID_TUEREN_FR):
		ret = z3_read_bytes(&zusi->recv, &var->lm_fr, *len - 2);
		break;
	case (ID_TUEREN_FLR):
		ret = z3_read_bytes(&zusi->recv, &var->lm_flr, *len - 2);
		break;
	case (ID_TUEREN_ZS):
		ret = z3_read_bytes(&zusi->recv, &var->lm_zs, *len - 2);
		break;
	case (ID_TUEREN_ZFL):
		ret = z3_read_bytes(&zusi->recv, &var->lm_zfl, *len - 2);
		break;
	case (ID_TUEREN_ZFR):
		ret = z3_read_bytes(&zusi->recv, &var->lm_zfr, *len - 2);
		break;
	case (ID_TUEREN_TAV):
		ret = z3_read_bytes(&zusi->recv, &var->lm_tav, *len - 2);
		break;
	default:
		zusi->recv.pos += *len - 2;
		return (z3_ok);
	}

	if (ret == z3_ok)
		SETBIT(_tueren_flags, DATA_CHANGED_FLAG);
	else if (ret > z3_ok)
		return (ret);


	return (z3_ok);
}

void z3_tueren_callback(zusi_data* zusi)
{
	if (GETBIT(_tueren_flags, DATA_CHANGED_FLAG)) {
		CLEARBIT(_tueren_flags, DATA_CHANGED_FLAG);
		zusi->data_callback(ZUSI_CAB_DATA, ID_TUEREN_GRUND);
	}
}