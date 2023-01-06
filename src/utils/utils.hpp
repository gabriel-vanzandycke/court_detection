#pragma once

#include <string>

#include <opencv2/ximgproc.hpp>
#include <opencv2/viz/types.hpp>
#include <Eigen/Dense>

std::string dummy();

#define PRINT(x)  std::cout << (x) << std::endl << std::endl


std::vector<cv::viz::Color> colors = {
    cv::viz::Color::cyan(),
    cv::viz::Color::green(),
    cv::viz::Color::red(),
    cv::viz::Color::pink(),
    cv::viz::Color::yellow(),
    cv::viz::Color::blue(),
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
    cv::viz::Color::orange(),
    cv::viz::Color::purple(),
    cv::viz::Color::raspberry(),
    cv::viz::Color::rose(),
    cv::viz::Color::teal(),
    cv::viz::Color::turquoise(),
    cv::viz::Color::violet(),
};

enum LineOrientation { horizontal, vertical };

Eigen::Vector2d closest_point(double rho, double theta, Eigen::Vector2d point);

class LineSegment
{
    public:
        LineSegment(double x1, double y1, double x2, double y2);
        double distance_to(Eigen::Vector2d point);
        Eigen::Vector2d intersect_with(LineSegment line);
        double x1, y1, x2, y2;
        double rho;
        double theta;
        double length;
        LineOrientation orientation;
};

void draw_line(LineSegment line, cv::Mat &output, cv::viz::Color color, int thickness=2, int markersize=5, std::string label="");