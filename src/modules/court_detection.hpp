#pragma once

#include <utils.hpp>
#include <opencv2/opencv.hpp>
#include <image_processing.hpp>

class CourtDetection {
    public:
        CourtDetection(std::string court_type);
        cv::Mat operator()(cv::Mat& image);
        int kernel;
    private:
};
