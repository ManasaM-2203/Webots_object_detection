#include <webots/robot.h>
#include <stdio.h>

#define TIME_STEP 64

int main() {
  wb_robot_init();

  printf("PROGRAM STARTED\n");
  fflush(stdout);

  while (wb_robot_step(TIME_STEP) != -1) {
    printf("RUNNING LOOP\n");
    fflush(stdout);
  }

  return 0;
}