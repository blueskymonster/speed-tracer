#include <opencv2/highgui/highgui.hpp>
#include <opencv2/video/background_segm.hpp>
#include <opencv2/core/utility.hpp>

#include <stdio.h>

#include <iostream>
#include <sstream>

using namespace cv;
using namespace std;

//global variables
Mat cameraFrame; //current frame from camera
Mat displayFrame; //frame to display
Mat fgMask; //current foreground mask learned by MOG method
Ptr<BackgroundSubtractor> pMOG; //MOG Background subtractor
int keyboard;
int frame = 0; //current frame number
String calibrationScript;
int learningFrames;
double learningRate;

//debug variables
bool debug;
Mat backgroundModel;

//function declarations
void calibrateRunningCamera();
void videoProcessLoop();

int main(int argc, char* argv[]) {
  const String commandLineKeys =
    "{help h usage ?    |                      | print this message                                                                }"
    "{debug             |                      | run in debug mode                                                                 }"
    "{calibrationScript |../do-nothing.sh      | sets the command to run to calibrate camera                                       }"
    "{learningFrames lf |0                     | sets number of frames to use to learn the background. (0 for continuous learning) }"
    "{learningRate lr   |-1                    | sets the background subtractors learning rate (between 0 and 1, negative for auto)}"
    ;

  CommandLineParser clParser(argc, argv, commandLineKeys);
  if (clParser.has("help")) {
    clParser.printMessage();
    return 0;
  }

  debug = clParser.has("debug");

  //debug windows
  if (debug) {
    namedWindow("camera"); //shows the raw camera feed
    namedWindow("background model"); //shows the current background model
    namedWindow("mask"); //shows the foreground mask
    //TODO: make a window that shows the masked camera feed
  }

  //create GUI window
  namedWindow("Speed Tracer");

  //get calibration script from command line args
  calibrationScript = clParser.get<String>("calibrationScript");

  //get learning frames param from command line args
  learningFrames = clParser.get<int>("learningFrames");

  //get learning rate for background subtractor;
  learningRate = clParser.get<double>("learningRate");

  //create background subtractor object
  pMOG = createBackgroundSubtractorMOG2();

  //here's where the magic happens
  videoProcessLoop();

  destroyAllWindows();

  return EXIT_SUCCESS;
}

void calibrateRunningCamera() {
  const char* calibScript = calibrationScript.c_str();
  system(calibScript);
}

void videoProcessLoop() {
  //initialize video capture from whatever webcam is default
  VideoCapture cap(0);
  
  //verify that the feed has been acquired properly
  if (!cap.isOpened()) {
    cerr << "Failed to acquire camera" << endl;
    exit(EXIT_FAILURE);
  }

  // Force camera on for calibration
  cap.read(cameraFrame);
  calibrateRunningCamera();

  //start the video loop. 'q' to quit
  while((char)keyboard != 'q') {
    //read frame from camera, quit if this fails
    if (!cap.read(cameraFrame)) {
      cerr << "Failed to read frame from camera" << endl;
      exit(EXIT_FAILURE);
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
  
  //let the webcam go free
  cap.release();
}
