#include <string>
#include <iostream>
#include <opencv2/core/mat.hpp>
#include <Eigen/Dense>

#include <utils.hpp>

#include "court_detection.hpp"

CourtDetection::CourtDetection(std::string court_type){
    // build list of operations
    // build court model

    // this->operations = {
    //     Skeletonize(),
    //     RemoveSmallComponents(50),
    //     FindSegments(1, 1, 10, 100, 100),
    //     Blur(5)
    // };
}


cv::Mat CourtDetection::operator()(cv::Mat& input_image) {
    // skeletonize
    cv::Mat output;
    cv::cvtColor(input_image, output, cv::COLOR_GRAY2RGB);
    cv::Mat skeletonized = Skeletonize()(input_image, output);

    // remove small connected components
    cv::cvtColor(input_image, output, cv::COLOR_GRAY2RGB);
    cv::Mat cleaned = RemoveSmallComponents(50)(skeletonized, output);

    // find segments
    cv::cvtColor(input_image, output, cv::COLOR_GRAY2RGB);
    std::vector<LineSegment> segments = FindSegments(1, 1, 10, 100, 100)(cleaned, output);

    // Cluster segments
    cv::cvtColor(input_image, output, cv::COLOR_GRAY2RGB);
    std::vector<LineSegment> lines = SegmentsClusterer(50, 5)(segments, output);

    // Label lines
    cv::cvtColor(input_image, output, cv::COLOR_GRAY2RGB);
    std::vector<LineSegment> labeled_lines = IdentifyLines(20)(lines, output);

    // Compute homography
    cv::cvtColor(input_image, output, cv::COLOR_GRAY2RGB);
    cv::Mat homography = ComputeHomography("ITF", input_image.size())(labeled_lines, output);

    return output;
}
