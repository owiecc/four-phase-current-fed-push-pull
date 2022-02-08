
#ifndef INPUT_H_
#define INPUT_H_

typedef enum button {BtnNothing, BtnOff, BtnClrTrip, BtnOn, BtnDecr, BtnZero, BtnIncr} button;

button button_pressed(void);

#endif /* INPUT_H_ */
