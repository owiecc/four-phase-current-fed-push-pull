
#ifndef PI_CONTROLLER_H_
#define PI_CONTROLLER_H_

struct piController {
    float a1b1;
    float a0b1;
    float b0;
    float inv_a0b1;
    float yprim;
    float yprim_0;
    float ylim_hi;
    float ylim_lo;
};

struct piController initPI(float, float, float, float, float, float, float);
float updatePI(struct piController *, float);
void resetPI(struct piController *);
inline float clamp(float, float, float);

#endif /* PI_CONTROLLER_H_ */
