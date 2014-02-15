#include <opencv2/highgui/highgui.hpp>
#include <opencv2/video/background_segm.hpp>

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

//debug variables
bool debug;
Mat backgroundModel;

//function declarations
void help();
void videoProcessLoop();

void help() {
  cout << "Usage: ./SpeedTracer [--help | --debug]"            << endl
       << "Options:"                                           << endl
       << "--help                          Shows this message" << endl
       << "--debug       Activates debug windows and messages" << endl;
}

int main(int argc, char* argv[]) {
  string helpOption = "--help";
  string debugOption = "--debug";

  debug = argc == 2 && argv[1] == debugOption;
  if (!debug && (argc > 2 || (argc == 2 && argv[1] == helpOption))) {
    help();
    return EXIT_FAILURE;
  }

  //create GUI window
  namedWindow("Speed Tracer");

  //debug windows
  if (debug) {
    namedWindow("camera"); //shows the raw camera feed
    //TODO: make a window that shows the currently learned background model
    namedWindow("mask"); //shows the foreground mask
    //TODO: make a window that shows the masked camera feed
  }

  //create background subtractor object
  pMOG = createBackgroundSubtractorMOG();

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

    //update the background model
    pMOG->apply(cameraFrame, fgMask);

    //copy only foreground pixels to the display image buffer
    cameraFrame.copyTo(displayFrame, fgMask);
    
    //display debug windows
    if (debug) {
      imshow("camera", cameraFrame);
      imshow("mask", fgMask);
    }

    //display image buffer
    imshow("Speed Tracer", displayFrame);

    //get input from the keyboard
    keyboard = waitKey(30);
  }
  
  //let the webcam go free
  cap.release();
}
