
#include "pi_controller.h"

struct piController initPI(float a1, float a0, float b1, float b0, float y0, float ylim_hi, float ylim_lo)
{
    return (struct piController) {.a1b1 = a1*b1, .a0b1 = a0*b1, .b0 = b0, .inv_a0b1 = 1/a0/b1, .yprim = y0/a0/b1, .yprim_0 = y0/a0/b1, .ylim_hi = ylim_hi, .ylim_lo = ylim_lo};
}

float updatePI(struct piController *a, float in)
{
    float out = in*a->a1b1 + a->yprim*a->a0b1;
    float out_cl = clamp(out, a->ylim_lo, a->ylim_hi);
    a->yprim = in - a->b0*a->yprim + (out_cl - out)*a->inv_a0b1;
    return out_cl;
}

void resetPI(struct piController *a)
{
    a->yprim = a->yprim_0;
}

inline float clamp(float x, float lo, float hi)
{
    return x <= lo ? lo : x >= hi ? hi : x;
}
