#include <webots/robot.h>
#include <webots/motor.h>
#include <webots/camera.h>
#include <webots/distance_sensor.h>
#include <stdio.h>

#define TIME_STEP 64

int main() {
  wb_robot_init();

  // Motors
  WbDeviceTag left_motor = wb_robot_get_device("left wheel motor");
  WbDeviceTag right_motor = wb_robot_get_device("right wheel motor");

  wb_motor_set_position(left_motor, INFINITY);
  wb_motor_set_position(right_motor, INFINITY);

  // Camera
  WbDeviceTag camera = wb_robot_get_device("camera");
  wb_camera_enable(camera, TIME_STEP);

  // Distance sensor
  WbDeviceTag ps = wb_robot_get_device("ps0");
  wb_distance_sensor_enable(ps, TIME_STEP);

  while (wb_robot_step(TIME_STEP) != -1) {

    // 📸 Capture Image
    const unsigned char *image = wb_camera_get_image(camera);
    int width = wb_camera_get_width(camera);
    int height = wb_camera_get_height(camera);

    int x = width / 2;
    int y = height / 2;

    int r = wb_camera_image_get_red(image, width, x, y);
    int g = wb_camera_image_get_green(image, width, x, y);
    int b = wb_camera_image_get_blue(image, width, x, y);

    // 🎨 Color Detection
    char *color;

    if (r > g && r > b)
      color = "RED";
    else if (g > r && g > b)
      color = "GREEN";
    else if (b > r && b > g)
      color = "BLUE";
    else
      color = "UNKNOWN";

    // 📏 Distance
    double distance = wb_distance_sensor_get_value(ps);

    // 📍 Improved Position Categories
    char *position;
    if (distance < 40)
      position = "VERY CLOSE";
    else if (distance < 80)
      position = "CLOSE";
    else if (distance < 120)
      position = "MEDIUM";
    else
      position = "FAR";

    // 🖥️ Output
    printf("\n=== OBJECT DETECTED ===\n");
    printf("Color: %s\n", color);
    printf("Distance: %.2f\n", distance);
    printf("Position: %s\n", position);
    printf("=======================\n");
    fflush(stdout);

    // 🚗 MOVEMENT LOGIC (FIXED + SMOOTH)

    if (distance > 120) {
      // No object → rotate to search
      wb_motor_set_velocity(left_motor, -1.0);
      wb_motor_set_velocity(right_motor, 1.0);
    }
    else if (distance > 60) {
      // Object detected → move forward
      wb_motor_set_velocity(left_motor, 2.0);
      wb_motor_set_velocity(right_motor, 2.0);
    }
    else {
      // Very close → stop
      wb_motor_set_velocity(left_motor, 0.0);
      wb_motor_set_velocity(right_motor, 0.0);
    }
  }

  wb_robot_cleanup();
  return 0;
}