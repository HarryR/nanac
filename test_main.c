#include "nanac.h"

int native_nanac (nanac_t *cpu);

int main( int argc, char **argv )
{
	(void)argc;
	(void)argv;

    nanac_t ctx;
    nanac_init(&ctx, 0);
    return native_nanac(&ctx);
}