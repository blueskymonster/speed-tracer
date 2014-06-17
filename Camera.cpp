#include <stdio.h>
#include <exception>
#include <stdexcept>
#include <opencv2/core/utility.hpp>
#include <flycapture/FlyCapture2.h>

using namespace std;

class CameraException : public runtime_error {
  public:
    CameraException(const string& message) : runtime_error(message) {}
};

// Abstract base class for cameras
class Camera {
  public:
    virtual void getFrame(cv::Mat &dest) =0;
    virtual ~Camera() {};
};

class FleaCam : public Camera {
  private:
    FlyCapture2::Error error;
    FlyCapture2::Camera camera;
    FlyCapture2::Image rawImage;
    FlyCapture2::Image rgbImage;
    unsigned int rowBytes;
  public:
    FleaCam() {
      error = camera.Connect(0);
      if (error != FlyCapture2::PGRERROR_OK) {
        throw CameraException("Failed to connect to FleaCam");
      }
      error = camera.StartCapture();
      if (error == FlyCapture2::PGRERROR_ISOCH_BANDWIDTH_EXCEEDED) {
        throw CameraException("FleaCam bandwidth exceeded");
      } else if (error != FlyCapture2::PGRERROR_OK) {
        throw CameraException("FleaCam failed to start image capture");
      }
    }

    void getFrame(cv::Mat &dest) {
      error = camera.RetrieveBuffer(&rawImage);
      if (error != FlyCapture2::PGRERROR_OK) {
        throw CameraException("FleaCam capture error");
      }
      rawImage.Convert(FlyCapture2::PIXEL_FORMAT_BGR, &rgbImage);
      rowBytes = (double)rgbImage.GetReceivedDataSize() / (double)rgbImage.GetRows();
      dest = cv::Mat(rgbImage.GetRows(), rgbImage.GetCols(), CV_8UC3, rgbImage.GetData(), rowBytes);
      return;
    }

    ~FleaCam() {
        camera.StopCapture();
        camera.Disconnect();
    }
};

class BuiltInCam : public Camera {
  private:
    cv::VideoCapture cap;
  public:
    BuiltInCam() : cap(0) {
        if (!cap.isOpened()) {
            throw CameraException("Failed to acquire built in camera");
        }
    }

    void getFrame(cv::Mat &dest) {
        if (!cap.read(dest)) {
            throw CameraException("Failed to read frame from built in camera");
        }
        return;
    }

    ~BuiltInCam() {
        cap.release();
    }
};

Camera * getCamera(bool useBuiltIn) {
    if (useBuiltIn) {
        return new BuiltInCam();
    } else {
        try {
            return new FleaCam();
        } catch (const CameraException &e) {
            cerr << e.what() << endl;
            cerr << "Falling back on built-in camera." << endl;
            return new BuiltInCam();
        }
    }
}