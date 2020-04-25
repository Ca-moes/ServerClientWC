#include "utils.h"

struct timespec start_time;
struct timespec current_time;

void startTime(){
    clock_gettime(CLOCK_MONOTONIC, &start_time);
}

double elapsedTime(){
    clock_gettime(CLOCK_MONOTONIC, &current_time);
    return (current_time.tv_sec-start_time.tv_sec)*1000+((current_time.tv_nsec-start_time.tv_nsec)/10e6);
}
