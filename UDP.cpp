#include <stdio.h>
#include <exception>
#include <stdexcept>

#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>

#include <opencv2/core/utility.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace std;

class UDPException : public runtime_error {
  public:
    UDPException(const string& message) : runtime_error(message) {}
};

class UDPSender {
  private:
    int handle;
    unsigned int address;
    unsigned short port;
    char packet_data[23];
    sockaddr_in addr;
    cv::Mat tinyImage;
  public:
    UDPSender() {
        handle = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (handle <= 0) {
            throw UDPException("Failed to create socket");
        }
        unsigned int a = 127;
        unsigned int b = 0;
        unsigned int c = 0;
        unsigned int d = 1;
        address = (a << 24) | (b << 16) | (c << 8) | d;
        port = 6668;
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = htonl(address);
        addr.sin_port = htons(port);
    }

    void sendColorPixels(cv::Mat &mat) {
        cv::resize(mat, this->tinyImage, cv::Size(0, 0), 0.25, 0.25);
        int rows = tinyImage.rows;
        int cols = tinyImage.cols;
        int channels = tinyImage.channels();
        unsigned char b;
        unsigned char g;
        unsigned char r;
        unsigned int pixel_location;
        int packet_size;
        int sent_bytes;
        for (int y = 0; y < rows; y++) {
            for (int x = 0; x < cols; x++) {
                pixel_location = channels * (cols * y + x);
                b = tinyImage.data[pixel_location];
                g = tinyImage.data[pixel_location + 1];
                r = tinyImage.data[pixel_location + 2];
                if ((int)b == 0 && (int)g == 0 && (int)r == 0) {
                    continue;
                }
                sprintf(
                    this->packet_data,
                    "set %d %d %d %d %d",
                    x, y, (int)r, (int)g, (int)b
                );
                packet_size = strlen(this->packet_data);
                sent_bytes = sendto(
                    this->handle,
                    (const char*)this->packet_data,
                    packet_size,
                    0,
                    (sockaddr*)&(this->addr),
                    sizeof(sockaddr_in)
                );
                if (sent_bytes != packet_size) {
                    cerr << "Failed to send packet!" << endl;
                }
            }
        }
    }
};
