speed-tracer
============

Real-time visual-tracer video effects

This project relies on [opencv 3.0.0](http://docs.opencv.org/trunk/doc/tutorials/introduction/table_of_content_introduction/table_of_content_introduction.html#table-of-content-introduction) 
for computer vision (under development at time of last modification of this file) and [cmake](http://www.cmake.org/) for building the project.

Build:

    mkdir build
    cd build
    cmake ..
    make

Run:

    ./SpeedTracer

Webcam issues
---------------
Many webcams have the habit of auto-adjusting color temperature, white balance, and brightness automatically.
While this is great for general purpose usage, it is undesirable in this application as it perturbs the background model which causes artifacts. 
Most webcams have driver settings that allow you to disable these auto-adjustments. 
I recommend that white balance, color temperature, and other color-affecting auto-adjustment features are disabled when using this application.

There is a script called webcam-settings.sh included in the root file which does these adjustments on linux systems with the v4l-utils tool installed.
On Debian based systems it can be installed with:
    sudo apt-get install v4l-utils
