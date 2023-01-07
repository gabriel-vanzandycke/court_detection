
#include <iostream>
#include <opencv2/calib3d.hpp>

#include <utils.hpp>
#include <court.hpp>
#include "operations.hpp"



Skeletonize::Skeletonize()
{};

cv::Mat Skeletonize::operator()(cv::Mat input_image, cv::Mat &debug_image)
{
    cv::Mat output_image;
    cv::ximgproc::thinning(input_image, output_image);
    debug_image.setTo(cv::Scalar(255,0,0), output_image);
    return output_image;
};



RemoveSmallComponents::RemoveSmallComponents(int max_area):
    max_area(max_area)
{};

cv::Mat RemoveSmallComponents::operator()(cv::Mat input_image, cv::Mat &debug_image)
{
    cv::Mat labeled_image, stats, centroids, mask;
    int n_labels = connectedComponentsWithStats(input_image, labeled_image, stats, centroids);
    int start_index = 1; // skip background component
    for (int i = start_index; i < n_labels; ++i)
    {
        if (stats.at<int>(i, cv::CC_STAT_AREA) < this->max_area)
        {
            cv::compare(labeled_image, i, mask, cv::CMP_EQ);
            input_image.setTo(0, mask);
        }
    }
    debug_image.setTo(cv::Scalar(255,0,0), input_image);
    return input_image;
};



FindSegments::FindSegments(float distance_step, float angle_step, int threshold, int min_line_length, int max_line_gap):
    distance_step(distance_step), angle_step(angle_step), threshold(threshold), min_line_length(min_line_length), max_line_gap(max_line_gap)
{};

std::vector<LineSegment> FindSegments::operator()(cv::Mat input_image, cv::Mat &debug_image)
{
    // find segments
    std::vector<cv::Vec4i> coordinates;
    cv::HoughLinesP(input_image, coordinates, this->distance_step, this->angle_step*CV_PI/180, this->threshold,
        this->min_line_length, this->max_line_gap);

    int num_segments = coordinates.size();
    std::vector<LineSegment> segments;
    for (size_t i = 0; i < num_segments; i++)
    {
        cv::Vec4i l = coordinates[i];
        LineSegment segment(l[0], l[1], l[2], l[3]);
        segments.push_back(segment);
        // draw segments
        cv::viz::Color color = colors[i % colors.size()];
        std::ostringstream label;
        label << i << " |" << (int)segment.rho << "| " << (int)(segment.theta*180/CV_PI);
        draw_line(segment, debug_image, color, 3, 10, label.str());
    }
    return segments;
};



/**
 * rho_threshold: in pixels
 * theta_threshold: in degrees
*/
SegmentsClusterer::SegmentsClusterer(float rho_threshold, float theta_threshold):
    rho_threshold(rho_threshold), theta_threshold(theta_threshold)
{};

std::vector<LineSegment> SegmentsClusterer::operator()(std::vector<LineSegment> segments, cv::Mat &debug_image)
{
    int num_segments = segments.size();

    // Computing full adjacency matrix is required to link segments that are not directly connected
    Eigen::MatrixXi adjacency = Eigen::MatrixXi::Zero(num_segments, num_segments);
    for (size_t i = 0; i < num_segments; ++i)
    {
        adjacency(i, i) = 1;
        for (size_t j = i+1; j < num_segments; ++j)
        {
            double rho   = std::abs(segments[i].rho - segments[j].rho);
            double theta = std::abs(std::fmod(segments[i].theta - segments[j].theta, CV_PI))*180/CV_PI;
            if (rho < this->rho_threshold && theta < this->theta_threshold)
            {
                adjacency(i, j) = 1;
                adjacency(j, i) = 1;
            }
        }
    }

    // Identify colinear segments
    Eigen::MatrixXi connected = adjacency;
    while (true)
    {
        Eigen::MatrixXi new_connected = connected*adjacency;
        if (connected.cast<bool>() == new_connected.cast<bool>())
            break;
        connected = new_connected;
    }

    // Group colinear segments into lines
    std::vector<LineSegment> lines;
    std::vector<int> indices;
    for(int i = 0; i < connected.rows(); i++)
        indices.push_back(i);
    while (!indices.empty())
    {
        // LineSegment in the form `mx + py = 1` to support vertical segments.
        // Solving linear model A @ [[m],
        //                           [p]] = b
        int n_points = connected.row(indices[0]).count();
        int node_index = indices[0];
        Eigen::MatrixXf A(n_points*2, 2);
        Eigen::MatrixXf b(n_points*2, 1); b.setOnes();
        int k = 0;
        for (size_t i = node_index; i < num_segments; i++)
        {
            if (connected(node_index, i) > 0)
            {
                LineSegment segment = segments[i];
                A.block<2,2>(k, 0) << segment.x1, segment.y1, segment.x2, segment.y2;
                indices.erase(std::remove(indices.begin(), indices.end(), i), indices.end());
                cv::viz::Color color = colors[lines.size()-1 % colors.size()];
                std::ostringstream label;
                label << i << " |" << (int)segment.rho << "| " << (int)(segment.theta*180/CV_PI);
                draw_line(segment, debug_image, color, 3, 10, label.str());
                k = k + 2;
            }
        }

        // Solve linear system
        Eigen::MatrixXf x = A.jacobiSvd(Eigen::ComputeThinU | Eigen::ComputeThinV).solve(b);
        double m = x(0);
        double p = x(1);
        double theta = atan2(p, m);
        double rho = std::abs(p) < std::abs(m) ? std::cos(theta)/m : std::sin(theta)/p;
        int coordinate = std::abs(p) < std::abs(m) ? 1 : 0; // use x if horizontal, y if vertical
        Eigen::Index argmax, argmin;
        A.col(coordinate).maxCoeff(&argmax);
        A.col(coordinate).minCoeff(&argmin);
        cv::Point2f point1 = closest_point(rho, theta, {A(argmin, 0), A(argmin, 1)});
        cv::Point2f point2 = closest_point(rho, theta, {A(argmax, 0), A(argmax, 1)});
        LineSegment line(point1.x, point1.y, point2.x, point2.y);
        lines.push_back(line);
    }
    return lines;
};



