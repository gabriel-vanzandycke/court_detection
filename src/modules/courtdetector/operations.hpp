#pragma once

#include <utils.hpp>
#include <court.hpp>


/**
 * @brief Perform a Thinning operation.
*/
class Skeletonize
{
    public:
        Skeletonize();
        /**
         * @brief performs the operation.
         * @param input_image: input image to be skeletonized.
         * @param debug_image: if not null, a visualization of the operation is
         * drawn on this image.
         * @return a binary image with the skeleton of pixel blos from the input
         * image.
        */
        cv::Mat operator()(cv::Mat input_image, cv::Mat *debug_image=nullptr);
    private:
};


/**
 * @brief Removes small connected components from a binary image.
 * @param max_area: maximum area (in pixels) for a connected component to be
 * considered.
*/
class RemoveSmallComponents
{
    public:
        RemoveSmallComponents(int max_area);
        /**
         * @brief performs the operation (in place).
         * @param input_image: binary image in which small connected components
         * are removed.
         * @param debug_image: if not null, a visualization of the operation is
         * drawn on this image.
         * @return the input binary image cleaned.
        */
        cv::Mat operator()(cv::Mat input_image, cv::Mat *debug_image=nullptr);
    private:
        int max_area;
};


/**
 * @brief Finds line segments in a binary image with the Hough Line detector.
 * @param distance_step: distance step for the Hough Line detector (in pixels).
 * @param angle_step: angle step for the Hough Line detector (in degrees).
 * @param threshold: threshold for the Hough Line detector.
 * @param min_line_length: minimum length for a line segment to be considered.
 * @param max_line_gap: maximum gap between two segments parts to be considered
 * as a single line segment.
*/
class FindSegments
{
    public:
        FindSegments(float distance_step, float angle_step, int threshold, int min_line_length, int max_line_gap);
        /**
         * @brief performs the operation.
         * @param input_image: binary image in which line segments are searched.
         * @param debug_image: if not null, a visualization of the operation is
         * drawn on this image.
         * @return line segments found in the image.
        */
        std::vector<LineSegment> operator()(cv::Mat input_image, cv::Mat *debug_image=nullptr);
    private:
        float distance_step;
        float angle_step;
        int threshold;
        int min_line_length;
        int max_line_gap;
};


/**
 * @brief Clusters colinear line segments.
 * @param rho_threshold: maximum difference of distances to the origin for two
 * line segments to be considered colinear.
 * @param theta_threshold: maximum difference of angles relative to the y=0 axis
 * for two line segments to be considered colinear.
*/
class ClusterSegments
{
    public:
        ClusterSegments(float rho_threshold, float theta_threshold);
        /**
         * @brief performs the operation
         * @param segments: line segments found in the image
         * @param debug_image: if not null, a visualization of the operation is
         * drawn on this image.
         * @return new line segments by clustering colinear input line segments.
        */
        std::vector<LineSegment> operator()(std::vector<LineSegment> segments, cv::Mat *debug_image=nullptr);
    private:
        float rho_threshold;
        float theta_threshold;
};

/**
 * @brief Identifies the lines necessary for performing the court homography
 * step. The serveline is first identified by finding the shortest horizontal
 * line. Then the two single sidelines are identified by finding the two
 * vertical lines being close to the serve line extremities. The baseline is the
 * horizontal line below the serveline. The centerline is the vertical line
 * whose extremity is the closest to the center of the serveline.
 * @param distance_threshold: maximum distance between lines and the serveline
 * endpoints to identify the two single sidelines; and maximum distance between
 * lines and the serveline midpoint to identify the centerline.
*/
class IdentifyLines
{
    public:
        IdentifyLines(int distance_threshold);
        /**
         * @brief performs the operation
         * @param lines: lines found in the image
         * @param debug_image: if not null, a visualization of the operation is
         * drawn on this image.
         * @return the lines necessary for performing the court homography step:
         * serveline, baseline, left_single_sideline, right_single_sideline and
         * centerline.
        */
        std::vector<LineSegment> operator()(std::vector<LineSegment> lines, cv::Mat *debug_image=nullptr);
    private:
        int distance_threshold;
};

/**
 * @brief Computes the homography matrix that maps a tennis court to the given
 * image.
 * @param court: tennis court definition
 * @param image_size: size of the image
*/
class ComputeHomography
{
    public:
        ComputeHomography(Court court, cv::Size image_size);
        /**
         * @brief performs the operation
         * @param lines: necessary lines found in the image: serveline,
         * baseline, left_single_sideline, right_single_sideline and centerline.
         * @param debug_image: if not null, a visualization of the operation is
         * drawn on this image.
         * @return the calibration parameters.
        */
        Calib operator()(std::vector<LineSegment> lines, cv::Mat *debug_image=nullptr);
    private:
        Court court;
        cv::Size image_size;
};
