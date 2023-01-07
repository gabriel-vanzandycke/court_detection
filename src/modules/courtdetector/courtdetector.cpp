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
    cv::Mat canvas;
    cv::Mat *canvas_ptr = this->debug ? &canvas : nullptr;

    // skeletonize
    if (this->debug) {cv::cvtColor(input_image, canvas, cv::COLOR_GRAY2RGB);}
    cv::Mat skeletonized = this->skeletonize(input_image, canvas_ptr);
    if (this->debug) {cv::imshow("after skeletonized", canvas); cv::waitKey();}

    // remove small connected components
    if (this->debug) {cv::cvtColor(input_image, canvas, cv::COLOR_GRAY2RGB);}
    cv::Mat cleaned = this->remove_small_components(skeletonized, canvas_ptr);
    if (this->debug) {cv::imshow("after removing small components", canvas); cv::waitKey();}

    // find segments
    if (this->debug) {cv::cvtColor(input_image, canvas, cv::COLOR_GRAY2RGB);}
    std::vector<LineSegment> segments = this->find_segments(cleaned, canvas_ptr);
    if (this->debug) {cv::imshow("after segments detection", canvas); cv::waitKey();}

    // Cluster segments
    if (this->debug) {cv::cvtColor(input_image, canvas, cv::COLOR_GRAY2RGB);}
    std::vector<LineSegment> lines = this->cluster_segments(segments, canvas_ptr);
    if (this->debug) {cv::imshow("after segments clustering", canvas); cv::waitKey();}

    // Identify lines
    if (this->debug) {cv::cvtColor(input_image, canvas, cv::COLOR_GRAY2RGB);}
    std::vector<LineSegment> labeled_lines = this->identify_lines(lines, canvas_ptr);
    if (this->debug) {cv::imshow("after lines identification", canvas); cv::waitKey();}

    // Compute homography
    if (this->debug) {cv::cvtColor(input_image, canvas, cv::COLOR_GRAY2RGB);}
    Calib calib = this->compute_homography(labeled_lines, canvas_ptr);
    if (this->debug) {cv::imshow("after projection", canvas); cv::waitKey();}

    return calib;
}
