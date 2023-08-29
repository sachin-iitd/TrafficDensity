What does it do?

We have built a simple queue length detector which works on the idea of background subtraction. It detects a red/green light and update background accordingly. We detect vehicles by background subtraction and homography and are able to identify queue length with good accuracy in daytime traffic videos.


Commands to compile & run the code with GPU:
    
    nvcc -c CudaFunc.cu -o cfo.o
    g++ -c code_cod2.cpp -o ccd.o pkg-config --cflags --libs opencv
    g++ -o cod.o ccd.o cfo.o -L/usr/local/cuda/lib64 -lcudart pkg-config --cflags --libs opencv
    ./cod.o __1__  __2__  __3__  __4__  __5__
    
   Arguments:
   
   __1__is the video name
   
   __2 __  is whether or not you have to set four values to configure the projection thing (this must be set to true only when you run a particular video for the first time,next time you run the same video you may set it to false --please read below)
   
       If this is set to true, you must do following things manually:
       You must select 4 points(by clicking using mouse) in the frame that appears when you run the code, the polygon formed by joining these points is the portion of the road that we consider for projecting.
       Then press "Esc" key.
       Next again select 4 points(by clicking using mouse) from the next frame that appears the polygon formed by joining these points is the portion of the projection that we consider in queue length detection.
       Then press "Esc" key.
       Each time the points should be selected in the order: Top-Left, Bottom-Left, Bottom-Right, Top-Right.
       If this is set to false, nothing needs to be done manually.
   
   __3 __  is whether or not GPU is on
   
   __4 __  is if you want to limit the fps
   
   __5 __  is whether you want data to acquire as soon as it is processed or wait for whole one second


Commands to compile & run the code without GPU:

(After commenting few lines in the code{mentioned in code itself})

    g++ code_cod2.cpp -o cod.o pkg-config --cflags --libs opencv
    ./cod.o __1__  __2__  __3__  __4__  __5__
    
    
What can/needs to be set?

Following are the parameters that needs to be fit as per requirements of video.

    1. XFIT //Number of vehicles that would fit in horizontal, now lets us suppose we can fit 3 cars so we may want to have XFIT atleast 3 but since we can also have bikes we can make it 4 or 5 or 6 

    2. YFIT //Similar explanation but since bikes and cars have similar length, this is not a problem

    3. DYNAMIC_TIME //Time within which a frame would completely disappear from average, we may want to keep it small so that slow vehicles or stopped vehicles disappear quickly in seconds or FrameUnits

    4. STATIC_TIME //in seconds or FRAMEUNITS

    5. VARIANCE_FROM_AVG //variance from avg pixel, the variance from average pixel when it is considered an anomaly; be careful, should be between 10 to 40

    6. Arguments of find_line inside isRED_() function



Results /Output

    1. Results will be available in results_InputFileName_.txt as-> loop_number: percentage_white_area , ratio_from_top

    2. If loop_number is "n" this means that it is the report of queue length for nth "FRAMEUNIT frames"/ Second. FRAMEUNIT is same as Fps but can be changed.

    3. ratio_from_top is the ratio of the queue length from the top of the frame to the road length in PROJECTED_FRAME. Small ratio_from_top means large queues/ last vehicle at far end.

