#include "Superpixel.h"
#include <iostream>

float Superpixel::getDistance(int _S, int m, const cv::Vec3b* _color, int _x, int _y) {
    float d_rgb = cv::norm(cv::Vec3f(rgb) - cv::Vec3f(*_color));
    float d_xy = sqrt(pow(x - _x, 2) + pow(y - _y, 2));
    float ratio = (float)m/_S;
    return d_rgb + ratio * d_xy;
}

// https://hajim.rochester.edu/ece/sites/gsharma/papers/CIEDE2000CRNAFeb05.pdf
float getDistanceLAB(const cv::Vec3f & A, const cv::Vec3f & B, float kL = 1.0f, float kC = 1.0f, float kH = 1.0f){
    const float deg2rad = M_PI / 180.0f;
    const float rad2deg = 180.0f / M_PI;

    // CALCUL C et h prime A et B,
    float C_A = sqrt((A[1] * A[1])+ (A[2] * A[2]));
    float C_B = sqrt((B[1] * B[1])+ (B[2] * B[2]));

    float CB = (C_A + C_B) * 0.5;
    float CB_p7 = pow(CB,7);

    float G = 0.5f*(1 - sqrt( CB_p7 / ( CB_p7+ pow(25,7) )) );
    //std::cout << "G= " << G << std::endl;

    float aP_A = (1.0f+G) * A[1];
    float aP_B = (1.0f+G) * B[1];

    float CP_A = sqrt((aP_A * aP_A)+ (A[2] * A[2]));
    float CP_B = sqrt((aP_B * aP_B)+ (B[2] * B[2]));

    //std::cout << "C'a = " << CP_A << std::endl;
    //std::cout << "C'b = " << CP_B << std::endl;

    float hp_A = (aP_A == 0 && A[2] == 0) ? 0 : atan2(A[2], aP_A) * rad2deg;
    float hp_B = (aP_B == 0 && B[2] == 0) ? 0 : atan2(B[2], aP_B) * rad2deg;

    // Convert to [0, 360] range parce que atan [-180, 180]
    if (hp_A < 0) hp_A += 360.0f;
    if (hp_B < 0) hp_B += 360.0f;

    //std::cout << "h'a = " << hp_A << std::endl;
    //std::cout << "h'b = " << hp_B << std::endl;

    // CALCUL DELTA LP CP HP
    float D_LP = B[0] - A[0];

    float D_CP = CP_B - CP_A;

    float D_hP;

    float diff_H = hp_B - hp_A;
    float abs_diff_H = abs(diff_H);

    if(CP_A*CP_B == 0) D_hP = 0;
    else {
        if ( abs_diff_H <= 180 ) D_hP = diff_H;
        else if (diff_H > 180) D_hP = diff_H - 360;
        else if (diff_H < -180) D_hP = diff_H + 360;
        else std::cout<< "erreur calcule de D_hP";
    }

    float D_HP = 2 * sqrt(CP_A * CP_B) * sin((D_hP * deg2rad) * 0.5) ;

    float sommeH = hp_A + hp_B;

    // CALCUL D_E00
    float LP = (A[0] + B[0]) * 0.5f;
    float CP = (CP_A + CP_B) * 0.5f;

    float hP;
    if (CP_A*CP_B == 0) hP = 0;
    else {
        if (CP_A*CP_B != 0 && abs_diff_H <= 180) hP = sommeH * 0.5f;
        else if( abs_diff_H > 180 &&  sommeH < 360) hP = (sommeH + 360) * 0.5f;
        else if (abs_diff_H > 180 &&  sommeH >= 360) hP = (sommeH - 360) * 0.5f;
        else std::cout<< "erreur calcule de D_hP";
    }

    float T = 1
              - 0.17f * cos((hP - 30) * deg2rad)
              + 0.24f * cos(2 * hP * deg2rad)
              + 0.32f * cos((3 * hP + 6) * deg2rad)
              - 0.20f * cos((4 * hP - 63) * deg2rad);

    //std::cout << "T = " << T << std::endl;

    float exp_truc = -pow((hP - 275)/25, 2);
    float D_PHI = 30 * exp(exp_truc);

    float CP_POW7 = pow(CP, 7);
    float RC = 2 * sqrt(CP_POW7 / ( CP_POW7 + pow(25,7) ));

    float LP_50_POW2 = pow((LP - 50),2);
    float SL = 1 + ( 0.015 * LP_50_POW2 ) / ( sqrt(20 + LP_50_POW2) );
    //std::cout << "Sl = " << SL << std::endl;
    float SC = 1 + 0.045 * CP;
    //std::cout << "Sc = " << SC << std::endl;
    float SH = 1 + 0.015 * CP * T;
    //std::cout << "Sh = " << SH << std::endl;
    float RT = -sin(2 * D_PHI * deg2rad) * RC;
    //std::cout << "Rt = " << RT << std::endl;


    float R1 = D_LP / (SL * kL);
    float R2 = D_CP / (SC * kC);
    float R3 = D_HP / (SH * kH);

    float PLUS = RT * R2 * R3;

    return sqrt( R1 * R1 + R2 * R2 + R3 * R3 + PLUS);

}

