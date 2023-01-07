#pragma once

#include <utils.hpp>
#include <opencv2/opencv.hpp>
#include "operations.hpp"

class CourtDetection {
    public:
        CourtDetection(std::string court_type);
        cv::Mat operator()(cv::Mat& input_image);
    private:
};