IdentifyLines::IdentifyLines(int distance_threshold):
    distance_threshold(distance_threshold)
{};

std::vector<LineSegment> IdentifyLines::operator()(std::vector<LineSegment> lines, cv::Mat &debug_image)
{
    LineSegment *baseline = nullptr;
    LineSegment *serveline = nullptr;
    for (auto &line : lines)
    {
        if (line.orientation == horizontal)
        {
            if (baseline == nullptr || line.length > baseline->length)
            {
                serveline = baseline;
                baseline = &line;
            }
            else
            {
                serveline = &line;
            }
        }
    }

    LineSegment *left_sideline, *right_sideline, *centerline;
    for (auto &line : lines)
    {
        if (line.orientation == vertical)
        {
            if (line.distance_to({serveline->x1, serveline->y1}) < this->distance_threshold)
                left_sideline = &line;
            else if (line.distance_to({serveline->x2, serveline->y2}) < this->distance_threshold)
                right_sideline = &line;
            else if (line.distance_to({(serveline->x1 + serveline->x2)/2, (serveline->y1 + serveline->y2)/2}) < distance_threshold)
                centerline = &line;
        }
    }

    draw_line(*serveline, debug_image, colors[0], 3, 10, std::string("serveline"));
    draw_line(*baseline, debug_image, colors[1], 3, 10, std::string("baseline"));
    draw_line(*left_sideline, debug_image, colors[2], 3, 10, std::string("left_sideline"));
    draw_line(*right_sideline, debug_image, colors[3], 3, 10, std::string("right_sideline"));
    draw_line(*centerline, debug_image, colors[4], 3, 10, std::string("centerline"));

    lines = {*serveline, *baseline, *left_sideline, *right_sideline, *centerline};
    return lines;
}



ComputeHomography::ComputeHomography(std::string court_type, cv::Size image_size):
    court(Court(court_type)),
    image_size(image_size)
{};

cv::Mat ComputeHomography::operator()(std::vector<LineSegment> lines, cv::Mat &debug_image)
{
    LineSegment *serveline = &lines[0];
    LineSegment *baseline = &lines[1];
    LineSegment *left_sideline = &lines[2];
    LineSegment *right_sideline = &lines[3];
    LineSegment *centerline = &lines[4];

    // Find intersections
    cv::Point2f A = serveline->intersect_with(*left_sideline);
    cv::Point2f B = serveline->intersect_with(*right_sideline);
    cv::Point2f C = baseline->intersect_with(*left_sideline);
    cv::Point2f D = baseline->intersect_with(*right_sideline);
    cv::Point2f E = serveline->intersect_with(*centerline);

    std::vector<cv::Point3f> world_keypoints = this->court.keypoints();
    std::vector<cv::Point2f> image_keypoints = {A, B, C, D, E};

    std::vector<std::vector<cv::Point3f>> objectPoints = {world_keypoints};
    std::vector<std::vector<cv::Point2f>> imagePoints = {image_keypoints};

    std::vector<cv::Mat> rvec, tvec;
    cv::Mat distCoefs = cv::Mat::zeros(1, 5, CV_64F);
    cv::Mat camera_matrix = cv::Mat::eye(3, 3, CV_64F);
    int flags = cv::CALIB_FIX_ASPECT_RATIO | cv::CALIB_ZERO_TANGENT_DIST | cv::CALIB_FIX_K1 | cv::CALIB_FIX_K2 | cv::CALIB_FIX_K3;
    cv::calibrateCamera(objectPoints, imagePoints, this->image_size, camera_matrix, distCoefs, rvec, tvec, flags=flags);

    return camera_matrix;
}
