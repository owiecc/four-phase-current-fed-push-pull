
#ifndef ADC_H_
#define ADC_H_

struct ADCScaling {
    float gain;
    float offset;
};

struct ADCCalibration {
    struct ADCScaling coeffACD0;
    struct ADCScaling coeffACD1;
    struct ADCScaling coeffACD2;
    struct ADCScaling coeffACD3;
};

struct ADCResult {
    float Vin;
    float Vout;
    float Vclamp;
    float Iout;
};

void initADC(void);
void initADCSOC(void);
float scaleADC(unsigned int, struct ADCScaling);
struct ADCResult readADC(void);

#endif /* ADC_H_ */
