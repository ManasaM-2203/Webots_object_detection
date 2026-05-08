#include <webots/robot.h>
#include <webots/motor.h>
#include <webots/camera.h>
#include <webots/distance_sensor.h>
#include <stdio.h>

#define TIME_STEP 64
#define SPEED 7.0          // increased speed
#define TURN_SPEED 5.0

int main() {
  wb_robot_init();

  WbDeviceTag left_motor  = wb_robot_get_device("left wheel motor");
  WbDeviceTag right_motor = wb_robot_get_device("right wheel motor");
  wb_motor_set_position(left_motor,  INFINITY);
  wb_motor_set_position(right_motor, INFINITY);
  wb_motor_set_velocity(left_motor,  0.0);
  wb_motor_set_velocity(right_motor, 0.0);

  WbDeviceTag camera = wb_robot_get_device("camera");
  wb_camera_enable(camera, TIME_STEP);

  // Enable ALL 8 proximity sensors
  WbDeviceTag ps[8];
  char ps_name[5];
  for (int i = 0; i < 8; i++) {
    sprintf(ps_name, "ps%d", i);
    ps[i] = wb_robot_get_device(ps_name);
    wb_distance_sensor_enable(ps[i], TIME_STEP);
  }

  int counter = 0;
  int stuck_counter = 0;

  // Lawnmower sweep state
  // 0=forward  1=turn_right  2=side_step  3=turn_right_again
  int phase = 0;
  int phase_timer = 0;
  int turn_dir = 1;  // 1=right, -1=left (alternates each row)

  // Tuned for e-puck arena — adjust if needed
  const int FORWARD_STEPS = 180;   // one full row crossing
  const int SIDE_STEPS    = 45;    // lane width shift
  const int TURN_STEPS    = 48;    // ~90 degree turn at TURN_SPEED

  while (wb_robot_step(TIME_STEP) != -1) {
    counter++;
    phase_timer++;

    // ===== READ ALL SENSORS =====
    double ps_val[8];
    for (int i = 0; i < 8; i++)
      ps_val[i] = wb_distance_sensor_get_value(ps[i]);

    // Front sensors: ps0 (front-right), ps7 (front-left)
    // Side sensors:  ps1, ps2 (right), ps5, ps6 (left)
    double front = (ps_val[0] + ps_val[7]) / 2.0;
    double front_right = ps_val[0];
    double front_left  = ps_val[7];

    // ===== CAMERA =====
    const unsigned char *image = wb_camera_get_image(camera);
    int width  = wb_camera_get_width(camera);
    int height = wb_camera_get_height(camera);

    int r = 0, g = 0, b = 0, cnt = 0;
    for (int i = -4; i <= 4; i++) {
      for (int j = -4; j <= 4; j++) {
        int px = width/2 + i, py = height/2 + j;
        if (px >= 0 && px < width && py >= 0 && py < height) {
          r += wb_camera_image_get_red(image,   width, px, py);
          g += wb_camera_image_get_green(image, width, px, py);
          b += wb_camera_image_get_blue(image,  width, px, py);
          cnt++;
        }
      }
    }
    r /= cnt; g /= cnt; b /= cnt;

    char *color = "UNKNOWN";
    int thr = 50;
    if      (r > g + thr && r > b + thr) color = "RED";
    else if (g > r + thr && g > b + thr) color = "GREEN";
    else if (b > r + thr && b > g + thr) color = "BLUE";

    // ===== DISTANCE LABEL =====
    char *position;
    if      (front < 50)  position = "FAR";
    else if (front < 100) position = "NEAR";
    else if (front < 200) position = "CLOSE";
    else                  position = "VERY CLOSE";

    if (counter % 20 == 0) {
      printf("\n=======================\n");
      printf("=== OBJECT DETECTED ===\n");
      printf("Color:    %s\n",   color);
      printf("Distance: %.2f\n", front);
      printf("Position: %s\n",   position);
      fflush(stdout);
    }

    // ===== WALL EMERGENCY =====
    // Only hard-stop for actual wall (very high sensor value)
    // e-puck sensors: wall ~ 300+, open space ~ 0-80
    int wall_ahead = (front_right > 200 || front_left > 200);
    int wall_any   = (ps_val[0] > 250 || ps_val[7] > 250 ||
                      ps_val[1] > 300 || ps_val[6] > 300);

    if (wall_any) {
      // Back up
      wb_motor_set_velocity(left_motor,  -3.0);
      wb_motor_set_velocity(right_motor, -3.0);
      stuck_counter++;

      // After backing up, force a turn
      if (stuck_counter > 15) {
        phase = 1;           // go to turn phase
        phase_timer = 0;
        stuck_counter = 0;
      }
      continue;
    }

    stuck_counter = 0;

    // ===== LAWNMOWER SWEEP =====
    switch (phase) {

      case 0: // Forward — cross the arena
        wb_motor_set_velocity(left_motor,  SPEED);
        wb_motor_set_velocity(right_motor, SPEED);
        if (phase_timer >= FORWARD_STEPS) {
          phase = 1;
          phase_timer = 0;
        }
        break;

      case 1: // First 90° turn
        if (turn_dir > 0) {
          wb_motor_set_velocity(left_motor,   TURN_SPEED);
          wb_motor_set_velocity(right_motor, -TURN_SPEED);
        } else {
          wb_motor_set_velocity(left_motor,  -TURN_SPEED);
          wb_motor_set_velocity(right_motor,  TURN_SPEED);
        }
        if (phase_timer >= TURN_STEPS) {
          phase = 2;
          phase_timer = 0;
        }
        break;

      case 2: // Short side step to next lane
        wb_motor_set_velocity(left_motor,  SPEED);
        wb_motor_set_velocity(right_motor, SPEED);
        if (phase_timer >= SIDE_STEPS) {
          phase = 3;
          phase_timer = 0;
        }
        break;

      case 3: // Second 90° turn (same direction = now facing back)
        if (turn_dir > 0) {
          wb_motor_set_velocity(left_motor,   TURN_SPEED);
          wb_motor_set_velocity(right_motor, -TURN_SPEED);
        } else {
          wb_motor_set_velocity(left_motor,  -TURN_SPEED);
          wb_motor_set_velocity(right_motor,  TURN_SPEED);
        }
        if (phase_timer >= TURN_STEPS) {
          phase = 0;
          phase_timer = 0;
          turn_dir = -turn_dir; // flip for next row
        }
        break;
    }
  }

  wb_robot_cleanup();
  return 0;
}