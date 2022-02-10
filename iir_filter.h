
#ifndef IIR_FILTER_H_
#define IIR_FILTER_H_

struct iirFilter {
    float as[3];
    float bs[3];
    float xs[3];
    float ys[3];
};

float update(struct iirFilter *, float);
void reset(struct iirFilter *);

#endif /* IIR_FILTER_H_ */
