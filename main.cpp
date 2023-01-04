#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <cstddef>
#include <cstdint>

#include <utils.hpp>
#include <image_processing.hpp>
#include <court_detection.hpp>
#include <opencv2/opencv.hpp>

int main(int argc, char *argv[]){
    int nImageSizeX = 1392;
    int nImageSizeY = 550;

    cv::Mat image(nImageSizeY, nImageSizeX, CV_8UC1);

    // Load image data
    FILE *fp = fopen("assets/image.raw", "rb");
    if (fp) {
        std::fread(image.data, nImageSizeX * nImageSizeY, 1, fp);
        fclose(fp);
    }

    // Create court detection module
    CourtDetection courtDetection("ITF");


    delete [] buffer;

    Blur blur(5);
    std::cout << dummy() << blur.kernel << std::endl;
    return 0;
}
