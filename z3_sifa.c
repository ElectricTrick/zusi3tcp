#include "z3_sifa.h"

byte _sifa_flags = 0;

z3_return_code z3_sifa_data(zusi_data* zusi, word id, dword* len)
{
	z3_return_code ret = z3_ok;

	static zusi_sifa_data* var = NULL;
	if (var == NULL) {
		for (byte n = 0; n < MAX_NEEDED_DATA; n++) {
			if (zusi->map[n].key == ZUSI_CAB_DATA && zusi->map[n].id == ID_SIFA_GRUND) {
				var = (zusi_sifa_data *) zusi->map[n].var;
				break;
			}
		}
	}

	if (var == NULL)
		return (z3_not_mapped);

	switch (id) {
	case (ID_SIFA_MELDER):
		ret = z3_read_bytes(&zusi->recv, &var->lm_sifa, *len - 2);
		break;
	case (ID_SIFA_HUPE):
		ret = z3_read_bytes(&zusi->recv, &var->hupe, *len - 2);
		break;
	default:
		zusi->recv.pos += *len - 2;
		return (z3_ok);
	}

	if (ret == z3_ok)
		SETBIT(_sifa_flags, DATA_CHANGED_FLAG);
	else if (ret > z3_ok)
		return (ret);


	return (z3_ok);
}

void z3_sifa_callback(zusi_data* zusi)
{
	if (GETBIT(_sifa_flags, DATA_CHANGED_FLAG)) {
		CLEARBIT(_sifa_flags, DATA_CHANGED_FLAG);
		zusi->data_callback(ZUSI_CAB_DATA, ID_SIFA_GRUND);
	}
}