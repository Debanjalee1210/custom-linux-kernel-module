#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <linux/input.h>
#include <errno.h>
#include "led_ioctl.h"

#define DEVICE_FILE "/dev/rgbled"
#define INPUT_EVENT_DEV "/dev/input/event2"  // Usually the keyboard device

volatile int stop_flag = 0; // Set by keyboard thread when SPACEBAR is pressed

void* input_monitor(void* arg) {
    int fd = open(INPUT_EVENT_DEV, O_RDONLY);
    if (fd < 0) {
        perror("Failed to open input device");
        return NULL;
    }

    struct input_event ev;
    while (read(fd, &ev, sizeof(ev)) > 0) {
        if (ev.type == EV_KEY && ev.code == KEY_SPACE && ev.value == 1) {
            printf("[Input] Spacebar pressed. Stopping...\n");
            stop_flag = 1;
            break;
        }
    }

    close(fd);
    return NULL;
}

int main(int argc, char **argv) {
    if (argc != 5) {
        printf("Usage: %s <PWM> <R> <G> <B>\n", argv[0]);
        return 1;
    }

    struct led_desc config;
    config.PWM = atoi(argv[1]);
    config.R = atoi(argv[2]);
    config.G = atoi(argv[3]);
    config.B = atoi(argv[4]);

    const char *base_patterns[] = { "R", "G", "B", "RG", "RB", "GB", "RGB" };
    int pattern_count = sizeof(base_patterns) / sizeof(base_patterns[0]);

    char patterns[7][16];  // Store generated patterns like "R70", "RGB30", etc.
    for (int i = 0; i < pattern_count; i++) 
    {
    	snprintf(patterns[i], sizeof(patterns[i]), "%s%d", base_patterns[i], config.PWM);
    }


    // Open RGB LED device
    int fd = open(DEVICE_FILE, O_WRONLY);
    if (fd < 0) {
        perror("Failed to open RGB LED device");
        return 1;
    }

    if (ioctl(fd, CONFIG, &config) < 0) 
    {
        perror("ioctl failed");
        close(fd);
        return 1;
    }

    // Launch keyboard input detection thread
    pthread_t input_thread;
    if (pthread_create(&input_thread, NULL, input_monitor, NULL) != 0) {
        perror("Failed to create input monitor thread");
        close(fd);
        return 1;
    }

    printf("LED config sent: PWM=%d, R=%d, G=%d, B=%d\n",config.PWM, config.R, config.G, config.B);

    // Main LED pattern loop
    int i = 0;
    while (!stop_flag) {
        const char *pat = patterns[i % pattern_count];
        printf("[LED] Sending pattern: %s\n", pat);
        if (write(fd, pat, strlen(pat)) < 0) {
            perror("Write failed");
            break;
        }
        sleep(1);
        i++;
    }

    printf("Pattern loop stopped.\n");
    pthread_join(input_thread, NULL);
    close(fd);
    return 0;
}

