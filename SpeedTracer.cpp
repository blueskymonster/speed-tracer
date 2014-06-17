#include <opencv2/highgui/highgui.hpp>
#include <opencv2/video/background_segm.hpp>
#include <opencv2/core/utility.hpp>

#include <stdio.h>

#include <iostream>
#include <sstream>

#include "Camera.cpp"

using namespace cv;
using namespace std;

//global variables
Mat cameraFrame; //current frame from camera
Mat displayFrame; //frame to display
Mat fgMask; //current foreground mask learned by MOG method
Ptr<BackgroundSubtractor> pMOG; //MOG Background subtractor
int keyboard;
int frame = 0; //current frame number
int learningFrames;
double learningRate;

//debug variables
bool debug;
Mat backgroundModel;

//function declarations
void videoProcessLoop(Camera * const);

int main(int argc, char* argv[]) {
  const String commandLineKeys =
    "{help h usage ?    |                      | print this message                                                                }"
    "{debug             |                      | run in debug mode                                                                 }"
    "{learningFrames lf |0                     | sets number of frames to use to learn the background. (0 for continuous learning) }"
    "{learningRate lr   |-1                    | sets the background subtractors learning rate (between 0 and 1, negative for auto)}"
    "{webcam            |                      | use built-in cam as default                                                       }"
    ;

  CommandLineParser clParser(argc, argv, commandLineKeys);
  if (clParser.has("help")) {
    clParser.printMessage();
    return 0;
  }

  debug = clParser.has("debug");

  //create GUI window
  namedWindow("Speed Tracer", WINDOW_NORMAL);

  //debug windows
  if (debug) {
    namedWindow("camera", WINDOW_NORMAL); //shows the raw camera feed
    namedWindow("background model", WINDOW_NORMAL); //shows the current background model
    namedWindow("mask", WINDOW_NORMAL); //shows the foreground mask
  } else {
    setWindowProperty("Speed Tracer", WND_PROP_FULLSCREEN, WINDOW_FULLSCREEN);
  }

  //get learning frames param from command line args
  learningFrames = clParser.get<int>("learningFrames");

  //get learning rate for background subtractor;
  learningRate = clParser.get<double>("learningRate");

  //create background subtractor object
  pMOG = createBackgroundSubtractorMOG2();

  //here's where the magic happens
  videoProcessLoop(getCamera(clParser.has("webcam")));

  destroyAllWindows();

  return EXIT_SUCCESS;
}

void videoProcessLoop(Camera * const camera) {
  // 'q' to quit
  while((char)keyboard != 'q') {
    try {
      camera->getFrame(cameraFrame);
    } catch (CameraException &e) {
      cerr << e.what() << endl;
      continue;
    }

    flip(cameraFrame, cameraFrame, 1);

    //update the background model
    if (learningFrames == 0 || frame < learningFrames) {
      pMOG->apply(cameraFrame, fgMask, learningRate);
      pMOG->getBackgroundImage(backgroundModel);
    }

    //copy only foreground pixels to the display image buffer
    cameraFrame.copyTo(displayFrame, fgMask);

    //display debug windows
    if (debug) {
      imshow("camera", cameraFrame);
      imshow("background model", backgroundModel);
      imshow("mask", fgMask);
    }

    //display image buffer
    imshow("Speed Tracer", displayFrame);

    frame++;

    //get input from the keyboard
    keyboard = waitKey(30);
  }

  // Make sure to properly disconnect the cameras
  delete camera;
}
