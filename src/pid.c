#include "dlab_def.h"
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>

#define MAXS 10000 // Maximum number of samples

pthread_t ControlThreadID;
sem_t data_avail;

float theta[MAXS];
float ref[MAXS];

float Kp = 1.0;
float Ti = 1.0;
float Td = 0.1;
float N = 20;
float Fs = 200.0;
float run_time = 10.0;
int motor_number = 2;

void *ControlThread(void *arg) {
    int k = 0;
    int no_of_samples = (int)(run_time * Fs);
    float motor_position, ek, uk, ek_prev = 0, integral = 0, derivative, prev_derivative = 0;
    float dt = 1.0 / Fs;

    while (k < no_of_samples) {
        sem_wait(&data_avail);
        motor_position = EtoR(ReadEncoder());
        ek = ref[k] - motor_position;

        // Integral term
        integral += (ek / Ti) * dt;

        // Derivative term (Backward Difference Method)
        derivative = ((ek - ek_prev) / dt);
        derivative = (Td * derivative + prev_derivative) / (1 + Td / (N * dt));
        prev_derivative = derivative;

        // Compute PID control signal
        uk = Kp * (ek + integral + Td * derivative);
        DtoA(VtoD(uk));
        theta[k] = motor_position;
        ek_prev = ek;
        k++;
    }
    pthread_exit(NULL);
}

int main() {
    char selection;
    while (1) {
        printf("\nMenu:\n");
        printf("r: Run the control algorithm\n");
        printf("p: Change value of Kp\n");
        printf("i: Change value of Ti\n");
        printf("d: Change value of Td\n");
        printf("n: Change value of N\n");
        printf("f: Change sample frequency Fs\n");
        printf("t: Change total run time Tf\n");
        printf("u: Change input type (Step/Square)\n");
        printf("g: Plot motor position\n");
        printf("h: Save plot as Postscript\n");
        printf("q: Exit\n");
        printf("Enter your selection: ");
        scanf(" %c", &selection);

        switch (selection) {
            case 'r':
                sem_init(&data_avail, 0, 0);
                Initialize(Fs, motor_number);
                pthread_create(&ControlThreadID, NULL, ControlThread, NULL);
                pthread_join(ControlThreadID, NULL);
                Terminate();
                sem_destroy(&data_avail);
                break;

            case 'p':
                printf("Enter new Kp: ");
                scanf("%f", &Kp);
                break;

            case 'i':
                printf("Enter new Ti: ");
                scanf("%f", &Ti);
                break;

            case 'd':
                printf("Enter new Td: ");
                scanf("%f", &Td);
                break;

            case 'n':
                printf("Enter new N: ");
                scanf("%f", &N);
                break;

            case 'f':
                printf("Enter new sampling frequency Fs: ");
                scanf("%f", &Fs);
                break;

            case 't':
                printf("Enter new total run time Tf: ");
                scanf("%f", &run_time);
                break;

            case 'u': {
                char input_type;
                printf("Enter input type (s for Step, q for Square): ");
                scanf(" %c", &input_type);
                if (input_type == 's') {
                    float magnitude;
                    printf("Enter step magnitude (degrees): ");
                    scanf("%f", &magnitude);
                    magnitude = magnitude * (3.14159265359 / 180.0);
                    for (int i = 0; i < MAXS; i++) ref[i] = magnitude;
                } else if (input_type == 'q') {
                    float magnitude, frequency, duty_cycle;
                    printf("Enter square wave magnitude (degrees): ");
                    scanf("%f", &magnitude);
                    magnitude = magnitude * (3.14159265359 / 180.0);
                    printf("Enter frequency (Hz): ");
                    scanf("%f", &frequency);
                    printf("Enter duty cycle (%%): ");
                    scanf("%f", &duty_cycle);
                    Square(ref, MAXS, Fs, magnitude, frequency, duty_cycle);
                } else {
                    printf("Invalid input type.\n");
                }
                break;
            }

            case 'g':
                plot(ref, theta, Fs, MAXS, SCREEN, "PID Step Response", "Time (s)", "Position (rad)");
                break;

            case 'h':
                plot(ref, theta, Fs, MAXS, PS, "PID Step Response", "Time (s)", "Position (rad)");
                break;

            case 'q':
                exit(0);

            default:
                printf("Invalid selection\n");
        }
    }
}