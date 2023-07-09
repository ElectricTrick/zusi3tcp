#include "zusi3notbremse.h"

byte _nb_flags = 0;

z3_return_code z3_notbrems_data(zusi_data* zusi, word id, dword* len)
{
	z3_return_code ret = z3_ok;

	static zusi_notbrems_data* var = NULL;
	if (var == NULL) {
		for (byte n = 0; n < MAX_NEEDED_DATA; n++) {
			if (zusi->map[n].key == ZUSI_CAB_DATA && zusi->map[n].id == ID_NB_GRUND) {
				var = (zusi_notbrems_data*)zusi->map[n].var;
				break;
			}
		}
	}

	if (var == NULL)
		return (z3_not_mapped);

	switch (id) {
	case (ID_NB_STATUS):
		ret = z3_read_bytes(&zusi->recv, &var->status, *len - 2);
		break;
	case (ID_NB_LM_SYS):
		ret = z3_read_bytes(&zusi->recv, &var->lm_nb_sys, *len - 2);
		break;
	case (ID_NB_LM_AKT):
		ret = z3_read_bytes(&zusi->recv, &var->lb_nb_akt, *len - 2);
		break;
	default:
		zusi->recv.pos += *len - 2;
		return (z3_ok);
	}

	if (ret == z3_ok)
		SETBIT(_nb_flags, DATA_CHANGED_FLAG);
	else if (ret > z3_ok)
		return (ret);


	return (z3_ok);
}

void z3_notbrems_callback(zusi_data* zusi)
{
	if (GETBIT(_nb_flags, DATA_CHANGED_FLAG)) {
		CLEARBIT(_nb_flags, DATA_CHANGED_FLAG);
		zusi->data_callback(ZUSI_CAB_DATA, ID_NB_GRUND);
	}
}