#ifndef NANACBUS_H_
#define NANACBUS_H_

#include <stdint.h>

struct nanac_cpu;
typedef struct nanac_cpu nanac_cpu_t;

typedef int (*nanac_op_f)( nanac_cpu_t *cpu, uint8_t arga, uint8_t argb );


typedef struct {
	uint8_t mod;
	uint8_t cmd;
	uint8_t arga;
	uint8_t argb;
} nanac_op_t;


typedef struct {
	const char *name;
	nanac_op_f run;
} nanac_cmd_t;

typedef nanac_cmd_t *nanac_cmd_p;


typedef struct {
	const char *name;
	uint8_t cmds_len;
	const nanac_cmd_t *cmds;
} nanac_mod_t;


struct nanac_cpu {
	nanac_op_t *ops;
	uint16_t ops_sz;
	uint16_t eip;
	uint16_t jip;
	int do_jump;
	void *regs[0xFF];
	nanac_mod_t mods[0xFF];
	uint8_t mods_cnt;
};

static inline uint16_t nanac_uint16_t( uint8_t arga, uint8_t argb ) {
	return arga | (argb<<8);
}

void nanac_init( nanac_cpu_t *cpu );

int nanac_addmod( nanac_cpu_t *cpu, const char *name, uint8_t cmds_len, const nanac_cmd_t cmds[] );

const nanac_op_t *nanac_op( const nanac_cpu_t *cpu, const uint16_t eip );

int nanac_step( nanac_cpu_t *cpu, const nanac_op_t *op );

int nanac_run( nanac_cpu_t *cpu );

const uint8_t *nanac_bytes( const nanac_cpu_t *cpu, uint32_t offset, uint32_t length );


#endif
