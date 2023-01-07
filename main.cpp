#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <cstddef>
#include <cstdint>

#include <utils.hpp>
#include <courtdetector.hpp>
#include <opencv2/opencv.hpp>

int main(int argc, char *argv[]){
    int nImageSizeX = 1392;
    int nImageSizeY = 550;

    // Load image data
    cv::Mat image = cv::Mat(nImageSizeY, nImageSizeX, CV_8UC1);
    FILE *fp = fopen("../assets/image.raw", "rb");
    if (fp) {
        std::fread(image.data, nImageSizeX * nImageSizeY, 1, fp);
        fclose(fp);
    }

    // Create court detection module
    CourtDetector courtdetector("ITF", image.size());

    // Run court detection with current image
    Calib calib = courtdetector(image);

    //cv::imshow("img", output); cv::waitKey();

    return 0;
}
