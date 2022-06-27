
#ifndef INPUT_H_
#define INPUT_H_

typedef enum button {BtnNothing, BtnOff, BtnClrTrip, BtnOn, BtnDecr, BtnSelectRef, BtnIncr} button;
typedef enum parameter {Iout, Vclamp} adj_param;

button button_pressed(void);

#endif /* INPUT_H_ */
