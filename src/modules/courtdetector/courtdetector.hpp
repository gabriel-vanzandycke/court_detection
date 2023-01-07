#pragma once

#include <utils.hpp>
#include <opencv2/opencv.hpp>
#include "operations.hpp"

/**
 * @brief Module responsible to detect tennis court in the given input image
 * with a series of operations. The operator() returns a Calib object that
 * correspond to input image calibration.
 * @param court Court object representing the current tenis cour to detect.
 * @param image_size Size of the input image
 * @param debug If true, the module will show intermediate results.
*/
class CourtDetector {
    public:
        CourtDetector(Court court, cv::Size image_size, bool debug=false);
        Calib operator()(cv::Mat& input_image);
    private:
        cv::Size image_size;
        bool debug;
        Skeletonize skeletonize;
        RemoveSmallComponents remove_small_components;
        FindSegments find_segments;
        ClusterSegments cluster_segments;
        IdentifyLines identify_lines;
        ComputeHomography compute_homography;
};
