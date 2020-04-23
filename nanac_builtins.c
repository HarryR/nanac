#include "nanac.h"


int reg_swp( nanac_t *cpu, uint8_t arga, uint8_t argb )
{
	nanac_reg_t tmp = nanac_reg_get(cpu, arga);
	nanac_reg_set(cpu, arga, nanac_reg_get(cpu, argb));
	nanac_reg_set(cpu, argb, tmp);
	return NANAC_OK;
}


int reg_mov( nanac_t *cpu, uint8_t arga, uint8_t argb )
{
	nanac_reg_set(cpu, arga, nanac_reg_get(cpu, argb));
	return NANAC_OK;
} 


int reg_clr( nanac_t *cpu, uint8_t arga, uint8_t argb )
{
	static const nanac_reg_t zero = {0};
	nanac_reg_set(cpu, arga, zero);
	nanac_reg_set(cpu, argb, zero);
	return NANAC_OK;
}


int reg_win( nanac_t *cpu, uint8_t arga, uint8_t argb )
{
	if( (cpu->regs_win + arga) > 0xFF )
		return NANAC_ERROR_REGWIN_OVERFLOW;

	if( (cpu->regs_win - argb) < 0 )
		return NANAC_ERROR_REGWIN_UNDERFLOW;

	cpu->regs_win += arga;
	cpu->regs_win -= argb;
	return NANAC_OK;
}


int jmp_to( nanac_t *cpu, uint8_t arga, uint8_t argb )
{
	cpu->eip = nanac_uint16(arga, argb);
	return NANAC_NO_EPILOGUE;
}


int jmp_sub( nanac_t *cpu, uint8_t arga, uint8_t argb )
{
	if( cpu->call_depth >= 0xFF )
		return NANAC_ERROR_SUBRET_OVERFLOW;

	/* Save EIP in call stack */
	cpu->call_stack[cpu->call_depth] = (cpu->eip + 1);
	cpu->call_depth += 1;

	return jmp_to(cpu, arga, argb);
}


int jmp_ret( nanac_t *cpu, uint8_t arga, uint8_t argb )
{
	if( cpu->call_depth <= 0 )
		return NANAC_ERROR_SUBRET_UNDERFLOW;

	/* Restore EIP from call stack */
	cpu->call_depth -= 1;
	cpu->eip = cpu->call_stack[cpu->call_depth];

	return reg_win(cpu, arga, argb);
}


int jmp_die( nanac_t *cpu, uint8_t arga, uint8_t argb )
{
	(void)cpu;	/* unused */
	(void)arga;	/* unused */
	(void)argb;	/* unused */
	return NANAC_ERROR_DIE;
}


int cnd_common( nanac_t *cpu, int success )
{
	/* Skip next opcode if conditional not successful */
	if( ! success )
		cpu->eip += 1;

	return NANAC_OK;
}


int cnd_eq( nanac_t *cpu, uint8_t arga, uint8_t argb )
{
	return cnd_common( cpu, nanac_reg_get(cpu, arga).ptr == nanac_reg_get(cpu, argb).ptr );
}


int cnd_neq( nanac_t *cpu, uint8_t arga, uint8_t argb )
{
	return cnd_common( cpu, nanac_reg_get(cpu, arga).ptr != nanac_reg_get(cpu, argb).ptr );
}


int cnd_nil( nanac_t *cpu, uint8_t arga, uint8_t argb )
{
	return cnd_common( cpu, 0 == nanac_reg_get(cpu, arga).ptr || 0 == nanac_reg_get(cpu, argb).ptr );
}


int cnd_nz( nanac_t *cpu, uint8_t arga, uint8_t argb )
{
	return cnd_common( cpu, 0 != nanac_reg_get(cpu, arga).ptr || 0 != nanac_reg_get(cpu, argb).ptr );
}


void nanac_mods_builtins ( nanac_mods_t *mods )
{
	static const nanac_cmd_t _cmds_jmp[] = (const nanac_cmd_t[]){
		{"to", &jmp_to},
		{"die", &jmp_die},
		{"sub", &jmp_sub},
		{"ret", &jmp_ret},
	};

	static const nanac_cmd_t _cmds_cnd[] = (const nanac_cmd_t[]){
		{"eq", &cnd_eq},
		{"neq", &cnd_neq},
		{"nil", &cnd_nil},
		{"nz", &cnd_nz},
	};

	static const nanac_cmd_t _cmds_reg[] = (const nanac_cmd_t[]){
		{"mov", &reg_mov},
		{"clr", &reg_clr},
		{"swp", &reg_swp},
		{"win", &reg_win},
	};

	nanac_mods_add(mods, "jmp", 4, _cmds_jmp);
	nanac_mods_add(mods, "cnd", 4, _cmds_cnd);
	nanac_mods_add(mods, "reg", 4, _cmds_reg);
}
