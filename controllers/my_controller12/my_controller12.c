#include <webots/robot.h>
#include <webots/motor.h>
#include <webots/camera.h>
#include <webots/distance_sensor.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define TIME_STEP 64
#define NUM_SENSORS 8
#define OBSTACLE_THRESHOLD 50.0

typedef enum { FORWARD, REVERSE, TURN } State;

int main() {
  wb_robot_init();
  srand(time(NULL));

  WbDeviceTag left_motor  = wb_robot_get_device("left wheel motor");
  WbDeviceTag right_motor = wb_robot_get_device("right wheel motor");
  wb_motor_set_position(left_motor,  INFINITY);
  wb_motor_set_position(right_motor, INFINITY);

  WbDeviceTag camera = wb_robot_get_device("camera");
  wb_camera_enable(camera, TIME_STEP);

  const char *sensor_names[NUM_SENSORS] = {
    "ps0","ps1","ps2","ps3","ps4","ps5","ps6","ps7"
  };
  WbDeviceTag ps[NUM_SENSORS];
  for (int i = 0; i < NUM_SENSORS; i++) {
    ps[i] = wb_robot_get_device(sensor_names[i]);
    wb_distance_sensor_enable(ps[i], TIME_STEP);
  }

  State state     = FORWARD;
  int state_steps = 0;
  int turn_dir    = 1;
  int step_count  = 0;

  while (wb_robot_step(TIME_STEP) != -1) {
    step_count++;

    // ===== READ ALL 8 SENSORS =====
    double ps_val[NUM_SENSORS];
    for (int i = 0; i < NUM_SENSORS; i++)
      ps_val[i] = wb_distance_sensor_get_value(ps[i]);

    int front_hit = (ps_val[0] > OBSTACLE_THRESHOLD ||
                     ps_val[7] > OBSTACLE_THRESHOLD ||
                     ps_val[1] > OBSTACLE_THRESHOLD ||
                     ps_val[6] > OBSTACLE_THRESHOLD);
    int left_hit  = (ps_val[5] > OBSTACLE_THRESHOLD ||
                     ps_val[6] > OBSTACLE_THRESHOLD);
    int right_hit = (ps_val[1] > OBSTACLE_THRESHOLD ||
                     ps_val[2] > OBSTACLE_THRESHOLD);

    // ===== STATE MACHINE MOVEMENT =====
    double lv = 0, rv = 0;

    if (state == FORWARD) {
      if (front_hit) {
        state       = REVERSE;
        state_steps = 30;
        turn_dir    = (rand() % 2 == 0) ? 1 : -1;
      } else if (left_hit) {
        lv = 6.28; rv = 2.0;
      } else if (right_hit) {
        lv = 2.0; rv = 6.28;
      } else {
        lv = rv = 6.28;
      }
    } else if (state == REVERSE) {
      lv = rv = -4.0;
      state_steps--;
      if (state_steps <= 0) {
        state       = TURN;
        state_steps = 40 + rand() % 60;
        if (left_hit && !right_hit)      turn_dir =  1;
        else if (right_hit && !left_hit) turn_dir = -1;
      }
    } else if (state == TURN) {
      lv =  turn_dir * 4.0;
      rv = -turn_dir * 4.0;
      state_steps--;
      if (state_steps <= 0)
        state = FORWARD;
    }

    wb_motor_set_velocity(left_motor,  lv);
    wb_motor_set_velocity(right_motor, rv);

    // ===== CAMERA COLOR DETECTION =====
    const unsigned char *image = wb_camera_get_image(camera);
    int width  = wb_camera_get_width(camera);
    int height = wb_camera_get_height(camera);

    int red_pixels = 0, green_pixels = 0, blue_pixels = 0;

    for (int px = 0; px < width; px++) {
      for (int py = 0; py < height; py++) {
        int r = wb_camera_image_get_red  (image, width, px, py);
        int g = wb_camera_image_get_green(image, width, px, py);
        int b = wb_camera_image_get_blue (image, width, px, py);

        if (g > 80 && g > r * 2 && g > b * 2) green_pixels++;
        if (r > 80 && r > g * 2 && r > b * 2) red_pixels++;
        if (b > 80 && b > r * 2 && b > g * 2) blue_pixels++;
      }
    }

    const char *color = "SCANNING";
    int max_px = 0;
    if (red_pixels   > max_px) { max_px = red_pixels;   color = "RED";   }
    if (green_pixels > max_px) { max_px = green_pixels; color = "GREEN"; }
    if (blue_pixels  > max_px) { max_px = blue_pixels;  color = "BLUE";  }

    // ===== DISTANCE using ps0 (front sensor only) =====
    // ps0 points forward - reads whatever is directly in front
    // e-puck ps sensor lookup table (from Webots documentation):
    // object touching  -> ~4000
    // 1cm away         -> ~3000  
    // 2cm away         -> ~2000
    // 3cm away         -> ~1500
    // 5cm away         -> ~800
    // 10cm away        -> ~200
    // no object nearby -> ~0 to 70 (background noise)
    double dist = ps_val[0];

    const char *position;
    if      (dist > 2000) position = "VERY CLOSE";  // almost touching
    else if (dist > 800)  position = "NEAR";         // within a few cm
    else if (dist > 150)  position = "MEDIUM";       // moderate distance
    else                  position = "FAR AWAY";     // far or no object

    // ===== PRINT OUTPUT =====
    if (step_count % 8 == 0 && max_px > 0) {
      printf("\n=== OBJECT DETECTED ===\n");
      printf("Color   : %s\n", color);
      printf("Distance: %.2f\n", dist);
      printf("Position: %s\n", position);
      printf("=======================\n");
      fflush(stdout);
    }
  }

  wb_robot_cleanup();
  return 0;
}