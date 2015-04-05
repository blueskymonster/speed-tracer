#include <opencv2/highgui/highgui.hpp>
#include <opencv2/highgui/highgui_c.h>
#include <opencv2/video/background_segm.hpp>
#include <opencv2/core/utility.hpp>

#include <stdio.h>

#include <ctime>

#include <iostream>
#include <sstream>
#include <string.h>

#include "Camera.cpp"
#include "UDP.cpp"

using namespace cv;
using namespace std;

//global variables
Mat cameraFrame; //current frame from camera
Mat displayFrame; //frame to display
Mat fgMask; //current foreground mask learned by MOG method
Mat foregroundFrame; //Only stores foreground pixels
Ptr<BackgroundSubtractor> pMOG; //MOG Background subtractor
int keyboard;
int frame = 0; //current frame number
int learningFrames;
double learningRate;
bool udp_mode;

//debug variables
bool debug;
Mat backgroundModel;
clock_t start;

//function declarations
void videoProcessLoop(Camera * const, VideoWriter *, UDPSender *);

int main(int argc, char* argv[]) {
  const String commandLineKeys =
    "{help h usage ?    |                      | print this message                                                                }"
    "{debug             |                      | run in debug mode                                                                 }"
    "{learningFrames lf |0                     | sets number of frames to use to learn the background. (0 for continuous learning) }"
    "{learningRate lr   |-1                    | sets the background subtractors learning rate (between 0 and 1, negative for auto)}"
    "{webcam            |                      | use built-in cam as default                                                       }"
    "{udp               |                      | send foreground as 'set x y r b g' UDP packets                                    }"
    ;

  CommandLineParser clParser(argc, argv, commandLineKeys);
  if (clParser.has("help")) {
    clParser.printMessage();
    return 0;
  }

  debug = clParser.has("debug");
  udp_mode = clParser.has("udp");

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

  // Set up the camera object
  Camera * camera = getCamera(clParser.has("webcam"));

  // Set up UDP sender
  UDPSender * udpSender;
  if (udp_mode) {
    udpSender = new UDPSender();
  }

  // Set up a video writer if the `save` param has been set
  VideoWriter * videoWriter = 0;
  // if (clParser.has("save")) {
      Mat sizingFrame;
      try {
          camera->getFrame(sizingFrame);
      } catch (CameraException &e) {
          cerr << e.what() << " Unable to initalize video writer." << endl;
          return EXIT_FAILURE;
      }
      String fileName = clParser.getPathToApplication() + "/speedTrace.mpeg";
      videoWriter = new VideoWriter(
        fileName,
        CV_FOURCC('M', 'P', 'E', 'G'),
        24.0,
        sizingFrame.size(),
        true);
  // }

  //here's where the magic happens
  videoProcessLoop(camera, videoWriter, udpSender);

  destroyAllWindows();

  return EXIT_SUCCESS;
}

void videoProcessLoop(
  Camera * const camera,
  VideoWriter * videoWriter,
  UDPSender * udpSender) {

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
    cameraFrame.copyTo(foregroundFrame, fgMask);

    //display debug windows
    if (debug) {
      imshow("camera", cameraFrame);
      imshow("background model", backgroundModel);
      imshow("mask", fgMask);
      imshow("foreground", foregroundFrame);
    }

    if (udp_mode) {
      udpSender->sendColorPixels(foregroundFrame);
    }

    foregroundFrame = Mat::zeros(
      foregroundFrame.rows,
      foregroundFrame.cols,
      foregroundFrame.type());

    //type image buffer
    imshow("Speed Tracer", displayFrame);

    if (frame == 0) {
      start = clock();
    } else if (frame == 1000) {
      double duration = (clock() - start) / (double)CLOCKS_PER_SEC;
      cerr << "1000 frames shown in " << duration << " seconds." << endl;
      cerr << "Closing video writer" << endl;
    }

    if (videoWriter != NULL) {
      if (videoWriter->isOpened()) {
        videoWriter->write(displayFrame);
      } else {
        cerr << "Failed to open video file to write to." << endl;
      }
    }

    frame++;

    //get input from the keyboard
    keyboard = waitKey(30);
  }

  // Make sure to properly disconnect the cameras
  delete camera;
  delete videoWriter;
}
