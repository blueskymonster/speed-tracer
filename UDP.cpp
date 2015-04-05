#include <stdio.h>
#include <stdlib.h>
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
    int current_x;
    int current_y;
    int x_delta;
    int y_delta;
    int wait_time;
    int MAX_X;
    int MAX_Y;
    double image_scale;

    int get_random_velocity() {
        return (rand() % 10 + 1) * 3;
    }

    int time_to_wait() {
        if (rand() % 30 == 0) {
            return rand() % 100 + 20;
        } else {
            return 0;
        }
    }

    void update_velocities(int x_limit, int y_limit) {
        if (this->wait_time > 0) {
            this->wait_time -= 1;
            return;
        } else {
            this->wait_time = this->time_to_wait();
        }
        this->current_x += this->x_delta;
        this->current_y += this->y_delta;

        if (this->current_x <= 0 || this->current_x >= x_limit) {
            this->x_delta = this->get_random_velocity();
            if (this->current_x > 0) {
                this->x_delta *= -1;
                this->current_x = x_limit;
            } else {
                this->current_x = 0;
            }
        }
        if (this->current_y <= 0 || this->current_y >= y_limit) {
            this->y_delta = this->get_random_velocity();
            if (this->current_y > 0) {
                this->y_delta *= -1;
                this->current_y = y_limit;
            } else {
                this->current_y = 0;
            }
        }
    }
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
        current_x = 0;
        current_y = 0;
        x_delta = this->get_random_velocity();
        y_delta = this->get_random_velocity();
        wait_time = 0;
        MAX_X = 1500;
        MAX_Y = 1000;
        image_scale = 1.0;
    }

    void sendColorPixels(cv::Mat &mat) {
        cv::resize(
            mat,
            this->tinyImage,
            cv::Size(0, 0),
            this->image_scale,
            this->image_scale);
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
                    "%d %d %d %d %d",
                    x + this->current_x,
                    y + this->current_y,
                    (int)r, (int)g, (int)b
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
        this->update_velocities(this->MAX_X - cols, this->MAX_Y - rows);
    }
};
