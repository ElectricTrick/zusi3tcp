#pragma once

#ifndef ZUSI3TCP

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/* Defines and macros */

#define MAX_NEEDED_DATA				12

#define NODE_START					0x00000000
#define NODE_END					0xFFFFFFFF

#define HIGHEST_NODE				0x010C
#define HIGHEST_ID					0x003B

#define ID_ZUSIVER					0x0001
#define ID_ZUSIINFO					0x0002
#define ID_HELLOACK					0x0003
#define ID_HELLOTIME				0x0004
#define ID_PROTOVER					0x0005

// Node path defines
#define PATH_ACK_HELLO				0x0001, 0x0002, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000
#define PATH_ACK_NEEDED_DATA		0x0002, 0x0004, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000
#define PATH_DATA_FTD				0x0002, 0x000A, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000

#define ZUSI_CAB_DATA				0

/* Type definitions and structs */

typedef unsigned char	byte;
typedef unsigned short	word;
typedef unsigned long	dword;
typedef word			z3_return_code;

enum z3_return_code {
	z3_ok,
	z3_not_initialized,
	z3_alloc_failed,
	z3_bytes_not_available,
	z3_out_of_memory,
	z3_memcpy_failed,
	z3_mem_pos_wrong,
	z3_add_failed,
	z3_read_failed,
	z3_wrong_node_id,
	z3_wrong_attr_id,
	z3_level_below_0,
	z3_buffer_not_empty,
};

typedef struct {
	byte* ptr;	//Pointer to memory
	dword len;	//Buffer lenght
	dword fil;	//Bytes filled
	dword pos;	//Reading position
} z3_buffer;

typedef struct {
	word key;
	word id;
	void* var;
} z3_mapping;

typedef struct {
	word path[10];
	byte level;
	dword count;
} z3_decoder;

typedef struct {
	char* zusi_version;
	char* zusi_info;
	char* protocol_version;
	byte connected;
	double timetable;
} server_info;

typedef struct {
	word protocol_version;
	word client_type;
	char* client_name;
	char* client_version;
} client_info;

typedef struct {
	byte* temp;
	z3_buffer recv;
	z3_buffer send;
	z3_mapping map[MAX_NEEDED_DATA];
	z3_decoder decode;
	client_info client;
	server_info server;
	dword bytes_received;
} zusi_data;


/* Function declarations */

/// <summary>
/// Creates the internal memory structure for decode/encode
/// </summary>
/// <param name="zusi">- Pointer to zusi_data</param>
/// <param name="input_buffer">- Receive buffer size</param>
/// <param name="output_buffer">- Output buffer size</param>
/// <returns>z3_return_code</returns>
z3_return_code z3_init(zusi_data* zusi, dword memory_size);

/// <summary>
/// Put received bytes to decoder buffer
/// </summary>
/// <param name="zusi">- Pointer to zusi_data</param>
/// <param name="source">- Pointer to received data</param>
/// <param name="num_bytes">- Number of bytes to copy</param>
/// <returns>z3_return_code</returns>
z3_return_code z3_put_bytes(zusi_data* zusi, byte* source, word num_bytes);

/// <summary>
/// Shift read bytes in decoder buffer by (pos) left to free memory, resets (pos) to 0
/// </summary>
/// <param name="zusi">- Pointer to zusi_data</param>
/// <returns></returns>
//z3_return_code z3_shift_bytes(zusi_data* zusi)
z3_return_code z3_shift_bytes(zusi_data* zusi);

/// <summary>
/// Copyies amount of bytes from decoder buffer to target variable, increases (pos)
/// </summary>
/// <param name="zusi">- Pointer to zusi_data</param>
/// <param name="target">- Pointer to target variable</param>
/// <param name="num_bytes">- Number of bytes to copy</param>
/// <returns>z3_return_code</returns>
z3_return_code z3_read_bytes(z3_buffer* buf, void* target, word num_bytes);

/// <summary>
/// Checks current node path with array of node ids
/// </summary>
/// <param name="zusi">- Pointer to zusi_data</param>
/// <param name="ids">- Pointer to array</param>
/// <returns>z3_return_code (z3_ok on match)</returns>
z3_return_code z3_is_node_path(zusi_data* zusi, word* ids);

/// <summary>
/// Reads ACK_HELLO node into server_info struct
/// </summary>
/// <param name="zusi">- Pointer to zusi_data</param>
/// <param name="id">- current attribute id</param>
/// <param name="len">- attribute lenght</param>
/// <returns>z3_return_code</returns>
z3_return_code z3_ack_hello(zusi_data* zusi, word id, dword* len);

/// <summary>
/// Beging new node.
/// Reads node ID, stores it to path and levels up.
/// </summary>
/// <param name="zusi">- Pointer to zusi_data</param>
/// <returns>z3_return_code</returns>
z3_return_code z3_begin_node(zusi_data* zusi);

/// <summary>
/// End a node.
/// Clears current path and levels down.
/// On level == 0, a complete node was read an counter is increased by 1
/// </summary>
/// <param name="zusi">- Pointer to zusi_data</param>
/// <returns>z3_return_code</returns>
z3_return_code z3_end_node(zusi_data* zusi);

/// <summary>
/// Read a attribute.
/// Checks ID then reads content to desired target variable.
/// The target variable is choosen by current path and needed_data mappings.
/// Special (non float) variables and server commands are handles seperately.
/// </summary>
/// <param name="zusi">- Pointer to zusi_data</param>
/// <param name="len">- Pointer to lenght code</param>
/// <returns>z3_return_code</returns>
z3_return_code z3_read_attribute(zusi_data* zusi, dword* len);

/// <summary>
/// Decodes buffered data until either buffer is empty or data could not be read.
/// </summary>
/// <param name="zusi">- Pointer to zusi_data</param>
/// <returns>z3_return_code</returns>
z3_return_code z3_decode(zusi_data* zusi);

/// <summary>
/// Encodes node footer or header to send buffer for transport.
/// </summary>
/// <param name="zusi">- Pointer to zusi_data</param>
/// <param name="node_id">- Node ID</param>
/// <returns>z3_return_code</returns>
z3_return_code z3_write_node(zusi_data* zusi, word node_id);

/// <summary>
/// Encodes attribute header and data to send buffer for transport.
/// </summary>
/// <param name="zusi">- Pointer to zusi_data</param>
/// <param name="attr_id">- Attribute ID</param>
/// <param name="data">- Pointer to payload data</param>
/// <param name="data_len">- lenght of payload data</param>
/// <returns>z3_return_code</returns>
z3_return_code z3_write_attribute(zusi_data* zusi, word attr_id, void* data, dword data_len);

/// <summary>
/// Shifts the bytes which has been transmitted and returns bytes left in send buffer.
/// num_bytes == 0 just returns bytes left.
/// </summary>
/// <param name="zusi">- Pointer to zusi_data</param>
/// <param name="num_bytes">- Number of bytes sent</param>
/// <returns>Numbers of bytes left in buffer</returns>
dword z3_bytes_sent(zusi_data* zusi, dword num_bytes);

/// <summary>
/// Returns pointer to encoded bytes if send buffer is filled.
/// </summary>
/// <param name="zusi">- Pointer to zusi_data</param>
/// <returns>Pointer to bytes</returns>
byte* z3_get_send_buffer(zusi_data* zusi);

z3_return_code z3_hello_msg(zusi_data* zusi, const byte client_type, const char* client_name, const char* client_version);

z3_return_code z3_add_needed_data(zusi_data* zusi, word key, word id, void* target);

#endif // !ZUSI3TCP
