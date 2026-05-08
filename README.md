# Autonomous Color Detection Robot using Webots

## Project Overview

This project implements an autonomous robot in Webots that can detect multiple colored objects in the environment using integrated sensors. The robot identifies object colors, estimates their position and distance in real time, and navigates autonomously within the arena.

---

## Features

* Autonomous robot movement
* Real-time object detection
* Color recognition using camera sensor
* Distance estimation using proximity sensor
* Dynamic console output
* Arena exploration with obstacle avoidance
* Detection of multiple objects (Red, Green, Blue)

---

## Technologies Used

* Webots Simulator
* C Programming
* e-puck Robot
* Camera Sensor
* Distance Sensor

---

## Environment Setup

The simulation environment contains:

* e-puck autonomous robot
* Colored objects (Red, Green, Blue)
* Floor arena
* Boundary walls for navigation

---

## Working Principle

1. The robot continuously scans the environment.
2. The camera captures image data.
3. RGB values are analyzed to identify object color.
4. Distance sensor measures proximity to objects.
5. The robot autonomously moves and avoids obstacles.
6. Object details are displayed dynamically in the console.

---

## Output

The system displays:

* Detected object color
* Distance value
* Position category (Very Close, Close, Medium, Far)

Example:

Color: RED
Distance: 45.22
Position: CLOSE

---

## Future Improvements

* Multi-object tracking
* Path planning algorithms
* AI-based object classification
* Voice-based control
* Real-world hardware implementation

---

## Author

Manasa M
