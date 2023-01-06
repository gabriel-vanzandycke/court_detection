#include <math.h>

#include "utils.hpp"

std::string dummy()
{
    return "auie";
}

LineSegment::LineSegment(double x1, double y1, double x2, double y2):
    x1(x1), y1(y1), x2(x2), y2(y2)
{
    this->theta = M_PI - atan2(x2 - x1, y2 - y1);
    this->rho = x1 * cos(this->theta) + y1 * sin(this->theta);
    if (this->rho < 0)
    {
        this->rho = -this->rho;
        this->theta = this->theta - M_PI;
    }
    this->orientation = (this->theta > -M_PI_4 && this->theta < M_PI_4) ? vertical : horizontal;
    this->length = sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2));
}

double LineSegment::distance_to(Eigen::Vector2d point)
{
    Eigen::Vector2d closest = closest_point(this->rho, this->theta, point);
    return (point - closest).norm();
}

Eigen::Vector2d LineSegment::intersect_with(LineSegment line)
{
    /* Find the intersection point of two lines
    */
    double rho1 = this->rho, theta1 = this->theta;
    double rho2 = line.rho, theta2 = line.theta;
    double x = (rho2*sin(theta1) - rho1*sin(theta2)) / (cos(theta2)*sin(theta1) - cos(theta1)*sin(theta2));
    double y = (rho2*cos(theta1) - rho1*cos(theta2)) / (sin(theta2)*cos(theta1) - sin(theta1)*cos(theta2));
    return {x, y};
}

Eigen::Vector2d closest_point(double rho, double theta, Eigen::Vector2d point)
{
    /* Find the closest point on a line to a point
       Line is defined in Hough space by `rho` and `theta`
       Point is defined by `x` and `y` in carthesian coordinates
    */
    Eigen::Vector2d b = {rho*cos(theta), rho*sin(theta)};
    Eigen::Vector2d a = {point.x(), point.y()};
    double alpha = theta - atan2(a.y(), a.x()); // angle between `a` and `b`
    return b + a - a.norm()*cos(alpha)*b/b.norm();
}


void draw_line(LineSegment line, cv::Mat &output, cv::viz::Color color, int thickness, int markersize, std::string label)
{
    cv::line(output, cv::Point(line.x1, line.y1), cv::Point(line.x2, line.y2), color, thickness);
    cv::circle(output, cv::Point(line.x1, line.y1), markersize, color, -1);
    cv::circle(output, cv::Point(line.x2, line.y2), markersize, color, -1);
    int x = (line.x1 + line.x2)/2, y = (line.y1 + line.y2)/2;
    cv::putText(output, label, cv::Point(x, y), cv::FONT_HERSHEY_SIMPLEX, 1, color, thickness);
}
