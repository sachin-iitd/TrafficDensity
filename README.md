### RealTime Traffic Density Estimation

The `cam.cpp` contains the code to receive the video frames from the camera and process it using background subtraction to generate Queue Density and Stop Density values for the given camera.

The `bg` folder contains the background images for each camera used to initiate the background subtraction algorithm in the morning.

The `proj_points` folder contains the projection coordinates to transform 3D view of the camera to 2D vertical view.

The `traffic` folder contains input/output of the 3D to 2D transformation, after running cam.cpp.

The `results` folder will contain processed queue and stop densities per camera.

The `density.py` contains the logic to convert the queue density values to cityflow-simulator formatted dataset.
```
Usage: csvFile outFile StartHour TrafficFactor SplitFactor Mode
TrafficFactor = 0.1
SplitFactor = 0.3 
Mode = 0
```

The `cuda` folder contains the cuda optimized code for queue length detection.