# Task2 - 3D Kinetic Sculpture (Glass + Water)

This folder contains the **assignment submission files** for:
**Assignment 2: 3D Kinetic Sculpture** (multiple lights + 3D transformation + generated geometry).

## What it does
- Procedurally generates a **glass cylinder mesh** (no model download needed).
- Simulates **water** as many small particles with simple physics inside the glass.
- Uses **multiple light sources** (4 point lights + directional + spotlight like LearnOpenGL).
- Interactive: rotate the glass and move the camera.

## Controls
- **W/A/S/D**: move camera
- **Mouse**: look around
- **Scroll**: zoom
- **I/K**: tilt glass up/down (pitch)
- **J/L**: rotate glass left/right (yaw)
- **ESC**: quit

## How to run (LearnOpenGL project required)
This is written to be compiled inside the LearnOpenGL CMake project.

1. Copy these files into LearnOpenGL:
   - Copy `task2_glass.cpp` to `LearnOpenGL-master/src/2.lighting/6.multiple_lights_task2_glass/`
   - Copy the 4 shader files (`task2_*.vs`, `task2_*.fs`) to the same folder.
   - Ensure `LearnOpenGL-master/CMakeLists.txt` includes `6.multiple_lights_task2_glass` in `set(2.lighting ...)`.

2. Build target:
   - `2.lighting__6.multiple_lights_task2_glass`

3. Run the produced `.exe` from LearnOpenGL `bin/2.lighting/Debug/`.

