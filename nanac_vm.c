#include "nanac.h"

#include <string.h>
#include <stdio.h>


void nanac_init( nanac_t *cpu, nanac_mods_t *mods )
{
	memset(cpu, 0, sizeof(*cpu));
	cpu->mods = mods;
}


void nanac_mods_init( nanac_mods_t *mods )
{
	memset(mods, 0, sizeof(*mods));
}


int nanac_mods_add( nanac_mods_t *mods, const char *name, const uint8_t cmds_len, const nanac_cmd_t cmds[] )
{
	nanac_mod_t *mod;

	if( mods->cnt >= 0xFF )
		return 0;

	mod = &mods->idx[mods->cnt++];
	mod->name = name;
	mod->cmds_len = cmds_len;
	mod->cmds = cmds;

	return 1;
}


#define FLAG_VALID(cpu, offset) (offset < (8*sizeof(cpu->flags)))


const nanac_op_t *nanac_op( const nanac_t *cpu, const uint16_t eip )
{
	if( eip < cpu->ops_sz )
		return &cpu->ops[eip];

	return NULL;
}


nanac_reg_t nanac_reg_get(const nanac_t *cpu, const uint8_t reg)
{
	return cpu->regs[(uint8_t)(cpu->regs_win + reg)];
}


void nanac_reg_set(nanac_t *cpu, const uint8_t reg, const nanac_reg_t val)
{
	cpu->regs[(uint8_t)(cpu->regs_win + reg)] = val;
}


int nanac_step_epilogue( nanac_t *cpu, int ret )
{
	if( ret == NANAC_OK )
		cpu->eip += 1;

	return ret;
}


int nanac_step( nanac_t *cpu, const nanac_op_t *op )
{
	const nanac_mod_t *mod = &cpu->mods->idx[op->mod];
	const nanac_cmd_t *cmd;

	if( ! mod )
		return NANAC_ERROR_STEP_NOMOD;

	if( op->cmd >= mod->cmds_len )
		return NANAC_ERROR_STEP_NOCMD;

	cmd = &mod->cmds[op->cmd];

#ifdef TRACE
	printf("@%-4X %s %s %d %d\n", cpu->eip, mod->name, cmd->name, op->arga, op->argb);
#endif

	return nanac_step_epilogue(cpu, cmd->run(cpu, op->arga, op->argb));
}


int nanac_run( nanac_t *cpu )
{
	int escape = 0;

	while( escape >= 0 )
	{
		const nanac_op_t *op = nanac_op( cpu, cpu->eip );

		if( ! op ) {
			escape = NANAC_ERROR_STEP_BADOP;
			break;
		}

		escape = nanac_step( cpu, op );
	}

	return escape;
}
