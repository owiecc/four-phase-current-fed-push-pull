
#ifndef PI_CONTROLLER_H_
#define PI_CONTROLLER_H_

struct piController {
    float a1b1;
    float a0b1;
    float b0;
    float xprim;
    float xprim_0;
};

struct piController initPI(float, float, float, float, float);
float updatePI(struct piController *, float);
void resetPI(struct piController *);

#endif /* PI_CONTROLLER_H_ */
