#include "utils.h"

struct timespec start_time;
struct timespec current_time;

void startTime(){
  if(clock_gettime(CLOCK_MONOTONIC, &start_time) == -1){
    perror("startTime");
  }
}

double elapsedTime(){
  if(clock_gettime(CLOCK_MONOTONIC, &current_time) == -1){
    perror("elapsedTime");
  }
  return (current_time.tv_sec-start_time.tv_sec)*1000+((current_time.tv_nsec-start_time.tv_nsec)/10e6);
}
