#include "nanac.h"

#include <stdio.h>
#include <stdlib.h>


static void load_file (struct nanac_s *cpu, const char *filename)
{
    FILE *fp;
    long lSize;
    char *buffer;

    fp = fopen ( filename , "rb" );
    if( !fp )
    {
        perror(filename);
        exit(1);
    }

    fseek( fp, 0L, SEEK_END);
    lSize = ftell( fp );
    rewind( fp );

    if( (lSize < 4) || (lSize % 4) )
    {
        fclose(fp);
        fputs("invalid size",stderr);
        exit(1);
    }

    /* allocate memory for entire content */
    buffer = calloc( 1, lSize+1 );
    if( !buffer )
    {
        fclose(fp);
        fputs("memory alloc fails",stderr);
        exit(1);
    }

    /* copy the file into the buffer */
    if( 1 != fread( buffer, lSize, 1, fp) )
    {
        fclose(fp);
        free(buffer);
        fputs("entire read fails",stderr);
        exit(1);
    }

    /* do your work here, buffer is a string contains the whole text */
    cpu->ops = (struct nanac_op_s*)buffer;
    cpu->ops_sz = lSize / 4;
    fclose(fp);
}


static int print_mods (struct nanac_mods_s *mods)
{
    struct nanac_mod_s *mod;
    unsigned char mod_idx;
    unsigned char cmd_idx;

    for( mod_idx = 0; mod_idx < mods->cnt; mod_idx++ )
    {
        mod = &mods->idx[mod_idx];

        for( cmd_idx = 0; cmd_idx < mod->cmds_len; cmd_idx++ )
        {
            printf("%02X %02X %s %s\n", mod_idx, cmd_idx, mod->name,
                mod->cmds[cmd_idx].name);
        }
    }
    return 0;
}


static int print_help (char *name)
{
    fprintf(stderr, "Usage: %s [-tX] [file.bin ...]\n", name);
    fprintf(stderr, "  -t   Trace opcodes at runtime\n");
    fprintf(stderr, "  -X   Display opcode hex reference\n");
    return 10;
}


int main( int argc, char **argv )
{
    int flag_trace = 0;
    int i = 1;
    struct nanac_mods_s mods;
    struct nanac_s ctx;

    nanac_mods_init(&mods);
    nanac_mods_builtins(&mods);

    if( argc < 2 )
    {
        return print_help(argv[0]);
    }

    if( argv[1][0] == '-' )
    {
        char *flags_ptr = &argv[1][1];
        i = 2;
        while( *flags_ptr != 0 )
        {
            switch( *flags_ptr ) {
            case 't':
                flag_trace++;
                break;
            case 'X':
                return print_mods(&mods);
            default:
                fprintf(stderr, "error: invalid option %c\n", *flags_ptr);
                return print_help(argv[0]);
            }
            flags_ptr++;
        }
    }

    for( ; i < argc; i++ )
    {
        nanac_init(&ctx, &mods);
        load_file(&ctx, argv[i]);
        nanac_run(&ctx);

        if( ctx.ops )
        {
            free(ctx.ops);
        }
    }

    return 0;
}