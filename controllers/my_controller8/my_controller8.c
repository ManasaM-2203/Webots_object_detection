#include <webots/robot.h>
#include <webots/motor.h>
#include <webots/camera.h>
#include <webots/distance_sensor.h>
#include <stdio.h>

#define TIME_STEP 64

int main() {
  wb_robot_init();

  WbDeviceTag left_motor  = wb_robot_get_device("left wheel motor");
  WbDeviceTag right_motor = wb_robot_get_device("right wheel motor");
  wb_motor_set_position(left_motor,  INFINITY);
  wb_motor_set_position(right_motor, INFINITY);

  WbDeviceTag camera = wb_robot_get_device("camera");
  wb_camera_enable(camera, TIME_STEP);

  WbDeviceTag ps0 = wb_robot_get_device("ps0");
  WbDeviceTag ps7 = wb_robot_get_device("ps7");
  wb_distance_sensor_enable(ps0, TIME_STEP);
  wb_distance_sensor_enable(ps7, TIME_STEP);

  int counter = 0;

  // ===== SWEEP PLAN =====
  // Repeat: go forward long → turn 90° → go forward short → turn 90° → repeat
  // This makes the robot sweep the whole board like a lawnmower

  // Each "phase" in the sweep pattern:
  // phase 0: forward (long stripe)
  // phase 1: turn right 90°
  // phase 2: forward (short step sideways)
  // phase 3: turn right 90°
  // ... keeps repeating across full board

  int phase = 0;
  int phase_counter = 0;

  // Tune these to match your arena size:
  int FORWARD_LONG  = 250;  // steps to cross the full arena
  int FORWARD_SHORT = 60;   // steps to shift one lane sideways
  int TURN_90       = 55;   // steps to turn ~90 degrees

  // Alternate turn direction so robot snakes across board
  int turn_dir = 1; // 1 = right, -1 = left

  while (wb_robot_step(TIME_STEP) != -1) {
    counter++;
    phase_counter++;

    // ================= CAMERA =================
    const unsigned char *image = wb_camera_get_image(camera);
    int width  = wb_camera_get_width(camera);
    int height = wb_camera_get_height(camera);
    int x = width / 2;
    int y = height / 2;

    int r = 0, g = 0, b = 0, cnt = 0;
    for (int i = -3; i <= 3; i++) {
      for (int j = -3; j <= 3; j++) {
        int px = x + i, py = y + j;
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
    int thr = 40;
    if      (r > g + thr && r > b + thr) color = "RED";
    else if (g > r + thr && g > b + thr) color = "GREEN";
    else if (b > r + thr && b > g + thr) color = "BLUE";

    // ================= DISTANCE =================
    double d0 = wb_distance_sensor_get_value(ps0);
    double d7 = wb_distance_sensor_get_value(ps7);
    double distance = (d0 + d7) / 2.0;

    char *position;
    if      (distance < 50)  position = "VERY CLOSE";
    else if (distance < 100) position = "CLOSE";
    else if (distance < 200) position = "NEAR";
    else                     position = "FAR";

    if (counter % 15 == 0) {
      printf("\n=======================\n");
      printf("=== OBJECT DETECTED ===\n");
      printf("Color:    %s\n",   color);
      printf("Distance: %.2f\n", distance);
      printf("Position: %s\n",   position);
      printf("=======================\n");
      fflush(stdout);
    }

    // ================= WALL EMERGENCY ONLY =================
    // Only react to distance sensor if hitting a real wall (very high value)
    // Do NOT let distance sensor interrupt the sweep for boxes
    int wall_hit = (d0 > 250 || d7 > 250);

    if (wall_hit) {
      // Back up a little then continue sweep from next phase
      wb_motor_set_velocity(left_motor,  -3.0);
      wb_motor_set_velocity(right_motor, -3.0);
      // skip to turn phase
      if (phase == 0) {
        phase = 1;
        phase_counter = 0;
      }
      continue;
    }

    // ================= LAWNMOWER SWEEP =================
    switch (phase) {

      case 0: // GO FORWARD (long stripe across arena)
        wb_motor_set_velocity(left_motor,  5.0);
        wb_motor_set_velocity(right_motor, 5.0);
        if (phase_counter >= FORWARD_LONG) {
          phase = 1;
          phase_counter = 0;
        }
        break;

      case 1: // TURN 90° (direction alternates each stripe)
        if (turn_dir == 1) {
          wb_motor_set_velocity(left_motor,  3.0);
          wb_motor_set_velocity(right_motor, -3.0);
        } else {
          wb_motor_set_velocity(left_motor,  -3.0);
          wb_motor_set_velocity(right_motor,  3.0);
        }
        if (phase_counter >= TURN_90) {
          phase = 2;
          phase_counter = 0;
        }
        break;

      case 2: // MOVE SIDEWAYS (short step to next lane)
        wb_motor_set_velocity(left_motor,  5.0);
        wb_motor_set_velocity(right_motor, 5.0);
        if (phase_counter >= FORWARD_SHORT) {
          phase = 3;
          phase_counter = 0;
        }
        break;

      case 3: // TURN 90° AGAIN (now face back across arena)
        if (turn_dir == 1) {
          wb_motor_set_velocity(left_motor,  3.0);
          wb_motor_set_velocity(right_motor, -3.0);
        } else {
          wb_motor_set_velocity(left_motor,  -3.0);
          wb_motor_set_velocity(right_motor,  3.0);
        }
        if (phase_counter >= TURN_90) {
          phase = 0;           // back to long forward stripe
          phase_counter = 0;
          turn_dir = -turn_dir; // flip direction for next sweep
        }
        break;
    }
  }

  wb_robot_cleanup();
  return 0;
}