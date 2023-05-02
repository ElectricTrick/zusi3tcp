#include "zusi3tcp.h"


z3_node* z3_alloc_node(zusi_data* zusi)
{
	for (byte n = 0; n <= MAX_NODES - 1; n++) {
		if (zusi->nodes[n].id == 0x0000)
			return (&zusi->nodes[n]);
	}

	return (NULL);
}

z3_return_code z3_new_node(zusi_data* zusi, word id, byte parent)
{
	z3_node* in_node = z3_alloc_node(zusi);
	if (in_node) {
		in_node->id = id;
		in_node->parent = parent;
		return (z3_ok);
	}

	return (z3_alloc_failed);
}

/// <summary>
/// Creates the internal memory structure for decode/encode
/// </summary>
/// <param name="zusi">- Pointer to zusi_data</param>
/// <param name="input_buffer">- Receive buffer size</param>
/// <param name="output_buffer">- Output buffer size</param>
/// <returns>z3_return_code</returns>
z3_return_code z3_init(zusi_data* zusi, dword memory_size)
{

	zusi->bytes_received = 0;

	//Allocate node buffer
	zusi->nodes = (z3_node*)calloc(MAX_NODES, sizeof(z3_node));
	zusi->attribs = (z3_attr*)calloc(MAX_ATTRIBS, sizeof(z3_attr));
	zusi->map = (z3_mapping*)calloc(MAX_NEEDED_DATA, sizeof(z3_mapping));

	RBUFMEM = (byte*)calloc(memory_size, sizeof(byte));
	zusi->recv.len = memory_size;
	RBUFPOS = 0;
	RBUFFIL = 0;

	zusi->decode.level = 0;
	zusi->decode.count = 0;
	memset(zusi->decode.path, 0, sizeof(zusi->decode.path));

	if (z3_new_node(zusi, 0xfff0, 0) != z3_ok)
		return (z3_alloc_failed);

	if (z3_new_node(zusi, 0xfff1, 0) != z3_ok)
		return (z3_alloc_failed);

	return (z3_ok);
}

/// <summary>
/// Frees decoder buffer memory and resets counters
/// </summary>
void z3_free(struct zusi_data* buf)
{
	;
}

/// <summary>
/// Put received bytes to decoder buffer
/// </summary>
/// <param name="zusi">- Pointer to zusi_data</param>
/// <param name="source">- Pointer to received data</param>
/// <param name="num_bytes">- Number of bytes to copy</param>
/// <returns>z3_return_code</returns>
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

/// <summary>
/// Shift read bytes in decoder buffer by (pos) left to free memory, resets (pos) to 0
/// </summary>
/// <param name="zusi">- Pointer to zusi_data</param>
/// <returns></returns>
z3_return_code z3_shift_bytes(zusi_data* zusi)
{
	if (zusi) {
		if (RBUFMEM) {
			if (RBUFPOS <= RBUFFIL) {
				if (memcpy((byte*)RBUFMEM, (byte*)RBUFMEM + RBUFPOS, RBUFFIL - RBUFPOS)) {
					RBUFFIL -= RBUFPOS;
					RBUFPOS = 0;
					return (z3_ok);
				}
				else
					return (z3_memcpy_failed);
			}
			else
				return (z3_mem_pos_wrong);
		}
	}

	return (z3_not_initialized);
}

/// <summary>
/// Copyies amount of bytes from decoder buffer to target variable, increases (pos)
/// </summary>
/// <param name="zusi">- Pointer to zusi_data</param>
/// <param name="target">- Pointer to target variable</param>
/// <param name="num_bytes">- Number of bytes to copy</param>
/// <returns>z3_return_code</returns>
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

/// <summary>
/// Checks current node path with array of node ids
/// </summary>
/// <param name="zusi">- Pointer to zusi_data</param>
/// <param name="ids">- Pointer to array</param>
/// <returns>z3_return_code (z3_ok on match)</returns>
z3_return_code z3_is_node_path(zusi_data* zusi, word* ids)
{
	if (memcmp(&zusi->decode.path, ids, 10 * sizeof(word)) == 0)
		return (z3_ok);

	return (z3_wrong_node_id);
}

/// <summary>
/// Reads ACK_HELLO node into server_info struct
/// </summary>
/// <param name="zusi">- Pointer to zusi_data</param>
/// <param name="id">- current attribute id</param>
/// <param name="len">- attribute lenght</param>
/// <returns>z3_return_code</returns>
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

/// <summary>
/// Beging new node.
/// Reads node ID, stores it to path and levels up.
/// </summary>
/// <param name="zusi">- Pointer to zusi_data</param>
/// <returns>z3_return_code</returns>
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

/// <summary>
/// End a node.
/// Clears current path and levels down.
/// On level == 0, a complete node was read an counter is increased by 1
/// </summary>
/// <param name="zusi">- Pointer to zusi_data</param>
/// <returns>z3_return_code</returns>
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

/// <summary>
/// Read a attribute.
/// Checks ID then reads content to desired target variable.
/// The target variable is choosen by current path and needed_data mappings.
/// Special (non float) variables and server commands are handles seperately.
/// </summary>
/// <param name="zusi">- Pointer to zusi_data</param>
/// <param name="len">- Pointer to lenght code</param>
/// <returns>z3_return_code</returns>
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

/// <summary>
/// Decodes buffered data until either buffer is empty or data could not be read.
/// </summary>
/// <param name="zusi">- Pointer to zusi_data</param>
/// <returns>z3_return_code</returns>
z3_return_code z3_decode(zusi_data* zusi)
{
	/*
	* 
	*/
	dword len = 0;
	z3_return_code ret;

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
			ret = z3_shift_bytes(zusi);
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