
#include "pi_controller.h"

struct piController initPI(float a1, float a0, float b1, float b0, float y0)
{
    return (struct piController) {.a1b1 = a1*b1, .a0b1 = a0*b1, .b0 = b0, .xprim = y0/a0/b1, .xprim_0 = y0/a0/b1};
}

float updatePI(struct piController *a, float in)
{
    float out = in*a->a1b1 + a->xprim*a->a0b1;
    a->xprim = in - a->b0*a->xprim;
    return out;
}

void resetPI(struct piController *a)
{
    a->xprim = a->xprim_0;
}
