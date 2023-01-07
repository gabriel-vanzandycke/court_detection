#pragma once

#include <utils.hpp>
#include <court.hpp>


class Skeletonize
{
    public:
        Skeletonize();
        cv::Mat operator()(cv::Mat input_image, cv::Mat &debug_image);
    private:
};

class RemoveSmallComponents
{
    public:
        RemoveSmallComponents(int max_area);
        cv::Mat operator()(cv::Mat input_image, cv::Mat &debug_image);
    private:
        int max_area;
};

// FindSegments finds segments in an image using the probabilistic Hough transform.
// The input image is assumed to be a binary image, and the output image is a debug image
// that shows the input image with the found segments overlaid.
// The segments are returned in the segments vector.
class FindSegments
{
    public:
        FindSegments(float distance_step, float angle_step, int threshold, int min_line_length, int max_line_gap);
        std::vector<LineSegment> operator()(cv::Mat input_image, cv::Mat &debug_image);
    private:
        float distance_step;
        float angle_step;
        int threshold;
        int min_line_length;
        int max_line_gap;
};

class ClusterSegments
{
    public:
        ClusterSegments(float rho_threshold, float theta_threshold);
        std::vector<LineSegment> operator()(std::vector<LineSegment> segments, cv::Mat &debug_image);
    private:
        float rho_threshold;
        float theta_threshold;
};

class IdentifyLines
{
    public:
        IdentifyLines(int distance_threshold);
        std::vector<LineSegment> operator()(std::vector<LineSegment> lines, cv::Mat &debug_image);
    private:
        int distance_threshold;
};

class ComputeHomography
{
    public:
        ComputeHomography(std::string court_type, cv::Size image_size);
        Calib operator()(std::vector<LineSegment> lines, cv::Mat &debug_image);
    private:
        Court court;
        cv::Size image_size;
};