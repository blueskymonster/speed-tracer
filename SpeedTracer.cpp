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

//debug variables
bool debug;
Mat backgroundModel;

//function declarations
void videoProcessLoop();

int main(int argc, char* argv[]) {
  const String commandLineKeys =
    "{help h usage ? |    | print this message    }"
    "{debug          |    | run in debug mode     }"
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

  //create background subtractor object
  pMOG = createBackgroundSubtractorMOG2();

  //here's where the magic happens
  videoProcessLoop();

  destroyAllWindows();

  return EXIT_SUCCESS;
}

void videoProcessLoop() {
  //initialize video capture from whatever webcam is default
  VideoCapture cap(0);
  
  //verify that the feed has been acquired properly
  if (!cap.isOpened()) {
    cerr << "Failed to acquire camera" << endl;
    exit(EXIT_FAILURE);
  }

  //start the video loop. 'q' to quit
  while((char)keyboard != 'q') {
    //read frame from camera, quit if this fails
    if (!cap.read(cameraFrame)) {
      cerr << "Failed to read frame from camera" << endl;
      exit(EXIT_FAILURE);
    }
    flip(cameraFrame, cameraFrame, 1);

    //update the background model
    pMOG->apply(cameraFrame, fgMask);

    //copy only foreground pixels to the display image buffer
    cameraFrame.copyTo(displayFrame, fgMask);
    
    //display debug windows
    if (debug) {
      imshow("camera", cameraFrame);
      pMOG->getBackgroundImage(backgroundModel);
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
