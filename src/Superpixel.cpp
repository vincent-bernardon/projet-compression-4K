#include "Superpixel.hpp"

float Superpixel::getDistance(int _S, int m, const cv::Vec3b* _color, int _x, int _y) {
    float d_rgb = cv::norm(cv::Vec3f(rgb) - cv::Vec3f(*_color));
    float d_xy = sqrt(pow(x - _x, 2) + pow(y - _y, 2));
    float ratio = (float)m/_S;
    return d_rgb + ratio * d_xy;
}