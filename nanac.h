#ifndef NANAC_H_
#define NANAC_H_

#ifndef NANAC_SETTING_MAXREGS
#define NANAC_SETTING_MAXREGS 0xFF
#endif

#ifndef NANAC_SETTING_MAXCALLS
#define NANAC_SETTING_MAXCALLS 0xFF
#endif

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

struct nanac_op_s {
	unsigned char mod;
	unsigned char cmd;
	unsigned char arga;
	unsigned char argb;
};


struct nanac_reg_s {
	void *ptr;
};


struct nanac_cmd_s {
	const char *name;
	int (*run)( struct nanac_s *cpu, unsigned char arga, unsigned char argb );
};


struct nanac_mod_s {
	const char *name;
	unsigned char cmds_len;
	const struct nanac_cmd_s *cmds;
};


struct nanac_mods_s {
	struct nanac_mod_s idx[0xFF];
	unsigned char cnt;
};


struct nanac_s {
	struct nanac_op_s *ops;
	unsigned short ops_sz;
	unsigned short eip;
	unsigned short call_stack[0xFF];
	unsigned char call_depth;
	struct nanac_reg_s regs[0xFF];
	unsigned char regs_win;
	struct nanac_mods_s *mods;
};

#define nanac_uint16(arga, argb) ( ((arga)&0xFF) | (((argb)&0xFF)<<8) )


void nanac_mods_init( struct nanac_mods_s *mods );

int nanac_mods_add( struct nanac_mods_s *mods, const char *name, const unsigned char cmds_len, const struct nanac_cmd_s *cmds );

void nanac_init( struct nanac_s *cpu, struct nanac_mods_s *mods );

const struct nanac_op_s *nanac_op( const struct nanac_s *cpu, const unsigned short eip );

int nanac_step_epilogue( struct nanac_s *cpu, int ret );

int nanac_step( struct nanac_s *cpu, const struct nanac_op_s *op );

int nanac_run( struct nanac_s *cpu );

struct nanac_reg_s nanac_reg_get(const struct nanac_s *cpu, const unsigned char reg);

void nanac_reg_set(struct nanac_s *cpu, const unsigned char reg, const struct nanac_reg_s val);

void nanac_mods_builtins ( struct nanac_mods_s *mods );

/* ifdef NANAC_H_ */
#endif
