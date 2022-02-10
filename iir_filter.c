
#include "iir_filter.h"

float update(struct iirFilter *a, float in)
{
    a->xs[2] = a->xs[1];
    a->ys[2] = a->ys[1];
    a->xs[1] = a->xs[0];
    a->ys[1] = a->ys[0];
    a->xs[0] = in;
    a->ys[0] = a->xs[0]*a->as[0]
             + a->xs[1]*a->as[1]
             + a->xs[2]*a->as[2]
             - a->ys[1]*a->bs[1]
             - a->ys[2]*a->bs[2];

    return a->ys[0];
}

void reset(struct iirFilter *a)
{
    a->xs[0] = 0;
    a->xs[1] = 0;
    a->xs[2] = 0;
    a->ys[0] = 0;
    a->ys[1] = 0;
    a->ys[2] = 0;
}
