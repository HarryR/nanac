#include "nanac.h"
#include <string.h>
#include <stdio.h>


void nanac_init( struct nanac_s *cpu, struct nanac_mods_s *mods )
{
	memset(cpu, 0, sizeof(*cpu));
	cpu->mods = mods;
}


void nanac_mods_init( struct nanac_mods_s *mods )
{
	memset(mods, 0, sizeof(*mods));
}


int nanac_mods_add( struct nanac_mods_s *mods, const char *name, const unsigned char cmds_len, const struct nanac_cmd_s *cmds )
{
	struct nanac_mod_s *mod;

	/* Constrained by uint8 */
	if( mods->cnt >= 0xFF )
		return 0;

	mod = &mods->idx[mods->cnt++];
	mod->name = name;
	mod->cmds_len = cmds_len;
	mod->cmds = cmds;

	return 1;
}


#define FLAG_VALID(cpu, offset) (offset < (8*sizeof(cpu->flags)))


const struct nanac_op_s *nanac_op( const struct nanac_s *cpu, const unsigned short eip )
{
	if( eip < cpu->ops_sz )
		return &cpu->ops[eip];

	return NULL;
}


struct nanac_reg_s nanac_reg_get(const struct nanac_s *cpu, const unsigned char reg)
{
	return cpu->regs[(unsigned char)(cpu->regs_win + reg)];
}


void nanac_reg_set(struct nanac_s *cpu, const unsigned char reg, const struct nanac_reg_s val)
{
	cpu->regs[(unsigned char)(cpu->regs_win + reg)] = val;
}


int nanac_step_epilogue( struct nanac_s *cpu, int ret )
{
	if( ret == NANAC_OK )
		cpu->eip += 1;

	return ret;
}


int nanac_step( struct nanac_s *cpu, const struct nanac_op_s *op )
{
	const struct nanac_mod_s *mod = &cpu->mods->idx[op->mod];
	const struct nanac_cmd_s *cmd;

	if( ! mod )
		return NANAC_ERROR_STEP_NOMOD;

	if( op->cmd >= mod->cmds_len )
		return NANAC_ERROR_STEP_NOCMD;

	cmd = &mod->cmds[op->cmd];

#ifdef NANAC_TRACE
	printf("@%-4X %s %s %d %d\n", cpu->eip, mod->name, cmd->name, op->arga, op->argb);
#endif

	return nanac_step_epilogue(cpu, cmd->run(cpu, op->arga, op->argb));
}


int nanac_run( struct nanac_s *cpu )
{
	int escape = 0;

	while( escape >= 0 )
	{
		const struct nanac_op_s *op = nanac_op( cpu, cpu->eip );

		if( ! op ) {
			escape = NANAC_ERROR_STEP_BADOP;
			break;
		}

		escape = nanac_step( cpu, op );
	}

	return escape;
}
