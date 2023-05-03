#include "zusi3tcp.h"

#define RBUFMEM		zusi->recv.ptr
#define RBUFLEN		zusi->recv.len
#define RBUFPOS		zusi->recv.pos
#define RBUFFIL		zusi->recv.fil



z3_return_code z3_init(zusi_data* zusi, dword memory_size)
{

	dword recv_buf_len = memory_size * 0.75;
	dword send_buf_len = memory_size - recv_buf_len;

	zusi->bytes_received = 0;

	zusi->map = (z3_mapping*)calloc(MAX_NEEDED_DATA, sizeof(z3_mapping));

	RBUFMEM = (byte*)calloc(recv_buf_len, sizeof(byte));
	RBUFLEN = recv_buf_len;
	RBUFPOS = 0;
	RBUFFIL = 0;

	zusi->send.ptr = (byte*)calloc(send_buf_len, sizeof(byte));
	zusi->send.len = send_buf_len;
	zusi->send.pos = 0;
	zusi->send.fil = 0;

	zusi->decode.level = 0;
	zusi->decode.count = 0;
	memset(zusi->decode.path, 0, sizeof(zusi->decode.path));

	return (z3_ok);
}

/* TODO */
/// <summary>
/// Frees decoder buffer memory and resets counters
/// </summary>
void z3_free(struct zusi_data* buf)
{
	;
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


z3_return_code z3_read_bytes(zusi_data* zusi, void* target, word num_bytes)
{
	if (zusi) {
		if (RBUFMEM) {
			if (RBUFPOS + num_bytes <= RBUFFIL) {
				if (memcpy(target, (byte*)RBUFMEM + RBUFPOS, num_bytes)) {
					RBUFPOS += num_bytes;
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
	if (memcmp(&zusi->decode.path, ids, 10 * sizeof(word)) == 0)
		return (z3_ok);

	return (z3_wrong_node_id);
}


z3_return_code z3_ack_hello(zusi_data* zusi, word id, dword* len)
{

	switch (id) {
	case ID_ZUSIVER:
		zusi->server.zusi_version = (char*)malloc(*len);
		return (z3_read_bytes(zusi, zusi->server.zusi_version, *len - 2));
	case ID_ZUSIINFO:
		zusi->server.zusi_info = (char*)malloc(*len);
		return (z3_read_bytes(zusi, zusi->server.zusi_info, *len - 2));
	case ID_HELLOACK:
		return (z3_read_bytes(zusi, &zusi->server.connected, *len - 2));
	}
}


z3_return_code z3_begin_node(zusi_data* zusi)
{
	//Knoten ID lesen
	word id;
	z3_return_code ret = z3_read_bytes(zusi, &id, sizeof(id));
	if (ret != z3_ok)
		return (ret);

	//Im erlaubten Bereich?
	if (id > HIGHEST_NODE)
		return (z3_wrong_node_id);

	//Level up und id an Pfad anhängen
	zusi->decode.path[zusi->decode.level] = id;
	zusi->decode.level += 1;

	return (z3_ok);

}


z3_return_code z3_end_node(zusi_data* zusi)
{
	if (zusi->decode.level > 0)
		zusi->decode.level -= 1;
	else
		return (z3_level_below_0);

	zusi->decode.path[zusi->decode.level] = 0;

	if (zusi->decode.level == 0)
		zusi->decode.count += 1;

	return (z3_ok);
}


z3_return_code z3_read_attribute(zusi_data* zusi, dword* len)
{
	//Attribut ID lesen
	word id;
	z3_return_code ret = z3_read_bytes(zusi, &id, sizeof(id));
	if (ret != z3_ok)
		return (ret);

	//Im erlaubten Bereich?
	if (id > HIGHEST_ID)
		return (z3_wrong_node_id);

	//Schauen in welchem Knoten wir uns befinden
	word node[] = { NODE_ACK_HELLO };
	if (z3_is_node_path(zusi, &node) == z3_ok)
		return (z3_ack_hello(zusi, id, len));



	return (z3_ok);
}


z3_return_code z3_decode(zusi_data* zusi)
{
	/*
	* Cyclic call this method. It decodes available data and empties the buffer.
	* Use z3_put_bytes to fill buffer with new incoming data. z3_decode will resume on fragemented packets,
	* this allow to work with tcp packet segmentation due to very limited SRAM on MCUs.
	*/
	dword len = 0;
	z3_return_code ret = z3_ok;

	//Schleife solange Daten im Puffer sind
	while (RBUFFIL > 0) {
		//Längencode lesen
		ret = z3_read_bytes(zusi, &len, sizeof(len));
		if (ret == z3_ok) {
			switch (len) {
			case NODE_START:
				//Neuer Knoten fängt an
				ret = z3_begin_node(zusi);
				break;
			case NODE_END:
				//Knoten endet
				ret = z3_end_node(zusi);
				break;
			default:
				//Attribut lesen
				ret = z3_read_attribute(zusi, &len);
			}
		}
		if (ret == z3_ok) {
			//Wenn ok, Bytes nach links shiften um Platz zu machen
			ret = z3_shift_bytes(&zusi->recv);
			if (ret != z3_ok)
				break;
		}
		else {
			//Ansonsten Position zurücksetzen und Schleife verlassen
			RBUFPOS = 0;
			break;
		}
	}

	printf("Decode return code %d\r\n", ret);
	return (ret);
}


z3_return_code z3_write_node(zusi_data* zusi, word node_id)
{
	dword code = NODE_START;
	if (node_id == 0)
		code = NODE_END;

	z3_return_code ret = z3_write_bytes(zusi, &code, sizeof(dword));

	if (ret == z3_ok && node_id > 0)
		ret = z3_write_bytes(zusi, &node_id, sizeof(word));

	return (ret);
}


z3_return_code z3_write_attribute(zusi_data* zusi, word attr_id, void* data, const dword data_len)
{
	dword code = data_len + 2;
	z3_return_code ret = z3_write_bytes(zusi, &code, sizeof(dword));
	if (ret == z3_ok) {
		ret = z3_write_bytes(zusi, &attr_id, sizeof(word));
		if (ret == z3_ok) {
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