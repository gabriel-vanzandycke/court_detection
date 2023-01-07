#pragma once

#include <utils.hpp>
#include <opencv2/opencv.hpp>
#include "operations.hpp"

class CourtDetector {
    public:
        CourtDetector(std::string court_type, cv::Size image_size);
        Calib operator()(cv::Mat& input_image);
    private:
        cv::Size image_size;
};
