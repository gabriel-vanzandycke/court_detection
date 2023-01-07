#include <stdlib.h>
#include <unistd.h>
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

    std::string filename = "../assets/image.raw";

    // Shut GetOpt error messages down (return '?'):
    opterr = 0;

    // Retrieve options:
    int opt;
    while ( (opt = getopt(argc, argv, "vh")) != -1 ) {
        switch ( opt ) {
            case 'v':
                debug = true;
                break;
            case 'h':
                std::cout << "Usage: " << argv[0] << " [-v] [-h]" << std::endl;
                std::cout << "Options:" << std::endl;
                std::cout << "  -v: verbose mode (debug)" << std::endl;
                std::cout << "  -h: print this help" << std::endl;
                return 0;
            case '?':
                std::cerr << "Unknown option: '" << char(optopt) << "'!" << std::endl;
                break;
        }
    }

    // Load image data
    cv::Mat image = cv::Mat(nImageSizeY, nImageSizeX, CV_8UC1);
    FILE *fp = fopen(filename.c_str(), "rb");
    if (fp)
    {
        std::fread(image.data, nImageSizeX * nImageSizeY, 1, fp);
        fclose(fp);
    }
    else
    {
        std::cerr << "Error: could not open '" << filename << "'\n";
        return 1;
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
    write_line("netline.csv", calib, court.netline(), steps, &canvas);
    write_line("baseline.csv", calib, court.baseline(), steps, &canvas);
    write_line("serveline.csv", calib, court.serveline(), steps, &canvas);
    write_line("centerline.csv", calib, court.centerline(), steps, &canvas);
    write_line("left_sideline.csv", calib, court.left_sideline(), steps, &canvas);
    write_line("right_sideline.csv", calib, court.right_sideline(), steps, &canvas);
    write_line("left_single_sideline.csv", calib, court.left_single_sideline(), steps, &canvas);
    write_line("right_single_sideline.csv", calib, court.right_single_sideline(), steps, &canvas);
    if (debug) {cv::imshow("lines sampled", canvas); cv::waitKey(0);}

    return 0;
}
