# Four-phase current-fed push-pull converter controller
Controller for a four-phase current-fed push-pull topology. Based on a TMDSCNCD280039C control card. 

## Pinout

### PWM

| Card pin | Function     |
|---------:|:-------------|
|   49, 51 | PWM1A, PWM1B |
|   53, 55 | PWM2A, PWM2B |
|   50, 52 | PWM3A, PWM3B |
|   54, 56 | PWM4A, PWM4B |
|   57, 59 | PWM5A, PWM5B |
|   61, 63 | PWM6A, PWM6B |
|   58, 60 | PWM7A, PWM7B |
|   62, 64 | PWM8A, PWM8B |

### ADC

| Card pin | ADC channel | Function |
|---------:|:------------|:---------|
|        9 | A0          | Vin      |
|       11 | A1          | Vout     |
|       15 | A2          | Vclamp   |
|       17 | A4          | Iout     |

### Control buttons

| Card pin | GPIO    | Net | Function |
|---------:|:--------|:----|:---------|
|       89 | GPIO-40 | SW1 | Reference decrease |
|       99 | GPIO-49 | SW2 | Reference zero |
|      101 | GPIO-51 | SW3 | Reference increase |
|      103 | GPIO-48 | SW4 | Off |
|      105 | GPIO-50 | SW5 | Reset |
|      107 | GPIO-52 | SW6 | On |

### Status LEDs

| Card pin | GPIO    | LED indication |
|---------:|:--------|:---------|
|       72 | GPIO-22 | Overvoltage: Vclamp |
|       74 | GPIO-23 | Overvoltage: Vout |
|       76 | GPIO-28 | Overvoltage: Vin |
|       78 | GPIO-29 | Overcurrent |
|       80 | GPIO-30 | PI regulator OK: Io |
|       82 | GPIO-31 | PI regulator OK: Vclamp |

## ADC and sampling

All hardware ADC channels are bandwidth limited. The exact frequency needs to be investigated. 

Sampling instance needs to be investigated. Four channels need to be sampled: Vin, Vout, Vclamp and Iout. 

Voltage inputs are isolated with an ACPL-C87B isolation amplifier. Nominal voltage for this amplifier is 2V. Voltage dividers are tuned for nominal voltage magnitudes. Maximum input voltage for the amplifier is 2.46. 

### Input voltage

Voltage divider with an isolation amplifier. R1 = 6×665k. R2 = 10k. Attenuation = 400. 800V ➞ 2.0V. 984V ➞ 2.46V (max).

### Output voltage

Voltage divider with an isolation amplifier. R1 = 6×665k. R2 = 10k. Attenuation = 400. 200V ➞ 0.5V. 920V ➞ 2.3V. 984V ➞ 2.46V (max).

### Clamp current

Current sensor with an isolation amplifier. R1 = 9×665k + 2×10k. R2 = 10k. Attenuation = 601.5. 1200V ➞ 1.995V. 1480V ➞ 2.46V (max). 

### Output current

LES XX-NP current transducer. 0A = 2.5V±0.625·I/Inom. There is a voltage divider on the output of the current transducer. R1 = 3.3k, R2 = 6.8k. Attenuation = 0.6733. ADC should see 1.68±0.4208·I/Inom volts. 

## Protection

Undervoltage and overvoltage setpoints still need to be confirmed in simulations. Output current limits need to be verified experimentally. Ideally there should also be a protection if the controllers are not steady-state within some time period. 

### Input voltage

Nominal voltage Uin = 800V. 

Overvoltage protection set at 105%·Uin = 840V. 

Undervoltage protection set at 90%·Uin = 720V. 

### Output voltage

Nominal voltage range Uout = 200-920V. 

Overvoltage protection set at Uin−10V = 190V. 

Undervoltage protection set at Uout+10V = 930V. 

### Clamp voltage

Nominal volatge Uclamp = Uin·n2/n1 = 1200V.

Overvoltage protection set at 105%·Uin = 1260V. This needs to be tested and controller needs to be tuned not to have an overshoot. 

### Output current

TBD, depends on the hardware limits. 
