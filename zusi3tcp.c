#include "zusi3tcp.h"

#include "z3_pzblzb.h"
#include "z3_tueren.h"
#include "z3_sifa.h"
#include "z3_notbrems.h"


#define RBUFMEM		zusi->recv.ptr
#define RBUFLEN		zusi->recv.len
#define RBUFPOS		zusi->recv.pos
#define RBUFFIL		zusi->recv.fil

word proto_ver = 02;

z3_return_code z3_init(zusi_data* zusi, word in_buf_size, word out_buf_size, z3_data_notify data_callback)
{

	zusi->bytes_received = 0;

	for (byte n = 0; n < MAX_NEEDED_DATA; n++)
		zusi->map[n].id = 0;

	zusi->recv.ptr = (byte*)calloc(in_buf_size, sizeof(byte));
	zusi->recv.len = in_buf_size;
	zusi->recv.pos = 0;
	zusi->recv.fil = 0;

	zusi->send.ptr = (byte*)calloc(out_buf_size, sizeof(byte));
	zusi->send.len = out_buf_size;
	zusi->send.pos = 0;
	zusi->send.fil = 0;

	zusi->decode.level = 0;
	zusi->decode.count = 0;
	memset(zusi->decode.path, 0, sizeof(zusi->decode.path));

	zusi->data_callback = data_callback;

	zusi->status = z3_not_connected;

	return (z3_ok);
}

z3_return_code z3_put_bytes(zusi_data* zusi, byte* source, word num_bytes)
{
	if (zusi) {
		if (RBUFMEM) {
			if (RBUFFIL + num_bytes <= zusi->recv.len) {
				if (memcpy((byte*)RBUFMEM + RBUFFIL, source, num_bytes)) {
					RBUFFIL += num_bytes;
					zusi->bytes_received += num_bytes;
					return (z3_ok);
				}
				else
					return (z3_memcpy_failed);
			}
			else
				return (z3_out_of_memory);
		}
	}

	return(z3_not_initialized);
}


z3_return_code z3_shift_bytes(z3_buffer* buf)
{
	if (buf) {
		if (buf->pos <= buf->fil) {
			if (memcpy((byte*)buf->ptr, (byte*)buf->ptr + buf->pos, buf->fil - buf->pos)) {
				buf->fil -= buf->pos;
				buf->pos = 0;
				return (z3_ok);
			}
			else
				return (z3_memcpy_failed);
		}
		else
			return (z3_mem_pos_wrong);
	}

	return (z3_not_initialized);
}


z3_return_code z3_read_bytes(z3_buffer* buf, void* target, word num_bytes)
{
	if (buf) {
		if (buf->pos + num_bytes <= buf->fil) {
			if (memcmp(target, (byte*)buf->ptr + buf->pos, num_bytes) != 0) {
				if (memcpy(target, (byte*)buf->ptr + buf->pos, num_bytes)) {
					buf->pos += num_bytes;
					return (z3_ok);
				}
				else
					return (z3_memcpy_failed);
			}
			else {
				buf->pos += num_bytes;
				return (z3_nop);
			}
		}
		else
			return (z3_bytes_not_available);
	}

	return (z3_not_initialized);
}

z3_return_code z3_write_bytes(zusi_data* zusi, void* source, word num_bytes)
{
	if (zusi) {
		if (zusi->send.ptr) {
			if (zusi->send.fil + num_bytes <= zusi->send.len) {
				if (memcpy(zusi->send.ptr + zusi->send.fil, (byte*)source, num_bytes)) {
					zusi->send.fil += num_bytes;
					return (z3_ok);
				}
				else
					return (z3_memcpy_failed);
			}
			else
				return (z3_bytes_not_available);
		}
	}

	return (z3_not_initialized);
}


z3_return_code z3_is_node_path(zusi_data* zusi, word* ids)
{
	for (byte n = 0; n < 10; n++) {
		if (ids[n] == 0)
			break;
		if (zusi->decode.path[n] != ids[n])
			return (z3_wrong_node_id);
	}

	return (z3_ok);
}


z3_return_code z3_ack_hello(zusi_data* zusi, word id, word* len)
{

	switch (id) {
	case ID_ZUSIVER:
		zusi->server.zusi_version = (char*)malloc(*len - 2);
		return (z3_read_bytes(&zusi->recv, zusi->server.zusi_version, *len - 2));
	case ID_ZUSIINFO:
		zusi->server.zusi_info = (char*)malloc(*len - 2);
		return (z3_read_bytes(&zusi->recv, zusi->server.zusi_info, *len - 2));
	case ID_HELLOACK:
		zusi->status = z3_ack_hello_ok;
		return (z3_read_bytes(&zusi->recv, &zusi->server.connected, *len - 2));
	default:
		zusi->recv.pos += *len - 2;
	}

	return (z3_ok);
}

