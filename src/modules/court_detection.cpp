#include <string>
#include <iostream>
#include <algorithm>
#include <opencv2/ximgproc.hpp>
#include <Eigen/Dense>

#include <utils.hpp>

#include "court_detection.hpp"

CourtDetection::CourtDetection(std::string court_type){
    // build list of operations
    // build court model
}


cv::Mat CourtDetection::operator()(cv::Mat& image) {
    // skeletonize
    cv::Mat skeleton = image.clone();
    cv::ximgproc::thinning(image, skeleton);

    // remove small connected components
    cv::Mat labeled_image, stats, centroids, mask;
    int n_labels = connectedComponentsWithStats(skeleton, labeled_image, stats, centroids);
    for (int i = 1; i < n_labels; ++i) // skip background component
    {
        if (stats.at<int>(i, cv::CC_STAT_AREA) < 50)
        {
            cv::compare(labeled_image, i, mask, cv::CMP_EQ);
            skeleton.setTo(0, mask);
        }
    }

    // find segments
    std::vector<cv::Vec4i> coordinates;
    float distance_step = 1;
    float angle_step = 1;
    int threshold = 10;
    int min_line_length = 100;
    int max_line_gap = 100;
    cv::HoughLinesP(skeleton, coordinates, distance_step, angle_step*CV_PI/180, threshold, min_line_length, max_line_gap);

    int num_segments = coordinates.size();
    std::vector<LineSegment> segments;
    cv::Mat output;
    cv::cvtColor(image, output, cv::COLOR_GRAY2RGB);

    for (size_t i = 0; i < num_segments; i++)
    {
        cv::Vec4i l = coordinates[i];
        LineSegment segment(l[0], l[1], l[2], l[3]);
        segments.push_back(segment);
        // draw segments
        cv::viz::Color color = colors[i % colors.size()];
        std::ostringstream label;
        label << i << " |" << (int)segment.rho << "| " << (int)(segment.theta*180/CV_PI);
        draw_line(segment, output, color, 3, 10, label.str());
    }

    // Cluster segments
    // Computing full adjacency matrix is required to link segments that are not directly connected
    cv::cvtColor(image, output, cv::COLOR_GRAY2RGB);
    Eigen::MatrixXi adjacency = Eigen::MatrixXi::Zero(num_segments, num_segments);
    double rho_threshold = 50;    // in pixels
    double theta_threshold = 5;   // in degrees
    for (size_t i = 0; i < num_segments; ++i)
    {
        adjacency(i, i) = 1;
        for (size_t j = i+1; j < num_segments; ++j)
        {
            double rho   = std::abs(segments[i].rho - segments[j].rho);
            double theta = std::abs(std::fmod(segments[i].theta - segments[j].theta, CV_PI))*180/CV_PI;
            if (rho < rho_threshold && theta < theta_threshold)
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
        int i = indices[0];
        int n_points = connected.row(i).count();
        Eigen::MatrixXf A(n_points*2, 2);
        Eigen::MatrixXf b(n_points*2, 1); b.setOnes();
        A.block<2,2>(0,0) << segments[i].x1, segments[i].y1, segments[i].x2, segments[i].y2;
        int k = 2;
        for (size_t j = i+1; j < num_segments; j++)
        {
            if (connected(indices[0], j) > 0)
            {
                A.block<2,2>(k, 0) << segments[j].x1, segments[j].y1, segments[j].x2, segments[j].y2;
                indices.erase(std::remove(indices.begin(), indices.end(), j), indices.end());
                k = k + 2;
            }
        }

        // Solve linear model
        Eigen::MatrixXf x = A.jacobiSvd(Eigen::ComputeThinU | Eigen::ComputeThinV).solve(b);
        double m = x(0);
        double p = x(1);
        double theta = atan2(p, m), rho;
        Eigen::Index argmax, argmin;
        if (std::abs(p) < std::abs(m)) // Vertical segment:
        {
            A.col(1).maxCoeff(&argmax); // use max y
            A.col(1).minCoeff(&argmin); // use min y
            rho = std::cos(theta)/m;
        }
        else
        {
            A.col(0).maxCoeff(&argmax); // use max x
            A.col(0).minCoeff(&argmin); // use min x
            rho = std::sin(theta)/p;
        }
        Eigen::Vector2d point1 = {A(argmin, 0), A(argmin, 1)};
        Eigen::Vector2d point2 = {A(argmax, 0), A(argmax, 1)};
        point1 = closest_point(rho, theta, point1);
        point2 = closest_point(rho, theta, point2);
        LineSegment line(point1.x(), point1.y(), point2.x(), point2.y());
        lines.push_back(line);
        indices.erase(indices.begin());
        int index = lines.size()-1;
        cv::viz::Color color = colors[index % colors.size()];
        draw_line(line, output, color, 3, 10, std::to_string(index));
    }

    // Label lines
    cv::cvtColor(image, output, cv::COLOR_GRAY2RGB);
    LineSegment *baseline = NULL;
    LineSegment *serveline = NULL;
    for (auto &line : lines)
    {
        if (line.orientation == horizontal)
        {
            if (baseline == NULL || line.length > baseline->length)
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
    int distance_threshold = 20;
    for (auto &line : lines)
    {
        if (line.orientation == vertical)
        {
            if (line.distance_to({serveline->x1, serveline->y1}) < distance_threshold)
                left_sideline = &line;
            else if (line.distance_to({serveline->x2, serveline->y2}) < distance_threshold)
                right_sideline = &line;
            else if (line.distance_to({(serveline->x1 + serveline->x2)/2, (serveline->y1 + serveline->y2)/2}) < distance_threshold)
                centerline = &line;
        }
    }

    draw_line(*serveline, output, colors[0], 3, 10, std::string("serveline"));
    draw_line(*baseline, output, colors[1], 3, 10, std::string("baseline"));
    draw_line(*left_sideline, output, colors[2], 3, 10, std::string("left_sideline"));
    draw_line(*right_sideline, output, colors[3], 3, 10, std::string("right_sideline"));
    draw_line(*centerline, output, colors[4], 3, 10, std::string("centerline"));

    // Find intersections
    Eigen::Vector2d A = serveline->intersect_with(*left_sideline);
    Eigen::Vector2d B = serveline->intersect_with(*right_sideline);
    Eigen::Vector2d C = baseline->intersect_with(*left_sideline);
    Eigen::Vector2d D = baseline->intersect_with(*right_sideline);
    Eigen::Vector2d E = serveline->intersect_with(*centerline);


    //cv::calibrateCamera({{A, B, C, D, E}}, {})

    return output;
}
