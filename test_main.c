#include "nanac.h"

int native_nanac (struct nanac_s *cpu);

int main( int argc, char **argv )
{
    struct nanac_s ctx;

	(void)argc;
	(void)argv;

    nanac_init(&ctx, 0);
    return native_nanac(&ctx);
}
