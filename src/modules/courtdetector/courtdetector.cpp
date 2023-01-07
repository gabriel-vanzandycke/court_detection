#include <string>
#include <iostream>
#include <opencv2/core/mat.hpp>

#include <utils.hpp>

#include "courtdetector.hpp"

CourtDetector::CourtDetector(Court court, cv::Size image_size, bool debug):
    debug(debug),
    image_size(image_size),
    skeletonize(Skeletonize()),
    remove_small_components(RemoveSmallComponents(50)),
    find_segments(FindSegments(1, 1, 10, 100, 100)),
    cluster_segments(ClusterSegments(50, 5)),
    identify_lines(IdentifyLines(20)),
    compute_homography(ComputeHomography(court, image_size))
{}


Calib CourtDetector::operator()(cv::Mat& input_image)
{
    cv::Mat output;
    cv::cvtColor(input_image, output, cv::COLOR_GRAY2RGB);

    // skeletonize
    cv::Mat *debug_image = this->debug ? &output : nullptr;
    cv::Mat skeletonized = this->skeletonize(input_image, debug_image);
    if (this->debug) {cv::imshow("after skeletonized", output); cv::waitKey();}

    // remove small connected components
    cv::cvtColor(input_image, output, cv::COLOR_GRAY2RGB);
    cv::Mat cleaned = this->remove_small_components(skeletonized, debug_image);
    if (this->debug) {cv::imshow("after removing small components", output); cv::waitKey();}

    // find segments
    cv::cvtColor(input_image, output, cv::COLOR_GRAY2RGB);
    std::vector<LineSegment> segments = this->find_segments(cleaned, debug_image);
    if (this->debug) {cv::imshow("after segments detection", output); cv::waitKey();}

    // Cluster segments
    cv::cvtColor(input_image, output, cv::COLOR_GRAY2RGB);
    std::vector<LineSegment> lines = this->cluster_segments(segments, debug_image);
    if (this->debug) {cv::imshow("after segments clustering", output); cv::waitKey();}

    // Identify lines
    cv::cvtColor(input_image, output, cv::COLOR_GRAY2RGB);
    std::vector<LineSegment> labeled_lines = this->identify_lines(lines, debug_image);
    if (this->debug) {cv::imshow("after lines identification", output); cv::waitKey();}

    // Compute homography
    cv::cvtColor(input_image, output, cv::COLOR_GRAY2RGB);
    Calib calib = this->compute_homography(labeled_lines, debug_image);
    if (this->debug) {cv::imshow("after projection", output); cv::waitKey();}

    return calib;
}
