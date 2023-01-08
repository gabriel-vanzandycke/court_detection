#pragma once

#include <string>

#include <opencv2/opencv.hpp>
#include <opencv2/viz/types.hpp>


const std::vector<cv::viz::Color> colors = {
    cv::viz::Color::cyan(),
    cv::viz::Color::green(),
    cv::viz::Color::red(),
    cv::viz::Color::blue(),
    cv::viz::Color::yellow(),
    cv::viz::Color::orange(),
    cv::viz::Color::pink(),
    cv::viz::Color::orange_red(),
    cv::viz::Color::amethyst(),
    cv::viz::Color::apricot(),
    cv::viz::Color::azure(),
    cv::viz::Color::bluberry(),
    cv::viz::Color::brown(),
    cv::viz::Color::celestial_blue(),
    cv::viz::Color::chartreuse(),
    cv::viz::Color::cherry(),
    cv::viz::Color::gold(),
    cv::viz::Color::indigo(),
    cv::viz::Color::lime(),
    cv::viz::Color::magenta(),
    cv::viz::Color::maroon(),
    cv::viz::Color::mlab(),
    cv::viz::Color::navy(),
    cv::viz::Color::olive(),
    cv::viz::Color::purple(),
    cv::viz::Color::raspberry(),
    cv::viz::Color::rose(),
    cv::viz::Color::teal(),
    cv::viz::Color::turquoise(),
    cv::viz::Color::violet(),
};


/**
 * @brief Class representing a calibration
 * @param cameraMatrix Camera intrinsic parameters
 * @param distCoeffs Lens distortion coefficients
 * @param rvec Rotation vector expressed from the world coordinate system to
 * the camera coordinate system with the Rodrigues convention.
 * @param tvec Translation vector expressed in the camera coordinate system.
 * @param image_size Size of the image
*/
class Calib
{
    public:
        Calib(cv::Mat cameraMatrix, cv::Mat distCoeffs, cv::Mat rvec, cv::Mat tvec, cv::Size image_size);
        std::vector<cv::Point2f> project(std::vector<cv::Point3f> point3D);
        cv::Size image_size;
        cv::Mat P;
    private:
        cv::Mat cameraMatrix;
        cv::Mat distCoeffs;
        cv::Mat rvec;
        cv::Mat tvec;
};

enum LineOrientation { horizontal, vertical };


/**
 * @brief Class representing a line segment in 2D space
 * @param x1 x coordinate of one of the segment extremities
 * @param y1 y coordinate of one of the segment extremities
 * @param x2 x coordinate of the other segment extremity
 * @param y2 y coordinate of the other segment extremity
*/
class LineSegment
{
    public:
        LineSegment(float x1, float y1, float x2, float y2);
        float distance_to(cv::Point2f point);
        cv::Point2f intersect_with(LineSegment line);
        float x1, y1, x2, y2;
        float rho;
        float theta;
        float length;
        LineOrientation orientation;
};


/**
 * @brief Finds the closest point on a line to a point. Line is defined in Hough
 * space by `rho` and `theta`
 * @param rho Distance from the origin to the line (in Hough space)
 * @param theta Angle of the line (in Hough space)
 * @param point Point to which the closest point on the line is found
 * @return Closest point on the line to `point`
*/
cv::Point2f closest_point(float rho, float theta, cv::Point2f point);


/**
 * @brief Draw a line in an image, given the 2D coordinates of its extremities
 * @param line Line to draw
 * @param output Image in which the line is drawn
 * @param color Color of the line (RGB)
 * @param thickness Thickness of the line
 * @param markersize Size of the markers at the extremities
 * @param label Label written at the middle of the line
*/
void draw_line(LineSegment line, cv::Mat &output, cv::viz::Color color, int thickness=2, int markersize=5, std::string label="");


/**
 * @brief Draw a line in an image, given the 3D coordinates of its extremities
 * @param calib Current calibration object
 * @param p1 One line extremity
 * @param p2 Other line extremity
 * @param output Image in which the line is drawn
 * @param color Color of the line (RGB)
 * @param thickness Thickness of the line
 * @param markersize Size of the markers at the extremities
 * @param label Label written at the middle of the line
*/
void draw_line_projected(Calib calib, cv::Point3f p1, cv::Point3f p2, cv::Mat &output, cv::viz::Color color, int thickness, int markersize, std::string label);


/**
 * @brief Write point coordinates of a line to a csv file. Points are uniformly
 * sampled along the line.
 * @param filename File in which the line is written
 * @param calib Current calibration object
 * @param line Pair of points defining the line extremities in the 3D world
 * @param steps Number of points to sample along the (3D) line
 * @param debug_image Optional debug image
*/
void write_line(std::string filename, Calib calib, std::vector<cv::Point3f> line, int steps, cv::Mat *debug_image=nullptr);
