#ifndef NANACBUS_H_
#define NANACBUS_H_

#include <stdint.h>


#define NANAC_OK 0
#define NANAC_NO_EPILOGUE 1
#define NANAC_ERROR_DIE (-1)
#define NANAC_ERROR_REGWIN_OVERFLOW (-101)
#define NANAC_ERROR_REGWIN_UNDERFLOW (-102)
#define NANAC_ERROR_STEP_NOMOD (-201)
#define NANAC_ERROR_STEP_NOCMD (-202)
#define NANAC_ERROR_STEP_BADOP (-203)
#define NANAC_ERROR_SUBRET_OVERFLOW (-301)
#define NANAC_ERROR_SUBRET_UNDERFLOW (-302)


struct nanac_s;
typedef struct nanac_s nanac_t;

typedef int (*nanac_op_f)( nanac_t *cpu, uint8_t arga, uint8_t argb );


typedef struct {
	uint8_t mod;
	uint8_t cmd;
	uint8_t arga;
	uint8_t argb;
} nanac_op_t;


struct nanac_reg_s {
	void *ptr;
};
typedef struct nanac_reg_s nanac_reg_t;


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


typedef struct {
	nanac_mod_t idx[0xFF];
	uint8_t cnt;
} nanac_mods_t;


struct nanac_s {
	nanac_op_t *ops;
	uint16_t ops_sz;
	uint16_t eip;
	uint16_t call_stack[0xFF];
	uint8_t call_depth;
	nanac_reg_t regs[0xFF];
	uint8_t regs_win;
	nanac_mods_t *mods;
};

static inline uint16_t nanac_uint16( uint8_t arga, uint8_t argb ) {
	return arga | (argb<<8);
}


void nanac_mods_init( nanac_mods_t *mods );

int nanac_mods_add( nanac_mods_t *mods, const char *name, uint8_t cmds_len, const nanac_cmd_t cmds[] );

void nanac_init( nanac_t *cpu, nanac_mods_t *mods );

const nanac_op_t *nanac_op( const nanac_t *cpu, const uint16_t eip );

int nanac_step_epilogue( nanac_t *cpu, int ret );

int nanac_step( nanac_t *cpu, const nanac_op_t *op );

int nanac_run( nanac_t *cpu );

nanac_reg_t nanac_reg_get(const nanac_t *cpu, uint8_t reg);

void nanac_reg_set(nanac_t *cpu, const uint8_t reg, const nanac_reg_t val);

void nanac_mods_builtins ( nanac_mods_t *mods );

#endif
