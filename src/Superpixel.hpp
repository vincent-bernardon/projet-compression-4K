#ifndef SUPERPIXEL_HPP
#define SUPERPIXEL_HPP

#include <opencv2/opencv.hpp>

class Superpixel{
    public:
        cv::Vec3b rgb;
        cv::Vec3f lab;

        int x;
        int y;
        int nbPixels;

        float getDistance(int _S, int m, const cv::Vec3b* _color, int _x, int _y);

        void setLAB();

        void setRGBfromLAB();
};


float getDistanceLAB(const cv::Vec3f & lab1, const cv::Vec3f & lab2, float k1, float k2, float k3);
void computeLAB(std::vector<Superpixel> & superpixels);

#endif