// les formule https://www.easyrgb.com/en/math.php
void Superpixel::setLAB(){
    // RGB
    float var_r = rgb[0] / 255.0f;
    float var_g = rgb[1] / 255.0f;
    float var_b = rgb[2] / 255.0f;

    var_r = (var_r > 0.04045f) ? pow((var_r + 0.055f) / 1.055f, 2.4f) : var_r / 12.92f;
    var_g = (var_g > 0.04045f) ? pow((var_g + 0.055f) / 1.055f, 2.4f) : var_g / 12.92f;
    var_b = (var_b > 0.04045f) ? pow((var_b + 0.055f) / 1.055f, 2.4f) : var_b / 12.92f;

    var_r *= 100.0f;
    var_g *= 100.0f;
    var_b *= 100.0f;

    // XYZ
    float x = 0.4124f * var_r + 0.3576f * var_g + 0.1805f * var_b;
    float y = 0.2126f * var_r + 0.7152f * var_g + 0.0722f * var_b;
    float z = 0.0193f * var_r + 0.1192f * var_g + 0.9505f * var_b;

    float xr = 95.047f;
    float yr = 100.0f;
    float zr = 108.883f;

    float var_x = x / xr;
    float var_y = y / yr;
    float var_z = z / zr;

    var_x = var_x > 0.008856f ? std::pow(var_x, 1.0f/3.0f) : (7.787f * var_x) + (16.0f/116.0f);
    var_y = var_y > 0.008856f ? std::pow(var_y, 1.0f/3.0f) : (7.787f * var_y) + (16.0f/116.0f);
    var_z = var_z > 0.008856f ? std::pow(var_z, 1.0f/3.0f) : (7.787f * var_z) + (16.0f/116.0f);

    // LAB
    float L = 116.0f * var_y - 16.0f;
    float a = 500.0f * (var_x - var_y);
    float B = 200.0f * (var_y - var_z);

    lab[0] = L;
    lab[1] = a;
    lab[2] = B;
}


void Superpixel::setRGBfromLAB(){
    float xr = 95.047;
    float yr = 100.0;
    float zr = 108.883;

    // cielab xyz
    float var_Y = (lab[0] + 16) / 116;
    float var_X = lab[1] / 500 + var_Y;
    float var_Z = var_Y - lab[2] / 200;

    if (pow(var_Y, 3) > 0.008856) var_Y = pow(var_Y, 3);
    else var_Y = (var_Y - 16.0f / 116.0f) / 7.787f;

    if (pow(var_X, 3) > 0.008856) var_X = pow(var_X, 3);
    else var_X = (var_X - 16.0f / 116.0f) / 7.787f;

    if (pow(var_Z, 3) > 0.008856) var_Z = pow(var_Z, 3);
    else var_Z = (var_Z - 16.0f / 116.0f) / 7.787f;

    float X = var_X * xr;
    float Y = var_Y * yr;
    float Z = var_Z * zr;

    // xyz rgb
    var_X = X / 100;
    var_Y = Y / 100;
    var_Z = Z / 100;

    float var_R = var_X * 3.2406 + var_Y * -1.5372 + var_Z * -0.4986;
    float var_G = var_X * -0.9689 + var_Y * 1.8758 + var_Z * 0.0415;
    float var_B = var_X * 0.0557 + var_Y * -0.2040 + var_Z * 1.0570;

    if (var_R > 0.0031308) var_R = 1.055f * pow(var_R, 1.0f / 2.4f) - 0.055f;
    else var_R = 12.92f * var_R;

    if (var_G > 0.0031308) var_G = 1.055f * pow(var_G, 1.0f / 2.4f) - 0.055f;
    else var_G = 12.92f * var_G;

    if (var_B > 0.0031308) var_B = 1.055f * pow(var_B, 1.0f / 2.4f) - 0.055f;
    else var_B = 12.92f * var_B;

    rgb[0] = static_cast<uchar>(std::max(0.0f, std::min(255.0f, std::round(var_R * 255))));
    rgb[1] = static_cast<uchar>(std::max(0.0f, std::min(255.0f, std::round(var_G * 255))));
    rgb[2] = static_cast<uchar>(std::max(0.0f, std::min(255.0f, std::round(var_B * 255))));
}


void computeLAB(std::vector<Superpixel> & superpixels){
    for (auto & superpixel : superpixels){
        superpixel.setLAB();
    }
}