z3_return_code z3_cab_data(zusi_data* zusi, word id, word* len)
{
	z3_return_code ret;
	for (byte n = 0; n < MAX_NEEDED_DATA; n++) {
		if (zusi->map[n].key == ZUSI_CAB_DATA && zusi->map[n].id == id) {
			zusi->status = z3_online;
			switch (id) {
			default:
				ret = z3_read_bytes(&zusi->recv, zusi->map[n].var, *len - 2);
				if (ret <= z3_ok)
					zusi->data_callback(ZUSI_CAB_DATA, id);
				return (ret);
			}

		}
		else if (zusi->map[n].id == 0)
		{
			zusi->recv.pos += *len - 2;
			break;
		}
	}

	return (z3_ok);
}

z3_return_code z3_begin_node(zusi_data* zusi)
{
	//Knoten ID lesen
	word id;
	z3_return_code ret = z3_read_bytes(&zusi->recv, &id, sizeof(id));
	if (ret != z3_ok)
		return (ret);

	//Im erlaubten Bereich?
	if (id > HIGHEST_NODE)
		return (z3_wrong_node_id);

	//Level up und id an Pfad anh�ngen
	zusi->decode.path[zusi->decode.level] = id;
	zusi->decode.level += 1;

	return (z3_ok);

}


z3_return_code z3_end_node(zusi_data* zusi)
{

//#ifdef MOD_TUEREN
	//Callback f�r ge�nderte PZB Daten
	if (z3_is_node_path(zusi, (word[]) { PATH_TUEREN_DATA }) <= z3_ok)
		z3_tueren_callback(zusi);
//#endif
//#ifdef MOD_PZBLZB
	//Callback f�r ge�nderte PZB Daten
	if (z3_is_node_path(zusi, (word[]) { PATH_PZB_DATA }) <= z3_ok)
		z3_pzb_data_callback(zusi);
//#endif
//#ifdef MOD_SIFA
	//Callback f�r ge�nderte PZB Daten
	if (z3_is_node_path(zusi, (word[]) { PATH_SIFA_DATA }) <= z3_ok)
		z3_sifa_callback(zusi);
//endif
//#ifdef MOD_SIFA
	//Callback f�r ge�nderte PZB Daten
	if (z3_is_node_path(zusi, (word[]) { PATH_NOTBREMS_DATA }) <= z3_ok)
		z3_sifa_callback(zusi);
//#endif

	if (zusi->decode.level > 0)
		zusi->decode.level -= 1;
	else
		return (z3_level_below_0);

	zusi->decode.path[zusi->decode.level] = 0;

	if (zusi->decode.level == 0)
		zusi->decode.count += 1;

	return (z3_ok);
}


z3_return_code z3_read_attribute(zusi_data* zusi, word* len)
{
	//Pr�fen ob genug Bytes im Puffer sind
	if (zusi->recv.pos + *len >= zusi->recv.fil)
		return (z3_bytes_not_available);

	//Attribut ID lesen
	word id;
	z3_return_code ret = z3_read_bytes(&zusi->recv, &id, sizeof(id));
	if (ret > z3_ok)
		return (ret);

	//Im erlaubten Bereich?
	if (id > HIGHEST_ID)
		return (z3_wrong_node_id);

	//Schauen in welchem Knoten wir uns befinden
//#ifdef MOD_TUEREN
	if (z3_is_node_path(zusi, (word[]) { PATH_TUEREN_DATA }) == z3_ok)
		return (z3_tueren_data(zusi, id, len));
//#endif 
//#ifdef MOD_PZBLZB
	if (z3_is_node_path(zusi, (word[]) { PATH_PZB_DATA }) == z3_ok)
		return (z3_pzb_data(zusi, id, len));
//#endif
//#ifdef MOD_SIFA
	if (z3_is_node_path(zusi, (word[]) { PATH_SIFA_DATA }) == z3_ok)
		return (z3_sifa_data(zusi, id, len));
//#endif
	if (z3_is_node_path(zusi, (word[]) { PATH_NOTBREMS_DATA }) == z3_ok)
		return (z3_notbrems_data(zusi, id, len));


	if (z3_is_node_path(zusi, (word[]) { PATH_DATA_FTD }) == z3_ok)
		return (z3_cab_data(zusi, id, len));
	if (z3_is_node_path(zusi, (word[]) { PATH_ACK_HELLO }) == z3_ok)
		return (z3_ack_hello(zusi, id, len));
	if (z3_is_node_path(zusi, (word[]) { PATH_ACK_NEEDED_DATA }) == z3_ok)
		zusi->status = z3_ack_needed_data_ok;

	zusi->recv.pos += *len - 2;

	return (z3_ok);
}


z3_return_code z3_decode(zusi_data* zusi, word recv_bytes)
{
	/*
	* Cyclic call this method. It decodes available data and empties the buffer.
	* Use z3_put_bytes to fill buffer with new incoming data. z3_decode will resume on fragemented packets,
	* this allow to work with tcp packet segmentation due to very limited SRAM on MCUs.
	*/
	dword len = 0;
	z3_return_code ret = z3_ok;

	zusi->recv.fil += recv_bytes;

	//Schleife solange Daten im Puffer sind
	while (RBUFFIL > 0) {
		//L�ngencode lesen
		ret = z3_read_bytes(&zusi->recv, &len, sizeof(len));
		if (ret <= z3_ok) {
			switch (len) {
			case NODE_START:
				//Neuer Knoten f�ngt an
				ret = z3_begin_node(zusi);
				break;
			case NODE_END:
				//Knoten endet
				ret = z3_end_node(zusi);
				break;
			default:
				//Attribut lesen
				ret = z3_read_attribute(zusi, (word*)&len);
			}
		}
		if (ret <= z3_ok) {
			//Wenn ok, Bytes nach links shiften um Platz zu machen
			ret = z3_shift_bytes(&zusi->recv);
			if (ret > z3_ok)
				break;
		}
		else {
			//Ansonsten Position zur�cksetzen und Schleife verlassen
			RBUFPOS = 0;
			break;
		}
	}

	return (ret);
}


