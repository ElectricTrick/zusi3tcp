#pragma once

#ifndef ZUSI3TCP

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

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
	z3_level_below_0
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
	word id;
	byte parent;
} z3_node;

typedef struct {
	word id;
	byte parent;
	word len;
	void* data;
} z3_attr;

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
	z3_mapping* map;
	z3_node* nodes;
	z3_attr* attribs;
	z3_decoder decode;
	client_info client;
	server_info server;
	dword bytes_received;
} zusi_data;

/* Defines and macros */

#define RBUFMEM		zusi->recv.ptr
#define RBUFLEN		zusi->recv.len
#define RBUFPOS		zusi->recv.pos
#define RBUFFIL		zusi->recv.fil

#define MAX_NEEDED_DATA				12
#define MAX_NODES					20
#define MAX_ATTRIBS					30

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
#define NODE_ACK_HELLO				0x0001, 0x0002, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000
#define NODE_ACK_NEEDED_DATA		0x0002, 0x0004, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000
#define NODE_DATA_FTD				0x0002, 0x000A, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000

/* Function declarations */
z3_return_code z3_init(zusi_data* zusi, dword memory_size);
z3_return_code z3_put_bytes(zusi_data* zusi, byte* source, word num_bytes);
z3_return_code z3_shift_bytes(zusi_data* zusi);
z3_return_code z3_read_bytes(zusi_data* zusi, void* target, word num_bytes);
z3_return_code z3_is_node_path(zusi_data* zusi, word* ids);
z3_return_code z3_ack_hello(zusi_data* zusi, word id, dword* len);
z3_return_code z3_begin_node(zusi_data* zusi);
z3_return_code z3_end_node(zusi_data* zusi);
z3_return_code z3_read_attribute(zusi_data* zusi, dword* len);
z3_return_code z3_decode(zusi_data* zusi);

#endif // !ZUSI3TCP
