// led_ioctl.h
#ifndef LED_IOCTL_H
#define LED_IOCTL_H

#include <linux/ioctl.h>

#define CONFIG 10

struct led_desc {
    int PWM;  // Duty cycle: 0–100
    int R;    // Red pin
    int G;    // Green pin
    int B;    // Blue pin
};

#endif

