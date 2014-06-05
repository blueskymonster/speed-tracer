#include <opencv2/highgui/highgui.hpp>
#include <opencv2/video/background_segm.hpp>
#include <opencv2/core/utility.hpp>

#include <flycapture/FlyCapture2.h>

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
void videoProcessLoop(bool);

int main(int argc, char* argv[]) {
  const String commandLineKeys =
    "{help h usage ?    |                      | print this message                                                                }"
    "{debug             |                      | run in debug mode                                                                 }"
    "{calibrationScript |../do-nothing.sh      | sets the command to run to calibrate camera                                       }"
    "{learningFrames lf |0                     | sets number of frames to use to learn the background. (0 for continuous learning) }"
    "{learningRate lr   |-1                    | sets the background subtractors learning rate (between 0 and 1, negative for auto)}"
    "{fleaCam           |                      | use the point grey flea cam drivers                                               }"
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
    //TODO: make a window that shows the masked camera feed
  } else {
    cout << "Running in fullscreen mode" << endl;
    setWindowProperty("Speed Tracer", WND_PROP_FULLSCREEN, WINDOW_FULLSCREEN);
  }

  //What type of camera are we using?
  bool useFleaCam = clParser.has("fleaCam");

  //get calibration script from command line args
  calibrationScript = clParser.get<String>("calibrationScript");

  //get learning frames param from command line args
  learningFrames = clParser.get<int>("learningFrames");

  //get learning rate for background subtractor;
  learningRate = clParser.get<double>("learningRate");

  //create background subtractor object
  pMOG = createBackgroundSubtractorMOG2();

  //here's where the magic happens
  videoProcessLoop(useFleaCam);

  destroyAllWindows();

  return EXIT_SUCCESS;
}

void calibrateRunningCamera() {
  const char* calibScript = calibrationScript.c_str();
  system(calibScript);
}

void videoProcessLoop(bool useFleaCam) {

  // //initialize video capture from whatever webcam is default
  // VideoCapture cap(0);

  // //verify that the feed has been acquired properly
  // if (!cap.isOpened()) {
  //   cerr << "Failed to acquire camera" << endl;
  //   exit(EXIT_FAILURE);
  // }

  // // Force camera on for calibration
  // cap.read(cameraFrame);

  FlyCapture2::Error error;
  FlyCapture2::Camera camera;
  FlyCapture2::CameraInfo camInfo;

  error = camera.Connect(0);
  if (error != FlyCapture2::PGRERROR_OK) {
    cout << "Failed to connect to camera" << endl;
    return;
  }

  error = camera.GetCameraInfo(&camInfo);
  if (error != FlyCapture2::PGRERROR_OK) {
    cout << "Failed to get camera info from camera" << endl;
    return;
  }

  cout << camInfo.vendorName << " " << camInfo.modelName << " " << camInfo.serialNumber << endl;

  error = camera.StartCapture();
  if (error == FlyCapture2::PGRERROR_ISOCH_BANDWIDTH_EXCEEDED) {
    cout << "Bandwidth exceeded" << endl;
    return;
  } else if (error != FlyCapture2::PGRERROR_OK) {
    cout << "Failed to start image capture" << endl;
    return;
  }

  FlyCapture2::Image rawImage;
  FlyCapture2::Image rgbImage;
  unsigned int rowBytes;

  calibrateRunningCamera();

  //start the video loop. 'q' to quit
  while((char)keyboard != 'q') {
    // //read frame from camera, quit if this fails
    // if (!cap.read(cameraFrame)) {
    //   cerr << "Failed to read frame from camera" << endl;
    //   exit(EXIT_FAILURE);
    // }
    // flip(cameraFrame, cameraFrame, 1);
    error = camera.RetrieveBuffer(&rawImage);
    if (error != FlyCapture2::PGRERROR_OK) {
      cout << "capture error" << endl;
      continue;
    }
    rawImage.Convert(FlyCapture2::PIXEL_FORMAT_BGR, &rgbImage);

    rowBytes = (double)rgbImage.GetReceivedDataSize()/(double)rgbImage.GetRows();
    cameraFrame = Mat(rgbImage.GetRows(), rgbImage.GetCols(), CV_8UC3, rgbImage.GetData(), rowBytes);


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

  // //let the webcam go free
  // cap.release();
  camera.StopCapture();
  camera.Disconnect();
}
