#pragma once

#include <utils.hpp>
#include <opencv2/opencv.hpp>
#include "operations.hpp"

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
