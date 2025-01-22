# Ball Simulation

This project simulates a large number of moving balls within a defined space using a QuadTree for efficient spatial partitioning and collision detection. The visualization is handled using OpenCV.

## Table of Contents
- [Overview](#overview)
- [Dependencies](#dependencies)
- [Code Structure](#code-structure)
- [How to Run](#how-to-run)
- [Key Features](#key-features)

## Overview

The simulation creates and animates 200,000 balls of different types (e.g., aggressive) that move randomly within a defined window. A QuadTree is utilized to optimize the detection of collisions between these balls. 

## Dependencies

To run this code, you need:
- [OpenCV](https://opencv.org/) library (for visualization)
- [OpenMP](https://www.openmp.org/) (for parallel processing)

Ensure you have the necessary libraries installed and properly linked in your Visual Studio 2020 project.

## Code Structure

### Main Classes

- **Ball**: Represents each ball with properties such as diameter, position, type, direction, speed, and a method to move the ball.
- **Rectangle**: A helper class that defines a rectangular area and checks if balls are contained within or intersect with other rectangles.
- **QuadTree**: Implements the QuadTree data structure to efficiently manage and query ball positions.

### Functions

- `createBalls`: Generates a specified number of balls and adds them to the global vector.
- `moveBalls`: Updates the position of each ball based on its direction and speed.
- `checkCollisions`: Checks for collisions between balls using the QuadTree.
- `createMap`: Renders the current state of the simulation on a blank image.
- `display`: Displays the rendered image using OpenCV.

## How to Run

1. Open the project in Visual Studio 2020.
2. Ensure all dependencies are correctly configured.
3. Build the project.
4. Run the executable. A window titled "Simulation" will display the moving balls.

## Key Features

- **QuadTree Optimization**: The QuadTree structure significantly improves the performance of collision detection by reducing the number of comparisons needed.
- **Parallel Processing**: Leveraging OpenMP allows multiple threads to check for collisions concurrently, enhancing the speed of the simulation.
- **Dynamic Movement**: Balls change direction randomly, simulating realistic behavior.
