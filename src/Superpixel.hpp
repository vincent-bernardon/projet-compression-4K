#ifndef SUPERPIXEL_HPP
#define SUPERPIXEL_HPP

#include <opencv2/opencv.hpp>

class Superpixel{
    public:
        cv::Vec3b rgb;
        int x;
        int y;
        int nbPixels;

        float getDistance(int _S, int m, const cv::Vec3b* _color, int _x, int _y);

};

#endif