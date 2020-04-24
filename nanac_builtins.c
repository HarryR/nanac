#include "nanac.h"


int reg_swp( struct nanac_s *cpu, unsigned char arga, unsigned char argb )
{
	struct nanac_reg_s tmp = nanac_reg_get(cpu, arga);
	nanac_reg_set(cpu, arga, nanac_reg_get(cpu, argb));
	nanac_reg_set(cpu, argb, tmp);
	return NANAC_OK;
}


int reg_mov( struct nanac_s *cpu, unsigned char arga, unsigned char argb )
{
	nanac_reg_set(cpu, arga, nanac_reg_get(cpu, argb));
	return NANAC_OK;
} 


int reg_clr( struct nanac_s *cpu, unsigned char arga, unsigned char argb )
{
	static const struct nanac_reg_s zero = {0};
	nanac_reg_set(cpu, arga, zero);
	nanac_reg_set(cpu, argb, zero);
	return NANAC_OK;
}


int reg_win( struct nanac_s *cpu, unsigned char arga, unsigned char argb )
{
	if( (cpu->regs_win + arga) > NANAC_SETTING_MAXREGS )
		return NANAC_ERROR_REGWIN_OVERFLOW;

	if( (cpu->regs_win - argb) < 0 )
		return NANAC_ERROR_REGWIN_UNDERFLOW;

	cpu->regs_win += arga;
	cpu->regs_win -= argb;
	return NANAC_OK;
}


int jmp_to( struct nanac_s *cpu, unsigned char arga, unsigned char argb )
{
	cpu->eip = nanac_uint16(arga, argb);
	return NANAC_NO_EPILOGUE;
}


int jmp_sub( struct nanac_s *cpu, unsigned char arga, unsigned char argb )
{
	if( cpu->call_depth >= NANAC_SETTING_MAXCALLS )
		return NANAC_ERROR_SUBRET_OVERFLOW;

	/* Save EIP in call stack */
	cpu->call_stack[cpu->call_depth] = (cpu->eip + 1);
	cpu->call_depth += 1;

	return jmp_to(cpu, arga, argb);
}


int jmp_ret( struct nanac_s *cpu, unsigned char arga, unsigned char argb )
{
	if( cpu->call_depth <= 0 )
		return NANAC_ERROR_SUBRET_UNDERFLOW;

	/* Restore EIP from call stack */
	cpu->call_depth -= 1;
	cpu->eip = cpu->call_stack[cpu->call_depth];

	return reg_win(cpu, arga, argb);
}


int jmp_die( struct nanac_s *cpu, unsigned char arga, unsigned char argb )
{
	(void)cpu;	/* unused */
	(void)arga;	/* unused */
	(void)argb;	/* unused */
	return NANAC_ERROR_DIE;
}


int cnd_common( struct nanac_s *cpu, int success )
{
	/* Skip next opcode if conditional not successful */
	if( ! success )
		cpu->eip += 1;

	return NANAC_OK;
}


int cnd_eq( struct nanac_s *cpu, unsigned char arga, unsigned char argb )
{
	return cnd_common( cpu, nanac_reg_get(cpu, arga).ptr == nanac_reg_get(cpu, argb).ptr );
}


int cnd_neq( struct nanac_s *cpu, unsigned char arga, unsigned char argb )
{
	return cnd_common( cpu, nanac_reg_get(cpu, arga).ptr != nanac_reg_get(cpu, argb).ptr );
}


int cnd_nil( struct nanac_s *cpu, unsigned char arga, unsigned char argb )
{
	return cnd_common( cpu, 0 == nanac_reg_get(cpu, arga).ptr || 0 == nanac_reg_get(cpu, argb).ptr );
}


int cnd_nz( struct nanac_s *cpu, unsigned char arga, unsigned char argb )
{
	return cnd_common( cpu, 0 != nanac_reg_get(cpu, arga).ptr || 0 != nanac_reg_get(cpu, argb).ptr );
}


void nanac_mods_builtins ( struct nanac_mods_s *mods )
{
	static const struct nanac_cmd_s _cmds_jmp[4] = {
		{"to", &jmp_to},
		{"die", &jmp_die},
		{"sub", &jmp_sub},
		{"ret", &jmp_ret}
	};

	static const struct nanac_cmd_s _cmds_cnd[4] = {
		{"eq", &cnd_eq},
		{"neq", &cnd_neq},
		{"nil", &cnd_nil},
		{"nz", &cnd_nz}
	};

	static const struct nanac_cmd_s _cmds_reg[4] = {
		{"mov", &reg_mov},
		{"clr", &reg_clr},
		{"swp", &reg_swp},
		{"win", &reg_win}
	};

	nanac_mods_add(mods, "jmp", 4, _cmds_jmp);
	nanac_mods_add(mods, "cnd", 4, _cmds_cnd);
	nanac_mods_add(mods, "reg", 4, _cmds_reg);
}