z3_return_code z3_write_node(zusi_data* zusi, word node_id)
{
	dword code = NODE_START;
	if (node_id == 0)
		code = NODE_END;

	z3_return_code ret = z3_write_bytes(zusi, &code, sizeof(dword));

	if (ret <= z3_ok && node_id > 0)
		ret = z3_write_bytes(zusi, &node_id, sizeof(word));

	return (ret);
}


z3_return_code z3_write_attribute(zusi_data* zusi, word attr_id, void* data, const dword data_len)
{
	dword code = data_len + 2;
	z3_return_code ret = z3_write_bytes(zusi, &code, sizeof(dword));
	if (ret <= z3_ok) {
		ret = z3_write_bytes(zusi, &attr_id, sizeof(word));
		if (ret <= z3_ok) {
			ret = z3_write_bytes(zusi, data, data_len);
		}
	}

	return (ret);
}


dword z3_bytes_sent(zusi_data* zusi, dword num_bytes)
{
	if (zusi) {
		if (num_bytes > 0) {
			zusi->send.pos = num_bytes;
			z3_shift_bytes(&zusi->send);
		}
		return (zusi->send.fil);
	}

	return (0);
}


byte* z3_get_send_buffer(zusi_data* zusi)
{
	if (zusi) {
		if (zusi->send.ptr) {
			if (zusi->send.fil > 0) {
				return (zusi->send.ptr);
			}
		}
	}

	return (NULL);
}

word z3_buffer_avail(zusi_data* zusi, byte direction)
{
	if (zusi) {
		if (direction == ZUSI_RECV_BUF)
			return (zusi->recv.len - zusi->recv.fil);
		else if (direction == ZUSI_SEND_BUF)
			return (zusi->send.len - zusi->send.fil);
	}

	return (0);
}

byte* z3_get_buffer(zusi_data* zusi, byte direction)
{
	if (zusi) {
		if (direction == ZUSI_RECV_BUF)
			return (zusi->recv.ptr + zusi->recv.fil);
		else if (direction == ZUSI_SEND_BUF)
			return (zusi->send.ptr + zusi->send.fil);
	}

	return (NULL);
}

z3_return_code zusi_hello_msg(zusi_data* zusi, word client_type, const char* client_name, const char* client_version)
{

	if (z3_write_node(zusi, 0x0001) <= z3_ok)
	 if (z3_write_node(zusi, 0x0001) <= z3_ok)
	  if (z3_write_attribute(zusi, 0x0001, &proto_ver, sizeof(word)) <= z3_ok)
	   if (z3_write_attribute(zusi, 0x0002, &client_type, sizeof(word)) <= z3_ok)
	    if (z3_write_attribute(zusi, 0x0003, client_name, strlen(client_name)) <= z3_ok)
	     if (z3_write_attribute(zusi, 0x0004, client_version, strlen(client_version)) <= z3_ok)
	      if (z3_write_node(zusi, 0) <= z3_ok)
	       if (z3_write_node(zusi, 0) <= z3_ok)
	        return (z3_ok);

	return (z3_memcpy_failed);
}

z3_return_code zusi_add_needed_data(zusi_data* zusi, word key, word id, void* target)
{
	for (byte n = 0; n <= MAX_NEEDED_DATA; n++) {
		if (zusi->map[n].id == 0) {
			zusi->map[n].id = id;
			zusi->map[n].key = key;
			zusi->map[n].var = target;
			return (z3_ok);
		}
	}

	return (z3_add_failed);
}

z3_return_code zusi_needed_data_msg(zusi_data* zusi)
{

	z3_return_code ret = z3_ok;

	if (z3_write_node(zusi, 0x0002) <= z3_ok)
		if (z3_write_node(zusi, 0x0003) <= z3_ok)
			if (z3_write_node(zusi, 0x000a) <= z3_ok) {
				for (byte n = 0; n < MAX_NEEDED_DATA; n++) {
					if (zusi->map[n].id > 0) {
						ret = z3_write_attribute(zusi, ZUSI_CAB_DATA, &zusi->map[n].id, sizeof(word));
						if (ret != z3_ok)
							return (ret);
					}
					else
						break;
				}
				if (z3_write_node(zusi, 0) <= z3_ok)
					if (z3_write_node(zusi, 0) <= z3_ok)
						if (z3_write_node(zusi, 0) <= z3_ok)
							return (z3_ok);
			}

	return (z3_memcpy_failed);
}