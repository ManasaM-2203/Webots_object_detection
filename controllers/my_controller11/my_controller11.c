#include <webots/robot.h>
#include <webots/motor.h>
#include <webots/camera.h>
#include <webots/distance_sensor.h>
#include <stdio.h>

#define TIME_STEP 64

int main() {

  wb_robot_init();

  // Motors
  WbDeviceTag left_motor =
    wb_robot_get_device("left wheel motor");

  WbDeviceTag right_motor =
    wb_robot_get_device("right wheel motor");

  wb_motor_set_position(left_motor, INFINITY);
  wb_motor_set_position(right_motor, INFINITY);

  // Camera
  WbDeviceTag camera =
    wb_robot_get_device("camera");

  wb_camera_enable(camera, TIME_STEP);

  // Distance Sensor
  WbDeviceTag ps =
    wb_robot_get_device("ps0");

  wb_distance_sensor_enable(ps, TIME_STEP);

  int counter = 0;

  while (wb_robot_step(TIME_STEP) != -1) {

    counter++;

    // ================= IMAGE =================

    const unsigned char *image =
      wb_camera_get_image(camera);

    int width = wb_camera_get_width(camera);
    int height = wb_camera_get_height(camera);

    int x = width / 2;
    int y = height / 2;

    int r = 0, g = 0, b = 0;
    int count = 0;

    // Check 5x5 area
    for (int i = -2; i <= 2; i++) {
      for (int j = -2; j <= 2; j++) {

        int px = x + i;
        int py = y + j;

        r += wb_camera_image_get_red(image, width, px, py);
        g += wb_camera_image_get_green(image, width, px, py);
        b += wb_camera_image_get_blue(image, width, px, py);

        count++;
      }
    }

    r /= count;
    g /= count;
    b /= count;

    // ================= COLOR =================

    char *color;

    if (r > g && r > b)
      color = "RED";

    else if (g > r && g > b)
      color = "GREEN";

    else if (b > r && b > g)
      color = "BLUE";

    else
      color = "UNKNOWN";

    // ================= DISTANCE =================

    double distance =
      wb_distance_sensor_get_value(ps);

    // Better categories
    char *position;

    if (distance < 30)
      position = "VERY NEAR";

    else if (distance < 70)
      position = "NEAR";

    else if (distance < 120)
      position = "MEDIUM";

    else
      position = "FAR";

    // ================= OUTPUT =================

    printf("\n=== OBJECT DETECTED ===\n");
    printf("Color: %s\n", color);
    printf("Distance: %.2f\n", distance);
    printf("Position: %s\n", position);
    printf("=======================\n");

    fflush(stdout);

    // ================= MOVEMENT =================

    if (distance < 35) {

      // Object too close
      // Rotate strongly and escape

      wb_motor_set_velocity(left_motor, -3.0);
      wb_motor_set_velocity(right_motor, 3.0);

    } else {

      // Roam around arena

      if (counter % 200 < 50) {

        // slight left
        wb_motor_set_velocity(left_motor, 2.0);
        wb_motor_set_velocity(right_motor, 4.0);

      } else if (counter % 200 < 100) {

        // slight right
        wb_motor_set_velocity(left_motor, 4.0);
        wb_motor_set_velocity(right_motor, 2.0);

      } else {

        // forward
        wb_motor_set_velocity(left_motor, 4.0);
        wb_motor_set_velocity(right_motor, 4.0);
      }
    }
  }

  wb_robot_cleanup();

  return 0;
}