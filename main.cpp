#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <iostream>
#include <cstddef>
#include <cstdint>

#include <utils.hpp>
#include <courtdetector.hpp>
#include <opencv2/opencv.hpp>
#include <boost/program_options.hpp>


int main(int argc, char *argv[])
{
    int nImageSizeX = 1392;
    int nImageSizeY = 550;
    bool debug = false;
    std::string filename;
    std::string rule_type = "ITF";
    int steps = 10;

    try
    {
        // Declare the supported options.
        boost::program_options::options_description desc("Options");
        desc.add_options()
            ("help", "produce help message")
            ("debug", "enable debug mode")
            ("filename", boost::program_options::value<std::string>(), "Input image filename (REQUIRED): a file containing the raw image bytes.")
            ("width", boost::program_options::value<int>(), "Input image width (required to decode raw image)")
            ("height", boost::program_options::value<int>(), "Input image height (required to decode raw image)")
            ("rule-type", boost::program_options::value<std::string>(), "Rule type describing the tennis court (REQUIRED): currently only 'ITF' is supported.")
            ("steps", boost::program_options::value<int>(), "Number of steps to use when discretizing the tennis court (default: 10).")
        ;

        boost::program_options::variables_map vm;
        boost::program_options::store(boost::program_options::parse_command_line(argc, argv, desc), vm);
        boost::program_options::notify(vm);

        if (vm.count("help"))
        {
            std::cout << desc << std::endl;
            return 0;
        }
        debug = vm.count("debug");

        if (vm.count("filename"))
        {
            std::cout << "Reading from " << vm["filename"].as<std::string>() << ".\n";
            filename = vm["filename"].as<std::string>();
        }
        else
        {
            std::cerr << "Error: no input filename specified. " << desc << std::endl;
            return 1;
        }

        if (vm.count("width"))
        {
            std::cout << "Image width is " << vm["width"].as<int>() << ".\n";
            nImageSizeX = vm["width"].as<int>();
        }
        else
        {
            std::cerr << "Warning: no image width specified. Using default width of " << nImageSizeX << " pixels" << std::endl;
        }

        if (vm.count("height"))
        {
            std::cout << "Image height is " << vm["height"].as<int>() << ".\n";
            nImageSizeY = vm["height"].as<int>();
        }
        else
        {
            std::cerr << "Warning: no image height specified. Using default height of " << nImageSizeY << " pixels" << std::endl;
        }

        if (vm.count("rule-type"))
        {
            std::cout << "Rule type is " << vm["rule-type"].as<std::string>() << ".\n";
            rule_type = vm["rule-type"].as<std::string>();
        }
        else
        {
            std::cerr << "Warring: no rule type specified. Using default " << rule_type << desc << std::endl;
        }

        if (vm.count("steps"))
        {
            std::cout << "Number of steps is " << vm["steps"].as<int>() << ".\n";
            steps = vm["steps"].as<int>();
        }
        else
        {
            std::cerr << "Warning: no number of steps specified. Using default " << steps << std::endl;
        }

    }
    catch(std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    catch(...)
    {
        std::cerr << "Error: unknown exception" << std::endl;
        return 1;
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
    Court court(rule_type);
    CourtDetector courtdetector(court, image.size(), debug);

    // Run court detection with current image
    Calib calib = courtdetector(image);

    // Traverse lines
    cv::Mat canvas;
    cv::Mat *canvas_ptr = debug ? &canvas : nullptr;
    if (debug) {cv::cvtColor(image, canvas, cv::COLOR_GRAY2RGB);}
    write_line("netline.csv", calib, court.netline(), steps, &canvas);
    write_line("baseline.csv", calib, court.baseline(), steps, &canvas);
    write_line("serveline.csv", calib, court.serveline(), steps, &canvas);
    write_line("centerline.csv", calib, court.centerline(), steps, &canvas);
    write_line("left_sideline.csv", calib, court.left_sideline(), steps, &canvas);
    write_line("right_sideline.csv", calib, court.right_sideline(), steps, &canvas);
    write_line("left_single_sideline.csv", calib, court.left_single_sideline(), steps, &canvas);
    write_line("right_single_sideline.csv", calib, court.right_single_sideline(), steps, &canvas);
    if (debug) {cv::imshow("lines sampled", canvas); cv::waitKey(0);}

    std::cout << "Image projection matrix P:" << std::endl;
    std::cout << calib.P << std::endl;

    return 0;
}
