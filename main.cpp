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
    bool debug = false;

    // Load image data
    cv::Mat image = cv::Mat(nImageSizeY, nImageSizeX, CV_8UC1);
    FILE *fp = fopen("../assets/image.raw", "rb");
    if (fp) {
        std::fread(image.data, nImageSizeX * nImageSizeY, 1, fp);
        fclose(fp);
    }

    // Create court detection module
    Court court("ITF");
    CourtDetector courtdetector(court, image.size(), debug);

    // Run court detection with current image
    Calib calib = courtdetector(image);

    // Traverse lines
    cv::Mat canvas;
    cv::Mat *canvas_ptr = debug ? &canvas : nullptr;
    if (debug) {cv::cvtColor(image, canvas, cv::COLOR_GRAY2RGB);}
    int steps = 10;
    write_line("netline", calib, court.netline(), steps, &canvas);
    write_line("baseline", calib, court.baseline(), steps, &canvas);
    write_line("serveline", calib, court.serveline(), steps, &canvas);
    write_line("centerline", calib, court.centerline(), steps, &canvas);
    write_line("left_sideline", calib, court.left_sideline(), steps, &canvas);
    write_line("right_sideline", calib, court.right_sideline(), steps, &canvas);
    write_line("left_single_sideline", calib, court.left_single_sideline(), steps, &canvas);
    write_line("right_single_sideline", calib, court.right_single_sideline(), steps, &canvas);
    if (debug) {cv::imshow("lines sampled", canvas); cv::waitKey(0);}

    return 0;
}
