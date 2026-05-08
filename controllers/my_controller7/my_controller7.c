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

      case 0: // GO FORWARD (long stripe across arena)
        wb_motor_set_velocity(left_motor,  5.0);
        wb_motor_set_velocity(right_motor, 5.0);
     